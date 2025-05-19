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

// ====== SLR(1) Tutor States =====================================
enum class StateSlr {
    A,
    A1,
    A2,
    A3,
    A4,
    A_prime,
    B,
    C,
    CA,
    CB,
    D,
    D1,
    D2,
    D_prime,
    E,
    E1,
    E2,
    F,
    FA,
    G,
    H,
    H_prime,
    fin
};

class TutorialManager;

// ====== Main Tutor Class for SLR(1) =============================
class SLRTutorWindow : public QMainWindow
{
    Q_OBJECT

public:
    // ====== Constructor / Destructor =============================
    explicit SLRTutorWindow(const Grammar &g,
                            TutorialManager *tm = nullptr,
                            QWidget *parent = nullptr);
    ~SLRTutorWindow();

    // ====== Core Flow Control =====================================
    QString generateQuestion();                    // Generate question for current state
    void updateState(bool isCorrect);              // Advance state depending on correctness
    QString FormatGrammar(const Grammar &grammar); // Utility for displaying grammar
    void fillSortedGrammar();                      // Prepares grammar in display-friendly format

    // ====== UI Interaction ========================================
    void addMessage(const QString &text, bool isUser);     // Add message to chat
    void addDivisorLine(const QString &stateName);         // Insert a visual divider
    void exportConversationToPdf(const QString &filePath); // Export full interaction
    void showTable();                                      // Render SLR(1) table
    void launchSLRWizard();
    void updateProgressPanel();                            // Refresh visual progress
    void addUserState(unsigned id);                        // Register a user-created state
    void addUserTransition(unsigned fromId,
                           const std::string &symbol,
                           unsigned toId); // Register a user-created transition

    // ====== Visual Feedback & Animations ==========================
    void animateLabelPop(QLabel *label);
    void animateLabelColor(QLabel *label, const QColor &flashColor);
    void wrongAnimation();             // Label animation for incorrect answer
    void wrongUserResponseAnimation(); // Message widget animation for incorrect answer

    // ====== Response Verification ================================
    bool verifyResponse(const QString &userResponse);
    bool verifyResponseForA(const QString &userResponse);
    bool verifyResponseForA1(const QString &userResponse);
    bool verifyResponseForA2(const QString &userResponse);
    bool verifyResponseForA3(const QString &userResponse);
    bool verifyResponseForA4(const QString &userResponse);
    bool verifyResponseForB(const QString &userResponse);
    bool verifyResponseForC(const QString &userResponse);
    bool verifyResponseForCA(const QString &userResponse);
    bool verifyResponseForCB(const QString &userResponse);
    bool verifyResponseForD(const QString &userResponse);
    bool verifyResponseForD1(const QString &userResponse);
    bool verifyResponseForD2(const QString &userResponse);
    bool verifyResponseForE(const QString &userResponse);
    bool verifyResponseForE1(const QString &userResponse);
    bool verifyResponseForE2(const QString &userResponse);
    bool verifyResponseForF(const QString &userResponse);
    bool verifyResponseForFA(const QString &userResponse);
    bool verifyResponseForG(const QString &userResponse);
    bool verifyResponseForH();

    // ====== Correct Solutions (Auto-generated) ====================
    QString solution(const std::string &state);
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
    std::ptrdiff_t solutionForE();
    QSet<unsigned> solutionForE1();
    QMap<unsigned, unsigned> solutionForE2();
    QSet<unsigned> solutionForF();
    QSet<QString> solutionForFA();
    QSet<QString> solutionForG();

    // ====== Pedagogical Feedback ==================================
    QString feedback(); // Delegates to appropriate feedback based on state
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
    QString feedbackForE();
    QString feedbackForE1();
    QString feedbackForE2();
    QString feedbackForF();
    QString feedbackForFA();
    QString feedbackForG();

private slots:
    void on_confirmButton_clicked();
    void on_userResponse_textChanged();

private:
    // ====== Helper Functions ======================================
    std::vector<std::string> qvectorToStdVector(const QVector<QString> &qvec);
    QVector<QString> stdVectorToQVector(const std::vector<std::string> &vec);
    QSet<QString> stdUnorderedSetToQSet(const std::unordered_set<std::string> &uset);
    std::unordered_set<std::string> qsetToStdUnorderedSet(const QSet<QString> &qset);
    std::unordered_set<Lr0Item> ingestUserItems(const QString &userResponse);
    std::vector<std::pair<std::string, std::vector<std::string>>> ingestUserRules(
        const QString &userResponse);
    void setupTutorial();
    // ====== Core Components ========================================
    Ui::SLRTutorWindow *ui;
    Grammar grammar;
    SLR1Parser slr1;

    // ====== State and Grammar Tracking =============================
    StateSlr currentState;
    QVector<QString> sortedNonTerminals;
    QVector<QPair<QString, QVector<QString>>> sortedGrammar;
    QString formattedGrammar;

    unsigned cntRightAnswers = 0;
    unsigned cntWrongAnswers = 0;

    // ====== State Machine Runtime Variables ========================
    std::unordered_set<state> userMadeStates; // All states the user has created
    std::unordered_map<unsigned, std::unordered_map<std::string, unsigned>>
        userMadeTransitions;             // Transitions made by the user
    UniqueQueue<unsigned> statesIdQueue; // States to be processed in B-C-CA-CB loop
    unsigned currentStateId = 0;
    state currentSlrState;

    QStringList followSymbols; // Used in CA-CB loop
    qsizetype currentFollowSymbolsIdx = 0;
    unsigned int nextStateId = 0;

    QVector<const state *> statesWithLr0Conflict; // Populated in F
    std::queue<unsigned> conflictStatesIdQueue;
    unsigned currentConflictStateId = 0;
    state currentConflictState;

    std::queue<unsigned> reduceStatesIdQueue; // States without conflicts but with reduce
    unsigned currentReduceStateId = 0;
    state currentReduceState;

    struct ActionEntry
    {
        enum Type { Shift, Reduce, Accept, Goto } type;
        int target;
        static ActionEntry makeShift(int s) { return {Shift, s}; }
        static ActionEntry makeReduce(int r) { return {Reduce, r}; }
        static ActionEntry makeAccept() { return {Accept, 0}; }
        static ActionEntry makeGoto(int g) { return {Goto, g}; }
    };

    QMap<int, QMap<QString, ActionEntry>> slrtable;
    QVector<QVector<QString>> rawTable;

    // ====== Conversation Log =======================================
    struct MessageLog
    {
        QString message;
        bool isUser;
        MessageLog(const QString &message, bool isUser)
            : message(message)
            , isUser(isUser)
        {}
    };

    QVector<MessageLog> conversationLog;
    QWidget *lastUserMessage = nullptr;

    TutorialManager *tm;
};

#endif // SLRTUTORWINDOW_H
