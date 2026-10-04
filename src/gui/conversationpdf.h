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

#ifndef CONVERSATIONPDF_H
#define CONVERSATIONPDF_H

#include "apptypography.h"
#include "appversion.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QFontDatabase>
#include <QLatin1StringView>
#include <QMarginsF>
#include <QPageSize>
#include <QPair>
#include <QPrinter>
#include <QString>
#include <QStringList>
#include <QTextDocument>
#include <QVector>

/**
 * @brief Builds the PDF of an LL(1) or SLR(1) practice session.
 *
 * Both tutors describe their exercise - grammar, conversation, sets, table -
 * and this class lays it out the same way. It only uses the subset of HTML
 * and CSS that QTextDocument supports when printing (tables, backgrounds,
 * fonts), and splits tables too wide for an A4 page into blocks of columns,
 * so grammars typed by the user, however large, stay readable.
 */
class ConversationPdf {
    Q_DECLARE_TR_FUNCTIONS(ConversationPdf)

  public:
    /// @brief One message of the conversation.
    struct Message {
        QString text;
        bool    fromStudent = false;
        bool    correct     = true;
    };

    /// @brief A grammar rule: antecedent and consequent symbols.
    using Rule = QPair<QString, QVector<QString>>;

    /// @brief Data columns per block before a wide table is split.
    static constexpr int kMaxTableColumns = 10;

    /// @brief Starts a document for the exercise named @p exercise.
    explicit ConversationPdf(const QString& exercise) {
        html_ += QStringLiteral("<html><head><style>%1</style></head><body>")
                     .arg(styleSheet());
        html_ += QStringLiteral("<p class='kicker'>SyntaxTutor</p>"
                                "<h1>%1</h1><p class='meta'>%2</p>")
                     .arg(exercise.toHtmlEscaped(),
                          tr("Generado el %1 con SyntaxTutor %2")
                              .arg(QDateTime::currentDateTime().toString(
                                       QStringLiteral("dd/MM/yyyy HH:mm")),
                                   SyntaxTutor::Version::current())
                              .toHtmlEscaped());
    }

    /// @brief The grammar, one rule per line; numbered when @p numbered.
    void addGrammar(const QVector<Rule>& rules, bool numbered) {
        addSection(tr("Gramática"));
        html_ += QStringLiteral("<table class='grammar' cellspacing='0' "
                                "cellpadding='1'>");
        for (int i = 0; i < rules.size(); ++i) {
            html_ += QStringLiteral("<tr>");
            if (numbered) {
                html_ += QStringLiteral("<td class='num'>(%1)</td>").arg(i);
            }
            html_ += QStringLiteral("<td class='mono'>%1 → %2</td></tr>")
                         .arg(rules[i].first.toHtmlEscaped(),
                              QStringList(rules[i].second.begin(),
                                          rules[i].second.end())
                                  .join(QLatin1Char(' '))
                                  .toHtmlEscaped());
        }
        html_ += QStringLiteral("</table>");
    }

    /// @brief The tutor's questions and the student's answers, in order.
    void addConversation(const QVector<Message>& messages) {
        addSection(tr("Conversación"));
        html_ += QStringLiteral("<table class='chat' width='100%' "
                                "cellspacing='0' cellpadding='6'>");
        for (const Message& message : messages) {
            const bool    wrong = message.fromStudent && !message.correct;
            const QString rowClass =
                wrong ? QStringLiteral("wrong")
                      : (message.fromStudent ? QStringLiteral("student")
                                             : QStringLiteral("tutor"));
            QString who = message.fromStudent ? tr("Usuario") : tr("Tutor");
            if (wrong) {
                who += QStringLiteral("<br><span class='flag'>%1</span>")
                           .arg(tr("incorrecta"));
            }
            html_ += QStringLiteral("<tr class='%1'><td class='who' "
                                    "width='14%'>%2</td><td>%3</td></tr>")
                         .arg(rowClass, who,
                              message.text.toHtmlEscaped().replace(
                                  QLatin1Char('\n'), QStringLiteral("<br>")));
        }
        html_ += QStringLiteral("</table>");
    }

    /// @brief A heading for the next block; @p newPage starts a new page.
    void addSection(const QString& title, bool newPage = false) {
        html_ += QStringLiteral("<h2%1>%2</h2>")
                     .arg(newPage ? QStringLiteral(" class='newpage'")
                                  : QString(),
                          title.toHtmlEscaped());
    }

    /// @brief Name/value pairs, such as FIRST(A) = { a, b }.
    void addDefinitions(const QVector<QPair<QString, QString>>& rows) {
        html_ += QStringLiteral("<table class='defs' cellspacing='0' "
                                "cellpadding='1'>");
        for (const auto& [name, value] : rows) {
            html_ += QStringLiteral("<tr><td class='mono name'>%1</td>"
                                    "<td class='mono'>%2</td></tr>")
                         .arg(name.toHtmlEscaped(), value.toHtmlEscaped());
        }
        html_ += QStringLiteral("</table>");
    }

    /// @brief A small heading inside a section, such as a state's name.
    void addSubheading(const QString& text) {
        html_ += QStringLiteral("<h3>%1</h3>").arg(text.toHtmlEscaped());
    }

    /// @brief Lines in a monospaced block, such as the items of a state.
    void addLines(const QStringList& lines) {
        QStringList escaped;
        for (const QString& line : lines) {
            escaped << line.toHtmlEscaped();
        }
        html_ += QStringLiteral("<p class='mono lines'>%1</p>")
                     .arg(escaped.join(QStringLiteral("<br>")));
    }

