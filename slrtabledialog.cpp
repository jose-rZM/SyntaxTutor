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

    submitButton = new QPushButton("Finalizar", this);
    connect(submitButton, &QPushButton::clicked, this, &QDialog::accept);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(table);
    layout->addWidget(submitButton);
    setLayout(layout);

    setWindowTitle("Completar tabla SLR");
    resize(800, 600);
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
