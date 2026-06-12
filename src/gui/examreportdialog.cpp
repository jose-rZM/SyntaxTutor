/*
 * SyntaxTutor - Interactive Tutorial About Syntax Analyzers
 * Copyright (C) 2025 Jose R. (jose-rzm)
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "examreportdialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

namespace {
constexpr auto kPassColor = "#11B3BC";
constexpr auto kFailColor = "#E0635F";

QString escaped(QString text) {
    return text.toHtmlEscaped().replace(QStringLiteral("\n"),
                                        QStringLiteral("<br/>"));
}
} // namespace

ExamReportDialog::ExamReportDialog(const ExamSession& session,
                                   const QString& examTitle, QWidget* parent)
    : QDialog(parent) {
    setObjectName("examReportDialog");
    setProperty("examReport", true);
    setWindowTitle(tr("Informe del examen"));
    setModal(true);
    resize(720, 600);
    setMinimumSize(620, 480);

    const double  grade     = session.grade();
    const QString gradeText = QLocale().toString(grade, 'f', 1);
    const bool    passed    = grade >= 5.0;
    const int     percent =
        session.total() == 0
                ? 0
                : static_cast<int>(100.0 * session.right() / session.total());

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(28, 24, 28, 24);
    rootLayout->setSpacing(12);

    auto* eyebrow = new QLabel(tr("MODO EXAMEN"), this);
    eyebrow->setObjectName("examReportEyebrow");
    rootLayout->addWidget(eyebrow);

    auto* title = new QLabel(examTitle, this);
    title->setObjectName("examReportTitle");
    rootLayout->addWidget(title);

    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(16);

    auto* gradeLabel = new QLabel(gradeText, this);
    gradeLabel->setObjectName("examReportGrade");
    gradeLabel->setProperty("passed", passed);
    headerLayout->addWidget(gradeLabel, 0, Qt::AlignTop);

    auto* statsColumn = new QVBoxLayout();
    statsColumn->setSpacing(2);
    auto* gradeCaption = new QLabel(tr("sobre 10"), this);
    gradeCaption->setObjectName("examReportGradeCaption");
    statsColumn->addWidget(gradeCaption);
    auto* statsLabel = new QLabel(tr("%1 de %2 respuestas correctas (%3%)")
                                      .arg(session.right())
                                      .arg(session.total())
                                      .arg(percent),
                                  this);
    statsLabel->setObjectName("examReportStats");
    statsColumn->addWidget(statsLabel);
    auto* verdictLabel =
        new QLabel(passed ? tr("Aprobado") : tr("Suspenso"), this);
    verdictLabel->setObjectName("examReportVerdict");
    verdictLabel->setProperty("passed", passed);
    statsColumn->addWidget(verdictLabel);
    statsColumn->addStretch(1);
    headerLayout->addLayout(statsColumn, 1);

    rootLayout->addLayout(headerLayout);

    auto* reviewCaption = new QLabel(tr("REVISIÓN"), this);
    reviewCaption->setObjectName("examReportReviewCaption");
    rootLayout->addWidget(reviewCaption);

    auto* review = new QTextBrowser(this);
    review->setObjectName("examReportReview");
    review->setOpenExternalLinks(false);
    const QString reviewHtml = buildReviewHtml(session);
    review->setHtml(reviewHtml);
    rootLayout->addWidget(review, 1);

    reportHtml_ = QStringLiteral("<h1>%1</h1>"
                                 "<p><b>%2</b> %3 — %4</p>%5")
                      .arg(escaped(examTitle), tr("Calificación:"),
                           tr("%1 / 10").arg(gradeText),
                           tr("%1 de %2 respuestas correctas (%3%)")
                               .arg(session.right())
                               .arg(session.total())
                               .arg(percent),
                           reviewHtml);

    auto* footerLayout = new QHBoxLayout();
    footerLayout->setSpacing(12);
    footerLayout->addStretch(1);

    auto* exportButton = new QPushButton(tr("Exportar PDF"), this);
    exportButton->setObjectName("examReportExportButton");
    exportButton->setProperty("role", "primary");
    exportButton->setAutoDefault(false);
    exportButton->setCursor(Qt::PointingHandCursor);
    footerLayout->addWidget(exportButton);

    auto* closeButton = new QPushButton(tr("Cerrar"), this);
    closeButton->setObjectName("examReportCloseButton");
    closeButton->setAutoDefault(false);
    closeButton->setCursor(Qt::PointingHandCursor);
    footerLayout->addWidget(closeButton);

    rootLayout->addLayout(footerLayout);

    connect(exportButton, &QPushButton::clicked, this,
            &ExamReportDialog::exportRequested);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

QString ExamReportDialog::buildReviewHtml(const ExamSession& session) const {
    QString html = QStringLiteral("<table width='100%' cellspacing='0' "
                                  "cellpadding='6' style='font-size:13px;'>");
    html +=
        QStringLiteral("<tr>"
                       "<th align='left'>%1</th>"
                       "<th align='left'>%2</th>"
                       "<th align='left'>%3</th>"
                       "<th align='left'></th>"
                       "</tr>")
            .arg(tr("Pregunta"), tr("Tu respuesta"), tr("Respuesta correcta"));

    int index = 0;
    for (const ExamRecord& record : session.records()) {
        const QString rowColor = (index % 2 == 0) ? QStringLiteral("#212526")
                                                  : QStringLiteral("#1B1F20");
        const QString mark =
            record.correct ? QStringLiteral("<span style='color:%1;'>✔</span>")
                                 .arg(kPassColor)
                           : QStringLiteral("<span style='color:%1;'>✘</span>")
                                 .arg(kFailColor);
        html += QStringLiteral("<tr style='background-color:%1;'>"
                               "<td>%2</td>"
                               "<td>%3</td>"
                               "<td>%4</td>"
                               "<td align='center'>%5</td>"
                               "</tr>")
                    .arg(rowColor, escaped(record.question),
                         record.userAnswer.trimmed().isEmpty()
                             ? tr("(vacía)")
                             : escaped(record.userAnswer),
                         escaped(record.correctAnswer), mark);
        ++index;
    }
    html += QStringLiteral("</table>");
    return html;
}
