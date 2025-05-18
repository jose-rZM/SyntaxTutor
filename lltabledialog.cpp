#include "lltabledialog.h"
#include <QFontDatabase>
#include <QStyledItemDelegate>

class CenterAlignDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void initStyleOption(QStyleOptionViewItem *opt, const QModelIndex &idx) const override
    {
        QStyledItemDelegate::initStyleOption(opt, idx);
        opt->displayAlignment = Qt::AlignCenter;
        opt->font = QFontDatabase::font("Noto Sans", "Regular", 14);
    }
};

LLTableDialog::LLTableDialog(const QStringList &rowHeaders,
                             const QStringList &colHeaders,
                             QWidget *parent,
                             QVector<QVector<QString>> *initialData)
    : QDialog(parent)
{
    table = new QTableWidget(rowHeaders.size(), colHeaders.size(), this);
    table->setItemDelegate(new CenterAlignDelegate(table));
    table->setAlternatingRowColors(true);
    table->setStyleSheet(R"(
    /* Fondo general y texto */
    QTableWidget {
        background-color: #1F1F1F;
        color:            #EEEEEE;
        gridline-color:   #444444;
        font-family:      'Noto Sans';
        font-size:        13px;
    }
    /* Cabeceras horizontales */
    QHeaderView::section {
        background-color: #2E2E2E;
        color:            #00ADB5;
        padding:          6px;
        border:           1px solid #444444;
    }
    /* Cabeceras verticales */
    QTableWidget QHeaderView::section:vertical {
        background-color: #2E2E2E;
        color:            #CCCCCC;
        padding:          6px;
        border:           1px solid #444444;
    }
    /* Botón esquina superior-izquierda */
    QTableCornerButton::section {
        background-color: #2E2E2E;
        border: 1px solid #444444;
    }
    /* Filas alternadas */
    QTableWidget {
        alternate-background-color: #252525;
    }
    /* Selección de celda */
    QTableWidget::item:selected {
        background-color: #00ADB5;
        color:            #FFFFFF;
    }
    /* Sin líneas en los bordes exterior */
    QTableWidget {
        show-decoration-selected: 1;
        selection-background-color: #00ADB5;
        selection-color: #FFFFFF;
    }
)");
    table->setHorizontalHeaderLabels(colHeaders);
    table->setVerticalHeaderLabels(rowHeaders);
    table->horizontalHeader()->setFont(QFontDatabase::font("Noto Sans", "Bold", 13));
    table->verticalHeader()->setFont(QFontDatabase::font("Noto Sans", "Bold", 13));
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
    submitButton->setStyleSheet(R"(
  QPushButton {
    background-color: #00ADB5;
    color: #FFFFFF;
    border: none;
    padding: 10px 20px;
    border-radius: 8px;
    font-size: 14px;
    font-family: 'Noto Sans';
    font-weight: bold;
  }
  QPushButton:hover {
    background-color: #00CED1;
  }
  QPushButton:pressed {
    background-color: #007F86;
  }
)");

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
