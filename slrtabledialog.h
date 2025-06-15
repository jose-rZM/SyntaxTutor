#ifndef SLRTABLEDIALOG_H
#define SLRTABLEDIALOG_H

#include <QDialog>
#include <QGuiApplication>
#include <QHeaderView>
#include <QPushButton>
#include <QScreen>
#include <QTableWidget>
#include <QVBoxLayout>

/**
 * @class SLRTableDialog
 * @brief Dialog window for completing and submitting an SLR(1) parsing table.
 *
 * This class displays a table-based UI for students to fill in the ACTION and GOTO
 * parts of the SLR(1) parsing table. It supports initializing the table with data,
 * retrieving user input, and integrating with correction logic in tutorial or challenge mode.
 */
class SLRTableDialog : public QDialog
{
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
    SLRTableDialog(int rowCount,
                   int colCount,
                   const QStringList &colHeaders,
                   QWidget *parent = nullptr,
                   QVector<QVector<QString>> *initialData = nullptr);

    /**
     * @brief Retrieves the content of the table after user interaction.
     * @return A 2D vector representing the current table values.
     */
    QVector<QVector<QString>> getTableData() const;

    /**
     * @brief Fills the table with existing data.
     *
     * This method is used to show a previous user submission (e.g., during retries or feedback).
     *
     * @param data 2D vector containing the table data to display.
     */
    void setInitialData(const QVector<QVector<QString>> &data);

private:
    QTableWidget *table;       ///< Widget for editing the SLR(1) table.
    QPushButton *submitButton; ///< Button used to submit the filled table.
};

#endif // SLRTABLEDIALOG_H
