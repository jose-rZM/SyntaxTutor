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

class LLTableDialog : public QDialog
{
    Q_OBJECT
public:
    LLTableDialog(const QStringList &rowHeaders,
                  const QStringList &colHeaders,
                  QWidget *parent,
                  QVector<QVector<QString>> *initialData = nullptr);
    QVector<QVector<QString>> getTableData() const;
    void setInitialData(const QVector<QVector<QString>> &data);
    void highlightIncorrectCells(const QList<QPair<int, int>> &coords);

signals:
    void submitted(const QVector<QVector<QString>> &data);

private:
    QTableWidget *table;
    QPushButton *submitButton;
};

#endif // LLTABLEDIALOG_H
