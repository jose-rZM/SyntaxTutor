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

#ifndef SLRTABLEDIALOG_H
#define SLRTABLEDIALOG_H

#include <QDialog>
#include <QGuiApplication>
#include <QHeaderView>
#include <QList>
#include <QPair>
#include <QPushButton>
#include <QScreen>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QVector>

/**
 * @class SLRTableDialog
 * @brief Dialog window for completing and submitting an SLR(1) parsing table.
 *
 * This class displays a table-based UI for students to fill in the ACTION and
 * GOTO parts of the SLR(1) parsing table. It supports initializing the table
 * with data, retrieving user input, and integrating with correction logic in
 * tutorial or challenge mode.
 */
class SLRTableDialog : public QDialog {
    Q_OBJECT
  public:
    /**
     * @brief Constructs the SLR(1) table dialog.
     *
     * @param rowCount Number of rows (usually equal to number of LR(0) states).
     * @param colCount Number of columns (symbols = terminals + non-terminals).
     * @param colHeaders Header labels for the columns.
     * @param parent Parent widget.
     * @param initialData Optional initial data to pre-fill the table.
     */
    SLRTableDialog(int rowCount, int colCount, const QStringList& colHeaders,
                   QWidget*                   parent      = nullptr,
                   QVector<QVector<QString>>* initialData = nullptr);

    /**
     * @brief Retrieves the content of the table after user interaction.
     * @return A 2D vector representing the current table values.
     */
    QVector<QVector<QString>> getTableData() const;

    /**
     * @brief Fills the table with existing data.
     *
     * This method is used to show a previous user submission (e.g., during
     * retries or feedback).
     *
     * @param data 2D vector containing the table data to display.
     */
    void setInitialData(const QVector<QVector<QString>>& data);
    void setGuidedModeActive(bool active);

    void highlightIncorrectCells(const QList<QPair<int, int>>& coords);
    void highlightInvalidCells(const QList<QPair<int, int>>& coords);

  private:
    void commitPendingEdit();

    QTableWidget* table;        ///< Widget for editing the SLR(1) table.
    QPushButton*  submitButton; ///< Button used to validate/submit the table.
    QPushButton*  guidedButton; ///< Button to launch guided mode.

  signals:
    void submitted(const QVector<QVector<QString>>& data);
    void guidedRequested(const QVector<QVector<QString>>& data);
};

#endif // SLRTABLEDIALOG_H
