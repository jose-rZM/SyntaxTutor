#include "dialogtablell.h"
#include "ui_dialogtablell.h"

DialogTableLL::DialogTableLL(const QStringList &row_headers, const QStringList &column_headers, int rows, int columns, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogTableLL)
{
    ui->setupUi(this);

    setWindowTitle("Rellena la Tabla LL(1)");
    setModal(true);

    ui->tableWidget->setRowCount(rows);
    ui->tableWidget->setColumnCount(columns);
    ui->tableWidget->setHorizontalHeaderLabels(column_headers);
    ui->tableWidget->setVerticalHeaderLabels(row_headers);
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(this->layout());
    if (layout) {
        layout->setAlignment(ui->tableWidget, Qt::AlignCenter);
    }

    connect(ui->pushButton, &QPushButton::clicked, this, &QDialog::accept);
}

DialogTableLL::~DialogTableLL()
{
    delete ui;
}

QTableWidget* DialogTableLL::getTableWidget() const {
    return ui->tableWidget;
}

void DialogTableLL::on_helpButton_clicked()
{
    QMessageBox::information(this, "Ayuda - Tabla LL(1)",
                             "Para rellenar la tabla LL(1):\n"
                             "1. Cada celda contiene una producción.\n"
                             "2. Las filas representan los no terminales.\n"
                             "3. Las columnas representan los terminales y '$'.\n"
                             "4. Si una celda tiene múltiples producciones, la gramática no es LL(1). En caso de ocurrir, separa las producciones con una \",\"\n\n"
                             "Formato:\n"
                             " - Indica solo el consecuente en cada celda. Cada símbolo debe estar separado por un espacio.\n"
                             " - Usa \"EPSILON\" para la producción vacía.");
}


