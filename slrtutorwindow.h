#ifndef SLRTUTORWINDOW_H
#define SLRTUTORWINDOW_H

#include <QAbstractItemView>
#include <QDialog>
#include <QFileDialog>
#include <QGraphicsColorizeEffect>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollBar>
#include <QShortcut>
#include <QTableWidget>
#include <QTextDocument>
#include <QTextEdit>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>
#include <QtPrintSupport/QPrinter>
#include "UniqueQueue.h"
#include "backend/grammar.hpp"
#include "backend/slr1_parser.hpp"
#include "slrtabledialog.h"

namespace Ui {
class SLRTutorWindow;
}

enum class StateSlr { A, A1, A2, A3, A4, A_prime, B, C, CA, CB, D, D1, D2, D_prime, E, fin };

class SLRTutorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SLRTutorWindow(const Grammar& grammar, QWidget *parent = nullptr);
    ~SLRTutorWindow();
    QString FormatGrammar(const Grammar& grammar);
    void fillSortedGrammar();
    void addMessage(const QString& text, bool isUser);
    void addDivisorLine(const QString &stateName);
    void exportConversationToPdf(const QString& filePath);
    void showTable();
    void updateProgressPanel();
    void addUserState(unsigned id);
    void wrongAnimation();
    void wrongUserResponseAnimation();
    void animateLabelPop(QLabel *label);
    void animateLabelColor(QLabel *label, const QColor &flashColor);

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
    bool verifyResponseForCB(const QString& userResponse);
    bool verifyResponseForD(const QString& userResponse);
    bool verifyResponseForD1(const QString& userResponse);
    bool verifyResponseForD2(const QString& userResponse);
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
    QStringList solutionForCA();
    std::unordered_set<Lr0Item> solutionForCB();
    QString solutionForD();
    QString solutionForD1();
    QString solutionForD2();
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
    QString feedbackForCA();
    QString feedbackForCB();
    QString feedbackForD();
    QString feedbackForD1();
    QString feedbackForD2();
    QString feedbackForDPrime();
    // END FEEDBACK ------------------------------------------
    // ------------------------------------------------------
    // ------------------------------------------------------

    void updateState(bool isCorrect);
    QString generateQuestion();

private slots:
    void on_confirmButton_clicked();

    void on_userResponse_textChanged();

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
    QVector<QString> sortedNonTerminals;
    QVector<QPair<QString, QVector<QString>>> sortedGrammar;
    QString formattedGrammar;
    SLR1Parser slr1;
    StateSlr currentState;

    unsigned cntRightAnswers = 0, cntWrongAnswers = 0;

    // VARIABLES
    std::unordered_set<state> userMadeStates; // Track states made by the user
    unsigned currentStateId; // Track current state during questions
    UniqueQueue<unsigned> statesIdQueue; // Stores all pending states. B-C loop will end when this queue becomes empty
    state currentSlrState; // Track current state for validation purposes
    QStringList followSymbols; // Track following symbols after the dot for CB question, filled in CA
    qsizetype currentFollowSymbolsIdx = 0; // Track current following symbol, used in CB-CB loop
    unsigned int nextStateId; // Filled in generateQuestion, next state used in C-CA-CB questions
    // END VARIABLES

    struct MessageLog {
        QString message;
        bool isUser;

        MessageLog(const QString &message, bool isUser)
            : message(message)
            , isUser(isUser)
        {}
    };

    QVector<MessageLog> conversationLog;

    QWidget *lastUserMessage = nullptr;
};

#endif // SLRTUTORWINDOW_H
