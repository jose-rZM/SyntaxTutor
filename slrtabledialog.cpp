#include "slrtabledialog.h"

SLRTableDialog::SLRTableDialog(int rowCount,
                               int colCount,
                               const QStringList &colHeaders,
                               QWidget *parent)
    : QDialog(parent)
{
    table = new QTableWidget(rowCount, colCount, this);
    table->setHorizontalHeaderLabels(colHeaders);

    QStringList rowLabels;
    for (int i = 0; i < rowCount; ++i)
        rowLabels << QString("State %1").arg(i);
    table->setVerticalHeaderLabels(rowLabels);

    table->resizeColumnsToContents();
    table->resizeRowsToContents();

    table->horizontalHeader()->setStretchLastSection(true);

    submitButton = new QPushButton("Finalizar", this);
    connect(submitButton, &QPushButton::clicked, this, &QDialog::accept);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(table);
    layout->addWidget(submitButton);
    layout->setContentsMargins(10, 10, 10, 10);
    setLayout(layout);

    setWindowTitle("Completar tabla SLR");

    int width = table->verticalHeader()->width();

    for (int i = 0; i < table->columnCount(); ++i)
        width += table->columnWidth(i);

    int height = table->horizontalHeader()->height();
    for (int i = 0; i < table->rowCount(); ++i)
        height += table->rowHeight(i);

    width += 60;   // extra
    height += 100; // extra

    QSize screenSize = QGuiApplication::primaryScreen()->availableSize();
    width = qMin(width, screenSize.width() - 100);
    height = qMin(height, screenSize.height() - 100);

    resize(width, height);
}

QVector<QVector<QString>> SLRTableDialog::getTableData() const
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
