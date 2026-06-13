/*
 * SyntaxTutor - Interactive Tutorial About Syntax Analyzers
 * Copyright (C) 2025 Jose R. (jose-rzm)
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SLRTUTORWINDOW_H
#define SLRTUTORWINDOW_H

#include "UniqueQueue.h"
#include "automatonview.h"
#include "examsession.h"
#include "grammar.hpp"
#include "grammarview.h"
#include "slr1_parser.hpp"
#include "slrtabledialog.h"
#include <QAbstractItemView>
#include <QDialog>
#include <QFileDialog>
#include <QGraphicsColorizeEffect>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QShortcut>
#include <QTableWidget>
#include <QTextDocument>
#include <QTextEdit>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QtPrintSupport/QPrinter>

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
/**
 * @class SLRTutorWindow
 * @brief Main window for the SLR(1) interactive tutoring mode in SyntaxTutor.
 *
 * This class implements an interactive, step-by-step tutorial to teach students
 * how to construct SLR(1) parsing tables, including closure, GOTO, automaton
 * construction, FOLLOW sets, and the final table.
 *
 * It supports animated feedback, pedagogical guidance, error correction, and
 * export of the tutoring session.
 *
 * The tutor follows a finite-state flow (`StateSlr`) to structure learning,
 * with corrective explanations and automatic evaluation at each step.
 */
class SLRTutorWindow : public QWidget {
    Q_OBJECT

  public:
    // ====== Constructor / Destructor =============================
    /**
     * @brief Constructs the SLR(1) tutor window with a given grammar.
     * @param g The grammar used for the session.
     * @param tm Optional pointer to the tutorial manager (for guided tour).
     * @param parent Parent widget.
     * @param examMode When true, all feedback is suppressed: answers are
     * recorded silently, error sub-questions never trigger, and a graded
     * report is shown at the end.
     */
    explicit SLRTutorWindow(const Grammar& g, TutorialManager* tm = nullptr,
                            QWidget* parent = nullptr, bool examMode = false);
    ~SLRTutorWindow();

    // ====== Core Flow Control =====================================
    /**
     * @brief Generates a new question for the current tutor state.
     * @return The formatted question string.
     */
    QString generateQuestion();

    /**
     * @brief Updates tutor state based on whether the last answer was correct.
     * @param isCorrect Whether the user's answer was correct.
     */
    void updateState(bool isCorrect);
    QString
    FormatGrammar(const Grammar& grammar); /// < Utility for displaying grammar
    QVector<GrammarView::Row> buildGrammarRows(const Grammar& grammar) const;
    void fillSortedGrammar(); /// < Prepares grammar in display-friendly format

    // ====== UI Interaction ========================================
    void addMessage(const QString& text, bool isUser); /// < Add message to chat
    void addGrammarMessage();
    void addWidgetMessage(QWidget* widget);
    void exportConversationToPdf(
        const QString& filePath); /// < Export full interaction
    void showTable();             /// < Render SLR(1) table
    void launchSLRWizard();
    void updateProgressPanel();     /// < Refresh visual progress
    void addUserState(unsigned id); /// < Register a user-created state
    void addUserTransition(unsigned fromId, const std::string& symbol,
                           unsigned toId); // Register a user-created transition

    // ====== Visual Feedback & Animations ==========================
    void animateLabelPop(QLabel* label);
    void animateLabelColor(QLabel* label, const QColor& flashColor);
    void wrongAnimation();             // Label animation for incorrect answer
    void wrongUserResponseAnimation(); // Message widget animation for incorrect
                                       // answer
    void markLastUserIncorrect();

    // ====== Response Verification ================================
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
    bool verifyResponseForE(const QString& userResponse);
    bool verifyResponseForE1(const QString& userResponse);
    bool verifyResponseForE2(const QString& userResponse);
    bool verifyResponseForF(const QString& userResponse);
    bool verifyResponseForFA(const QString& userResponse);
    bool verifyResponseForG(const QString& userResponse);
    bool verifyResponseForH();

    // ====== Correct Solutions (Auto-generated) ====================
    QString                     solution(const std::string& state);
    std::unordered_set<Lr0Item> solutionForA();
    QString                     solutionForA1();
    QString                     solutionForA2();
    std::vector<std::pair<std::string, std::vector<std::string>>>
                                solutionForA3();
    std::unordered_set<Lr0Item> solutionForA4();
    unsigned                    solutionForB();
    unsigned                    solutionForC();
    QStringList                 solutionForCA();
    std::unordered_set<Lr0Item> solutionForCB();
    QStringList                 solutionForD();
    QString                     solutionForD1();
    QString                     solutionForD2();
    std::ptrdiff_t              solutionForE();
    QSet<unsigned>              solutionForE1();
    QMap<unsigned, unsigned>    solutionForE2();
    QSet<unsigned>              solutionForF();
    QSet<QString>               solutionForFA();
    QSet<QString>               solutionForG();

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
    QString TeachDeltaFunction(const std::unordered_set<Lr0Item>& items,
                               const QString&                     symbol);
    void TeachClosureStep(std::unordered_set<Lr0Item>& items, unsigned int size,
                          std::unordered_set<std::string>& visited, int depth,
                          QString& output);
    QString TeachClosure(const std::unordered_set<Lr0Item>& initialItems);
    void    updatePlaceholder();
    bool    confirmExitToHome();
    QString promptExportFilePath() const;

    // ====== LR(0) Automaton Panel =================================
    /**
     * @brief Creates the automaton tab and loads the LR(0) graph, hidden.
     *
     * Does nothing in exam mode: the automaton would reveal the collection.
     */
    void setupAutomatonPanel();

