#ifndef SLRTABLEDIALOG_H
#define SLRTABLEDIALOG_H

#include <QDialog>
#include <QGuiApplication>
#include <QHeaderView>
#include <QPushButton>
#include <QScreen>
#include <QTableWidget>
#include <QVBoxLayout>

class SLRTableDialog : public QDialog
{
    Q_OBJECT
public:
    SLRTableDialog(int rowCount,
                   int colCount,
                   const QStringList &colHeaders,
                   QWidget *parent = nullptr,
                   QVector<QVector<QString>> *initialData = nullptr);

    QVector<QVector<QString>> getTableData() const;
    void setInitialData(const QVector<QVector<QString>> &data);

private:
    QTableWidget *table;
    QPushButton *submitButton;
};

#endif // SLRTABLEDIALOG_H
