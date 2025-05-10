#include "lltabledialog.h"

LLTableDialog::LLTableDialog(const QStringList &rowHeaders,
                             const QStringList &colHeaders,
                             QWidget *parent)
    : QDialog(parent)
{
    table = new QTableWidget(rowHeaders.size(), colHeaders.size(), this);
    table->setHorizontalHeaderLabels(colHeaders);
    table->setVerticalHeaderLabels(rowHeaders);

    table->resizeColumnsToContents();
    table->resizeRowsToContents();

    table->horizontalHeader()->setStretchLastSection(true);

    submitButton = new QPushButton("Finalizar", this);
    submitButton->setStyleSheet(R"(
    QPushButton {
        background-color: #393E46;
        color: white;
        border: none;
        padding: 8px 20px;
        border-radius: 8px;
        font-weight: bold;
        font-size: 13px;
        font-family: 'Noto Sans';
    }

    QPushButton:hover {
        background-color: #50575F;
    }

    QPushButton:pressed {
        background-color: #222831;
    }        
)");
    submitButton->setCursor(Qt::PointingHandCursor);
    connect(submitButton, &QPushButton::clicked, this, &QDialog::accept);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(table);
    layout->addWidget(submitButton);
    layout->setContentsMargins(10, 10, 10, 10);
    setLayout(layout);

    setWindowTitle("Completar tabla LL(1)");

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
