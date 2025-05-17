#include "lltabledialog.h"
#include <QFontDatabase>

LLTableDialog::LLTableDialog(const QStringList &rowHeaders,
                             const QStringList &colHeaders,
                             QWidget *parent,
                             QVector<QVector<QString>> *initialData)
    : QDialog(parent)
{
    table = new QTableWidget(rowHeaders.size(), colHeaders.size(), this);
    table->setHorizontalHeaderLabels(colHeaders);
    table->setVerticalHeaderLabels(rowHeaders);
    table->horizontalHeader()->setFont(QFontDatabase::font("Noto Sans", "Regular", 11));
    table->verticalHeader()->setFont(QFontDatabase::font("Noto Sans", "Regular", 11));
    table->resizeColumnsToContents();
    table->resizeRowsToContents();

    // Wider columns
    for (int i = 0; i < table->columnCount(); ++i) {
        table->setColumnWidth(i, table->columnWidth(i) + 40);
    }

    // Taller rows
    for (int i = 0; i < table->rowCount(); ++i) {
        table->setRowHeight(i, table->rowHeight(i) + 5);
    }

    table->horizontalHeader()->setStretchLastSection(true);

    submitButton = new QPushButton("Finalizar", this);
    QFont submitButtonFont = QFontDatabase::font("Noto Sans", "Regular", 12);
    submitButtonFont.setBold(true);
    submitButton->setFont(submitButtonFont);
    submitButton->setCursor(Qt::PointingHandCursor);
    connect(submitButton, &QPushButton::clicked, this, &QDialog::accept);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(table);
    layout->addWidget(submitButton);
    layout->setContentsMargins(10, 10, 10, 10);
    setLayout(layout);

    setWindowTitle("Completar tabla LL(1)");

    // Total width = columns + vertical header
    int width = table->verticalHeader()->width();
    for (int i = 0; i < table->columnCount(); ++i)
        width += table->columnWidth(i);

    // Total height = rows + horizontal header
    int height = table->horizontalHeader()->height();
    for (int i = 0; i < table->rowCount(); ++i)
        height += table->rowHeight(i);

    width += 60;
    height += 100;

    QSize screenSize = QGuiApplication::primaryScreen()->availableSize();
    width = qMin(width, screenSize.width() - 100);
    height = qMin(height, screenSize.height() - 100);

    if (initialData != nullptr) {
        setInitialData(*initialData);
    }

    resize(width, height);
}

QVector<QVector<QString>> LLTableDialog::getTableData() const
{
    QVector<QVector<QString>> data;
    for (int i = 0; i < table->rowCount(); ++i) {
        QVector<QString> row;
        for (int j = 0; j < table->columnCount(); ++j) {
            QTableWidgetItem *item = table->item(i, j);
            row.append(item ? item->text() : "");
        }
        data.append(row);
    }
    return data;
}

void LLTableDialog::setInitialData(const QVector<QVector<QString>> &data)
{
    const int rows = qMin(data.size(), table->rowCount());
    const int cols = (rows > 0) ? qMin(data[0].size(), table->columnCount()) : 0;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QTableWidgetItem *item = table->item(i, j);

            if (!item) {
                item = new QTableWidgetItem();
                item->setTextAlignment(Qt::AlignCenter);
                table->setItem(i, j, item);
            }

            item->setText(data[i][j]);
        }
    }
}
