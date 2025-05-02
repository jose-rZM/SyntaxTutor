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

enum class StateSlr {
    A, A1, A2, A3, A4, A_prime, B, C, CA, CB, fin
};

class SLRTutorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SLRTutorWindow(const Grammar& grammar, QWidget *parent = nullptr);
    ~SLRTutorWindow();
    QString FormatGrammar(const Grammar& grammar);

    void addMessage(const QString& text, bool isUser);

    // ------------------------------------------------------
    // ------------------------------------------------------
    // VERIFY RESPONSE ---------------------------------------
    bool verifyResponse(const QString& userResponse);
    bool verifyResponseForA(const QString& userResponse);
    bool verifyResponseForA1(const QString& userResponse);
    bool verifyResponseForA2(const QString& userResponse);
    bool verifyResponseForA3(const QString& userResponse);
    bool verifyResponseForA4(const QString& userResponse);
    bool verifyResponseForB(const QString& userResponse);
    bool verifyResponseForC(const QString& userResponse);
    bool verifyResponseForCA(const QString& userResponse);

    // END VERIFY RESPONSE ----------------------------------
    // ------------------------------------------------------
    // ------------------------------------------------------


    // ------------------------------------------------------
    // ------------------------------------------------------
    // SOLUTIONS --------------------------------------------
    QString solution(const std::string& state);
    std::unordered_set<Lr0Item> solutionForA();
    QString solutionForA1();
    QString solutionForA2();
    std::vector<std::pair<std::string, std::vector<std::string>>> solutionForA3();
    std::unordered_set<Lr0Item> solutionForA4();
    unsigned solutionForB();
    unsigned solutionForC();
    // END SOLUTIONS -----------------------------------------
    // ------------------------------------------------------
    // ------------------------------------------------------


    // ------------------------------------------------------
    // ------------------------------------------------------
    // FEEDBACK ----------------------------------------------
    QString feedback();
    QString feedbackForA();
    QString feedbackForA1();
    QString feedbackForA2();
    QString feedbackForA3();
    QString feedbackForA4();
    QString feedbackForAPrime();
    QString feedbackForB();
    QString feedbackForB1();
    QString feedbackForB2();
    QString feedbackForBPrime();
    QString feedbackForC();
    // END FEEDBACK ------------------------------------------
    // ------------------------------------------------------
    // ------------------------------------------------------

    void updateState(bool isCorrect);
    QString generateQuestion();

private slots:
    void on_confirmButton_clicked();

private:
    // HELPER FUNCTIONS --------------------
    std::vector<std::string> qvectorToStdVector(const QVector<QString>& qvec);
    QVector<QString> stdVectorToQVector(const std::vector<std::string>& vec);
    QSet<QString> stdUnorderedSetToQSet(const std::unordered_set<std::string>& uset);
    std::unordered_set<std::string> qsetToStdUnorderedSet(const QSet<QString>& qset);
    std::unordered_set<Lr0Item> ingestUserItems(const QString& userResponse);
    std::vector<std::pair<std::string, std::vector<std::string>>> ingestUserRules(const QString& userResponse);
    // END HELPER FUNCTIONS ----------------

    Ui::SLRTutorWindow *ui;
    Grammar grammar;
    QVector<QPair<QString, QVector<QString>>> sortedGrammar;
    SLR1Parser slr1;
    StateSlr currentState;
    unsigned currentStateId = 0;
    unsigned currentTotalStates = 1;
    state currentSlrState;
};

#endif // SLRTUTORWINDOW_H