    /**
     * @brief A table with a row header column.
     *
     * @p headers starts with the corner cell; each row of @p rows starts
     * with its row header. Tables wider than kMaxTableColumns data columns
     * are printed as consecutive blocks, each repeating the row headers,
     * and every block repeats its header row on each page it spans.
     */
    void addTable(const QStringList&          headers,
                  const QVector<QStringList>& rows) {
        const int dataColumns = static_cast<int>(headers.size()) - 1;
        for (int first = 0; first < qMax(dataColumns, 1);
             first += kMaxTableColumns) {
            const int last = qMin(first + kMaxTableColumns, dataColumns);
            if (dataColumns > kMaxTableColumns) {
                html_ += QStringLiteral("<p class='caption'>%1</p>")
                             .arg(tr("Columnas %1 a %2 de %3")
                                      .arg(first + 1)
                                      .arg(last)
                                      .arg(dataColumns));
            }
            html_ += QStringLiteral("<table class='grid' cellspacing='0' "
                                    "cellpadding='4'><thead><tr>");
            html_ += QStringLiteral("<th>%1</th>")
                         .arg(headers.value(0).toHtmlEscaped());
            for (int c = first; c < last; ++c) {
                html_ += QStringLiteral("<th class='mono'>%1</th>")
                             .arg(headers.value(c + 1).toHtmlEscaped());
            }
            html_ += QStringLiteral("</tr></thead><tbody>");
            for (int r = 0; r < rows.size(); ++r) {
                html_ += QStringLiteral("<tr class='%1'>")
                             .arg(r % 2 ? QStringLiteral("odd")
                                        : QStringLiteral("even"));
                html_ += QStringLiteral("<th class='mono'>%1</th>")
                             .arg(rows[r].value(0).toHtmlEscaped());
                for (int c = first; c < last; ++c) {
                    html_ += QStringLiteral("<td class='mono'>%1</td>")
                                 .arg(rows[r].value(c + 1).toHtmlEscaped());
                }
                html_ += QStringLiteral("</tr>");
            }
            html_ += QStringLiteral("</tbody></table>");
        }
    }

    /// @brief The document as HTML, for inspection.
    QString html() const { return html_ + QStringLiteral("</body></html>"); }

    /// @brief Prints the document to @p filePath as an A4 PDF.
    void print(const QString& filePath) const {
        ensureMonospaceFont();
        QTextDocument doc;
        // A fixed size: paper does not follow the on-screen text scale.
        QFont font = AppTypography::systemFont();
        font.setPointSizeF(10.0);
        doc.setDefaultFont(font);
        doc.setHtml(html());

        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(filePath);
        printer.setPageSize(QPageSize(QPageSize::A4));
        printer.setPageMargins(QMarginsF(15, 15, 15, 15),
                               QPageLayout::Millimeter);
        doc.print(&printer);
    }

  private:
    static constexpr auto kMonospaceFamily = "JetBrains Mono";

    // The PDF names its monospaced font in CSS, which needs a real family:
    // the platform's fixed font is an internal family on macOS that the
    // printer cannot resolve. The bundled JetBrains Mono is the same
    // everywhere; main() only loads it on Windows and Linux.
    static void ensureMonospaceFont() {
        if (!QFontDatabase::families().contains(
                QLatin1StringView(kMonospaceFamily))) {
            QFontDatabase::addApplicationFont(
                QStringLiteral(":/resources/fonts/JetBrainsMono-Regular.ttf"));
            QFontDatabase::addApplicationFont(
                QStringLiteral(":/resources/fonts/JetBrainsMono-Bold.ttf"));
        }
    }

    static QString styleSheet() {
        // Print palette shared with the exam report.
        return QStringLiteral(
                   "body { color: #1A1A1A; font-size: 10pt; }"
                   "p.kicker { color: #007B8A; font-size: 9pt; "
                   "font-weight: bold; margin: 0; }"
                   "h1 { font-size: 18pt; margin: 2px 0 2px 0; }"
                   "p.meta { color: #666666; font-size: 9pt; "
                   "margin: 0 0 12px 0; }"
                   "h2 { color: #007B8A; font-size: 13pt; "
                   "margin: 18px 0 6px 0; }"
                   "h2.newpage { page-break-before: always; }"
                   "h3 { font-size: 10.5pt; margin: 10px 0 2px 0; }"
                   ".mono { font-family: '%1'; }"
                   "p.lines { margin: 0 0 4px 12px; }"
                   "p.caption { color: #666666; font-size: 8.5pt; "
                   "margin: 10px 0 2px 0; }"
                   "td.num { color: #666666; }"
                   "td.name { padding-right: 12px; }"
                   "table.chat { margin-top: 2px; }"
                   "table.chat td { border-bottom: 1px solid #E0E0E0; }"
                   "td.who { color: #007B8A; font-weight: bold; "
                   "font-size: 9pt; }"
                   "tr.student td.who { color: #555555; }"
                   "tr.wrong td { background-color: #FBEAEA; }"
                   "tr.wrong td.who { color: #B23A37; }"
                   "span.flag { font-size: 8pt; font-weight: normal; }"
                   "table.grid { border-collapse: collapse; font-size: 9pt; "
                   "margin-bottom: 6px; }"
                   "table.grid th, table.grid td { border: 1px solid "
                   "#BBBBBB; text-align: center; }"
                   "table.grid thead th { background-color: #E8E8E8; }"
                   "table.grid tbody th { background-color: #F3F3F3; }"
                   "tr.odd td { background-color: #FAFAFA; }")
            .arg(QLatin1StringView(kMonospaceFamily));
    }

    QString html_;
};

#endif // CONVERSATIONPDF_H
