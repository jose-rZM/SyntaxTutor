#ifndef LLTUTORWINDOW_H
#define LLTUTORWINDOW_H

#include <QAbstractItemView>
#include <QDialog>
#include <QFileDialog>
#include <QGraphicsColorizeEffect>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QMessageBox>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollBar>
#include <QShortcut>
#include <QTableWidget>
#include <QTextDocument>
#include <QTextEdit>
#include <QTime>
#include <QTimer>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QtPrintSupport/QPrinter>

#include "backend/grammar.hpp"
#include "backend/ll1_parser.hpp"
#include "lltabledialog.h"

class TutorialManager;

namespace Ui {
class LLTutorWindow;
}

// ====== LL(1) Tutor States ===================================
enum class State { A, A1, A2, A_prime, B, B1, B2, B_prime, C, C_prime, fin };

// ====== LL(1) Tutor Main Class ===============================
class LLTutorWindow : public QMainWindow
{
    Q_OBJECT

public:
    // ====== Derivation Tree (used in TeachFirst) =============
    struct TreeNode
    {
        QString label;
        std::vector<std::unique_ptr<TreeNode>> children;
    };

    // ====== Constructor / Destructor =========================
    explicit LLTutorWindow(const Grammar &grammar,
                           TutorialManager *tm = nullptr,
                           QWidget *parent = nullptr);
    ~LLTutorWindow();

    // ====== State Machine & Question Logic ====================
    QString generateQuestion();                    // Returns question for current state
    void updateState(bool isCorrect);              // Advance state based on user input
    QString FormatGrammar(const Grammar &grammar); // Nicely format grammar for display

    // ====== UI Interaction ====================================
    void addMessage(const QString &text, bool isUser);     // Add text message to chat
    void addWidgetMessage(QWidget *widget);                // Add widget (e.g., table, tree)
    void addDivisorLine(const QString &stateName);         // Visual separator by phase
    void exportConversationToPdf(const QString &filePath); // Export chat to PDF
    void showTable();                                      // Render LL(1) table
    void showTableForCPrime();
    void updateProgressPanel(); // Update progress panel

    // ====== Visual Feedback / Animations ======================
    void animateLabelPop(QLabel *label);
    void animateLabelColor(QLabel *label, const QColor &flashColor);
    void wrongAnimation();
    void wrongUserResponseAnimation();
    void markLastUserIncorrect();

    // ====== Tree Generation (TeachFirst mode) =================
    void TeachFirstTree(const std::vector<std::string> &symbols,
                        std::unordered_set<std::string> &first_set,
                        int depth,
                        std::unordered_set<std::string> &processing,
                        QTreeWidgetItem *parent);

    std::unique_ptr<TreeNode> buildTreeNode(
        const std::vector<std::string> &symbols,
        std::unordered_set<std::string> &first_set,
        int depth,
        std::vector<std::pair<std::string, std::vector<std::string>>> &active_derivations);

    int computeSubtreeWidth(const std::unique_ptr<TreeNode> &node, int hSpacing);
    void drawTree(const std::unique_ptr<TreeNode> &root,
                  QGraphicsScene *scene,
                  QPointF pos,
                  int hSpacing,
                  int vSpacing);

    void showTreeGraphics(std::unique_ptr<TreeNode> root); // Display derivation tree visually

    // ====== User Response Verification ========================
    bool verifyResponse(const QString &userResponse); // Delegates to current state's verification
    bool verifyResponseForA(const QString &userResponse);
    bool verifyResponseForA1(const QString &userResponse);
    bool verifyResponseForA2(const QString &userResponse);
    bool verifyResponseForB(const QString &userResponse);
    bool verifyResponseForB1(const QString &userResponse);
    bool verifyResponseForB2(const QString &userResponse);
    bool verifyResponseForC(); // C is non-textual (checks internal table)

    // ====== Expected Solutions (Auto-generated) ===============
    QString solution(const std::string &state);
    QStringList solutionForA();
    QString solutionForA1();
    QString solutionForA2();
    QSet<QString> solutionForB();
    QSet<QString> solutionForB1();
    QSet<QString> solutionForB2();

    // ====== Feedback (Corrective Explanations) ================
    QString feedback(); // Delegates by state
    QString feedbackForA();
    QString feedbackForA1();
    QString feedbackForA2();
    QString feedbackForAPrime();
    QString feedbackForB();
    QString feedbackForB1();
    QString feedbackForB2();
    QString feedbackForBPrime();
    QString feedbackForC();
    QString feedbackForCPrime();
    void feedbackForB1TreeWidget();   // TreeWidget of Teach (LL1 TeachFirst)
    void feedbackForB1TreeGraphics(); // Show derivation tree
    QString TeachFollow(const QString &nt);
    QString TeachPredictionSymbols(const QString &ant, const production &conseq);
    QString TeachLL1Table();

    void handleTableSubmission(const QVector<QVector<QString>> &raw, const QStringList &colHeaders);
private slots:
    void on_confirmButton_clicked();
    void on_userResponse_textChanged();

signals:
    void sessionFinished(int cntRight, int cntWrong);

protected:
    void closeEvent(QCloseEvent *event) override
    {
        emit sessionFinished(cntRightAnswers, cntWrongAnswers);
        QWidget::closeEvent(event);
    }

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    // ====== Core Objects ======================================
    Ui::LLTutorWindow *ui;
    Grammar grammar;
    LL1Parser ll1;

    // ====== State & Grammar Tracking ==========================
    State currentState;
    size_t currentRule = 0;
    const unsigned kMaxHighlightTries = 3;
    const unsigned kMaxTotalTries = 5;
    unsigned lltries = 0;
    unsigned cntRightAnswers = 0, cntWrongAnswers = 0;

    using Cell = std::pair<QString, QString>;
    std::vector<Cell> lastWrongCells;
    LLTableDialog *currentDlg = nullptr;

    QVector<QString> sortedNonTerminals;
    QVector<QPair<QString, QVector<QString>>> sortedGrammar;
    QString formattedGrammar;

    QMap<QString, QMap<QString, QVector<QString>>> lltable;
    QVector<QVector<QString>> rawTable;
    QSet<QString> solutionSet;

    // ====== Conversation Logging ==============================
    struct MessageLog
    {
        QString message;
        bool isUser;
        bool isCorrect = true;
        MessageLog(const QString &message, bool isUser)
            : message(message)
            , isUser(isUser)
        {}
        void toggleIsCorrect() { isCorrect = false; }
    };

    QVector<MessageLog> conversationLog;
    QWidget *lastUserMessage = nullptr;
    qsizetype lastUserMessageLogIdx = -1;

    QMap<QString, QString> userCAB;
    QMap<QString, QString> userSIG;
    QMap<QString, QString> userSD;

    // ====== Helper Conversions ================================
    std::vector<std::string> qvectorToStdVector(const QVector<QString> &qvec);
    QVector<QString> stdVectorToQVector(const std::vector<std::string> &vec);
    QSet<QString> stdUnorderedSetToQSet(const std::unordered_set<std::string> &uset);
    std::unordered_set<std::string> qsetToStdUnorderedSet(const QSet<QString> &qset);

    void setupTutorial();

    void fillSortedGrammar(); // Populate sortedGrammar from internal representation

    QPropertyAnimation *m_shakeAnimation
        = nullptr; // For interrupting userResponse animation if they spam enter key

    TutorialManager *tm = nullptr;
};

#endif // LLTUTORWINDOW_H