    /**
     * @brief Syncs the automaton panel with the current tutor state.
     *
     * Highlights the state under analysis during the C/CA/CB loop, switches
     * to full consultation mode from D onwards, and marks conflict (F/FA)
     * and reduce (G) states.
     */
    void updateAutomatonPanel();

    // ====== Exam Mode =============================================
    void    postQuestion();     ///< Shows and remembers the next question.
    QString examSolutionText(); ///< Expected answer for the current state.
    void    scoreExamTable(
           const QStringList& colHeaders); ///< Grades the SLR table per cell.
    void showExamReport();                 ///< Opens the end-of-exam report.
    void exportExamReportToPdf(const QString& filePath,
                               const QString& html) const;
#ifdef SYNTAXTUTOR_TESTING
  public:
    QString  currentStateForTest() const;
    int      rightCountForTest() const;
    int      wrongCountForTest() const;
    void     setAnswerForTest(const QString& text);
    void     submitForTest();
    unsigned currentStateIdForTest() const;
    QString  currentCbSymbolForTest() const;
    void     setNextExportFilePathForTest(const QString& filePath);
    double   examGradeForTest() const { return examSession.grade(); }
    int      examTotalForTest() const { return examSession.total(); }
    int      examRightForTest() const { return examSession.right(); }
#endif
  private slots:
    void on_backButton_clicked();
    void on_confirmButton_clicked();
    void on_userResponse_textChanged();

  signals:
    void sessionFinished(int cntRight, int cntWrong);
    void exitRequested(bool applyResults, int cntRight, int cntWrong);

  protected:
    void closeEvent(QCloseEvent* event) override {
        emit sessionFinished(cntRightAnswers, cntWrongAnswers);
        QWidget::closeEvent(event);
    }

    bool eventFilter(QObject* obj, QEvent* event) override;

  private:
    void relayoutChatMessages();

    // ====== Helper Functions ======================================
    std::vector<std::string> qvectorToStdVector(const QVector<QString>& qvec);
    QVector<QString> stdVectorToQVector(const std::vector<std::string>& vec);
    QSet<QString>
    stdUnorderedSetToQSet(const std::unordered_set<std::string>& uset);
    std::unordered_set<std::string>
    qsetToStdUnorderedSet(const QSet<QString>& qset);
    std::unordered_set<Lr0Item> ingestUserItems(const QString& userResponse);
    std::vector<std::pair<std::string, std::vector<std::string>>>
         ingestUserRules(const QString& userResponse);
    void setupTutorial();
    void requestExit(bool applyResults);
    // ====== Core Components ========================================
    Ui::SLRTutorWindow* ui;
    Grammar             grammar;
    SLR1Parser          slr1;

    // ====== State and Grammar Tracking =============================
    StateSlr                                  currentState;
    QVector<QString>                          sortedNonTerminals;
    QVector<QPair<QString, QVector<QString>>> sortedGrammar;
    QString                                   formattedGrammar;
    GrammarView*                              grammarView = nullptr;

    unsigned cntRightAnswers = 0;
    unsigned cntWrongAnswers = 0;

    // ====== Exam Mode ==============================================
    bool        examMode = false;
    ExamSession examSession;
    QString     currentQuestionText;

    // ====== State Machine Runtime Variables ========================
    std::unordered_set<state> userMadeStates; // All states the user has created
    std::unordered_map<unsigned, std::unordered_map<std::string, unsigned>>
        userMadeTransitions; // Transitions made by the user
    UniqueQueue<unsigned>
             statesIdQueue; // States to be processed in B-C-CA-CB loop
    unsigned currentStateId = 0;
    state    currentSlrState;

    QStringList  followSymbols; // Used in CA-CB loop
    qsizetype    currentFollowSymbolsIdx = 0;
    unsigned int nextStateId             = 0;

    QVector<const state*> statesWithLr0Conflict; // Populated in F
    std::queue<unsigned>  conflictStatesIdQueue;
    unsigned              currentConflictStateId = 0;

    // ====== LR(0) Automaton Panel ==================================
    AutomatonView* automatonView = nullptr; // null in exam mode
    QSet<unsigned> reduceStateIds;          // States highlighted in G
    state                 currentConflictState;

    std::queue<unsigned>
             reduceStatesIdQueue; // States without conflicts but with reduce
    unsigned currentReduceStateId = 0;
    state    currentReduceState;

    struct ActionEntry {
        enum Type { Shift, Reduce, Accept, Goto } type;
        int                target;
        static ActionEntry makeShift(int s) { return {Shift, s}; }
        static ActionEntry makeReduce(int r) { return {Reduce, r}; }
        static ActionEntry makeAccept() { return {Accept, 0}; }
        static ActionEntry makeGoto(int g) { return {Goto, g}; }
    };

    QMap<int, QMap<QString, ActionEntry>> slrtable;
    QVector<QVector<QString>>             rawTable;

    // ====== Conversation Log =======================================
    struct MessageLog {
        QString message;
        bool    isUser;
        bool    isCorrect = true;

        MessageLog(const QString& message, bool isUser)
            : message(message), isUser(isUser) {}

        void toggleIsCorrect() { isCorrect = false; }
    };

    QVector<MessageLog> conversationLog;
    QWidget*            lastUserMessage       = nullptr;
    qsizetype           lastUserMessageLogIdx = -1;

    QPropertyAnimation* m_shakeAnimation =
        nullptr; // For interrupting userResponse animation if they spam enter
                 // key

    TutorialManager* tm;

#ifdef SYNTAXTUTOR_TESTING
    mutable QString nextExportFilePathForTest;
#endif

    QRegularExpression       re{"^\\s+|\\s+$"};
    const QRegularExpression kWhitespace{"\\s+"};
};

#endif // SLRTUTORWINDOW_H
