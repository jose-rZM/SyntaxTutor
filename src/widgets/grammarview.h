#pragma once

#include <QFrame>
#include <QString>
#include <QVector>

class QGridLayout;

class GrammarView : public QFrame {
  public:
    struct Row {
        QString index;
        QString lhs;
        QString marker;
        QString rhs;
    };

    explicit GrammarView(QWidget* parent = nullptr);

    void setRows(const QVector<Row>& rows);

  private:
    void clearRows();

    QGridLayout*   gridLayout;
    QVector<Row>   currentRows;
};
