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

#ifndef EXAMREPORTDIALOG_H
#define EXAMREPORTDIALOG_H

#include "examsession.h"
#include <QDialog>

/**
 * @class ExamReportDialog
 * @brief End-of-exam report: grade, stats and a question-by-question review.
 *
 * Shown when an exam-mode exercise finishes. The headline is the 0-10
 * grade; below it a review lists every graded item with the student's
 * answer and the expected one, like a corrected exam sheet. The full report
 * can be exported to PDF through the owning tutor window.
 */
class ExamReportDialog : public QDialog {
    Q_OBJECT
  public:
    /**
     * @brief Builds the report for a finished exam session.
     *
     * @param session Graded exam data.
     * @param examTitle Exercise name shown in the header (e.g. "Examen
     * LL(1)").
     * @param parent Parent widget.
     */
    ExamReportDialog(const ExamSession& session, const QString& examTitle,
                     QWidget* parent = nullptr);

    /**
     * @brief Full report as a printable HTML document (used for the PDF
     * export).
     */
    QString reportHtml() const { return reportHtml_; }

  signals:
    /// @brief Emitted when the user asks to export the report to PDF.
    void exportRequested();

  private:
    QString buildReviewHtml(const ExamSession& session) const;

    QString reportHtml_;
};

#endif // EXAMREPORTDIALOG_H
