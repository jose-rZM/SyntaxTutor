#ifndef SLRTUTORWINDOW_H
#define SLRTUTORWINDOW_H

#include <QMainWindow>
#include "backend/grammar.hpp"
#include "backend/slr1_parser.hpp"
#include <QMessageBox>
#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QAbstractItemView>
#include <QTime>

namespace Ui {
class SLRTutorWindow;
}

class SLRTutorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SLRTutorWindow(const Grammar& grammar, QWidget *parent = nullptr);
    ~SLRTutorWindow();
    QString FormatGrammar(const Grammar& grammar);

    void addMessage(const QString& text, bool isUser);

private slots:
    void on_confirmButton_clicked();

private:
    // HELPER FUNCTIONS --------------------
    std::vector<std::string> qvectorToStdVector(const QVector<QString>& qvec);
    QVector<QString> stdVectorToQVector(const std::vector<std::string>& vec);
    QSet<QString> stdUnorderedSetToQSet(const std::unordered_set<std::string>& uset);
    std::unordered_set<std::string> qsetToStdUnorderedSet(const QSet<QString>& qset);
    // END HELPER FUNCTIONS ----------------

    Ui::SLRTutorWindow *ui;
    Grammar grammar;
    QVector<QPair<QString, QVector<QString>>> sortedGrammar;
    SLR1Parser slr1;
};

#endif // SLRTUTORWINDOW_H
