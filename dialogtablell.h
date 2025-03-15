#ifndef DIALOGTABLELL_H
#define DIALOGTABLELL_H

#include <QDialog>
#include <QTableWidget>
#include <QMessageBox>

namespace Ui {
class DialogTableLL;
}

class DialogTableLL : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTableLL(const QStringList &row_headers, const QStringList &column_headers, int rows, int columns, QWidget *parent = nullptr);
    ~DialogTableLL();
    QTableWidget* getTableWidget() const;

private slots:
    void on_helpButton_clicked();

private:
    Ui::DialogTableLL *ui;
};

#endif // DIALOGTABLELL_H
