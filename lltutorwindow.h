#ifndef LLTUTORWINDOW_H
#define LLTUTORWINDOW_H

#include <QMainWindow>
#include "backend/grammar.hpp"
#include "backend/ll1_parser.hpp"
#include <QMessageBox>
#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QAbstractItemView>
#include <QTime>
#include "dialogtablell.h"

namespace Ui {
class LLTutorWindow;
}
enum class State {
    A, A1, A2, A_prime, B, B1, B2, B_prime, C, fin
};
class LLTutorWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit LLTutorWindow(const Grammar& grammar, QWidget *parent = nullptr);
    ~LLTutorWindow();
    QString FormatGrammar(const Grammar& grammar);

    void addMessage(const QString& text, bool isUser);

    // VERIFY RESPONSE ---------------------------------------
    bool verifyResponse(const QString& userResponse);
    bool verifyResponseForA(const QString& userResponse);
    bool verifyResponseForA1(const QString& userResponse);
    bool verifyResponseForA2(const QString& userResponse);
    bool verifyResponseForB(const QString& userResponse);
    bool verifyResponseForB1(const QString& userResponse);
    bool verifyResponseForB2(const QString& userResponse);
    bool verifyResponseForC();
    // END VERIFY RESPONSE ----------------------------------

    // SOLUTIONS --------------------------------------------
    QString solution(const std::string& state);
    QString solutionForA();
    QString solutionForA1();
    QString solutionForA2();
    QSet<QString> solutionForB();
    QSet<QString> solutionForB1();
    QSet<QString> solutionForB2();
    // END SOLUTIONS -----------------------------------------

    // FEEDBACK ----------------------------------------------
    QString feedback();
    QString feedbackForA();
    QString feedbackForA1();
    QString feedbackForA2();
    QString feedbackForAPrime();
    QString feedbackForB();
    QString feedbackForB1();
    QString feedbackForB2();
    QString feedbackForBPrime();
    // END FEEDBACK ------------------------------------------


    void updateState(bool isCorrect);
    QString generateQuestion();
    void llTable();
private slots:
    void on_confirmButton_clicked();

private:
    // HELPER FUNCTIONS --------------------
    std::vector<std::string> qvectorToStdVector(const QVector<QString>& qvec);
    QVector<QString> stdVectorToQVector(const std::vector<std::string>& vec);
    QSet<QString> stdUnorderedSetToQSet(const std::unordered_set<std::string>& uset);
    std::unordered_set<std::string> qsetToStdUnorderedSet(const QSet<QString>& qset);
    // END HELPER FUNCTIONS ----------------

    Ui::LLTutorWindow *ui;
    Grammar grammar;
    LL1Parser ll1;


    State currentState;
    QVector<QPair<QString, QVector<QString>>> sortedGrammar;
    QMap<QString, QMap<QString, QVector<QVector<QString>>>> lltable;
    QSet<QString> solutionSet;
    size_t currentRule = 0;
};

#endif // LLTUTORWINDOW_H
