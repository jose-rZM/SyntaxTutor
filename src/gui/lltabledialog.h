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

#ifndef LLTABLEDIALOG_H
#define LLTABLEDIALOG_H

#include <QDialog>
#include <QGuiApplication>
#include <QHeaderView>
#include <QList>
#include <QPair>
#include <QPushButton>
#include <QScreen>
#include <QTableWidget>
#include <QVBoxLayout>

/**
 * @class LLTableDialog
 * @brief Dialog for filling and submitting an LL(1) parsing table.
 *
 * This class represents a dialog window that displays a table for users to
 * complete the LL(1) parsing matrix. It provides functionality to initialize
 * the table with data, retrieve the user's input, and highlight incorrect
 * answers.
 */
class LLTableDialog : public QDialog {
    Q_OBJECT
  public:
    /**
     * @brief Constructs the LL(1) table dialog with given headers and optional
     * initial data.
     * @param rowHeaders Row labels (non-terminal symbols).
     * @param colHeaders Column labels (terminal symbols).
     * @param parent Parent widget.
     * @param initialData Optional initial table data to pre-fill cells.
     */
    LLTableDialog(const QStringList& rowHeaders, const QStringList& colHeaders,
                  QWidget*                   parent,
                  QVector<QVector<QString>>* initialData = nullptr);

    /**
     * @brief Returns the contents of the table filled by the user.
     * @return A 2D vector representing the LL(1) table.
     */
    QVector<QVector<QString>> getTableData() const;

    /**
     * @brief Pre-fills the table with existing user data.
     *
     * This is used to populate the table with a previous (possibly incorrect)
     * answer when retrying a task or providing feedback.
     *
     * @param data A 2D vector of strings representing the initial cell values.
     */
    void setInitialData(const QVector<QVector<QString>>& data);

    /**
     * @brief Highlights cells that are incorrect based on provided coordinates.
     * @param coords A list of (row, column) pairs to highlight as incorrect.
     */
    void highlightIncorrectCells(const QList<QPair<int, int>>& coords);

  signals:
    /**
     * @brief Signal emitted when the user submits the table.
     * @param data The filled table data submitted by the user.
     */
    void submitted(const QVector<QVector<QString>>& data);

  private:
    QTableWidget* table; ///< The widget representing the LL(1) parsing table.
    QPushButton*  submitButton; ///< Button to submit the completed table.
};

#endif // LLTABLEDIALOG_H
