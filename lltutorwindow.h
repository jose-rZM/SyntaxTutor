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
    // ====== STRUCTS ==========================================
    struct TreeNode
    {
        QString label;
        std::vector<std::unique_ptr<TreeNode>> children;
    };

    // ====== CONSTRUCTOR / DESTRUCTOR ==========================
    explicit LLTutorWindow(const Grammar &grammar, QWidget *parent = nullptr);
    ~LLTutorWindow();

    // ====== CORE FUNCTIONALITY ================================
    QString generateQuestion();
    void updateState(bool isCorrect);
    QString FormatGrammar(const Grammar &grammar);

    // ====== UI INTERACTION ====================================
    void addMessage(const QString &text, bool isUser);
    void addWidgetMessage(QWidget *widget);
    void addDivisorLine(const QString &stateName);
    void exportConversationToPdf(const QString &filePath);
    void showTable();
    void updateProgressPanel();

    // ====== ANIMATIONS ========================================
    void animateLabelPop(QLabel *label);
    void animateLabelColor(QLabel *label, const QColor &flashColor);
    void wrongAnimation();
    void wrongUserResponseAnimation();

    // ====== FIRST SET TREE (TEACH MODE) =======================
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

    void drawTree(const std::unique_ptr<TreeNode> &root,
                  QGraphicsScene *scene,
                  QPointF pos,
                  int hSpacing,
                  int vSpacing);

    void showTreeGraphics(std::unique_ptr<TreeNode> root);

    // ====== USER RESPONSE VERIFICATION ========================
    bool verifyResponse(const QString &userResponse);
    bool verifyResponseForA(const QString &userResponse);
    bool verifyResponseForA1(const QString &userResponse);
    bool verifyResponseForA2(const QString &userResponse);
    bool verifyResponseForB(const QString &userResponse);
    bool verifyResponseForB1(const QString &userResponse);
    bool verifyResponseForB2(const QString &userResponse);
    bool verifyResponseForC();

    // ====== SOLUTIONS =========================================
    QString solution(const std::string &state);
    QString solutionForA();
    QString solutionForA1();
    QString solutionForA2();
    QSet<QString> solutionForB();
    QSet<QString> solutionForB1();
    QSet<QString> solutionForB2();

    // ====== FEEDBACK ==========================================
    QString feedback();
    QString feedbackForA();
    QString feedbackForA1();
    QString feedbackForA2();
    QString feedbackForAPrime();
    QString feedbackForB();
    QString feedbackForB1();
    void feedbackForB1TreeWidget();
    void feedbackForB1TreeGraphics();
    QString feedbackForB2();
    QString feedbackForBPrime();
    QString feedbackForC();

private slots:
    void on_confirmButton_clicked();
    void on_userResponse_textChanged();

private:
    // ====== UI & CORE OBJECTS =================================
    Ui::LLTutorWindow *ui;
    Grammar grammar;
    LL1Parser ll1;

    // ====== STATE TRACKING ====================================
    State currentState;
    size_t currentRule = 0;
    unsigned lltries = 0;
    unsigned cntRightAnswers = 0;
    unsigned cntWrongAnswers = 0;

    // ====== GRAMMAR DATA ======================================
    QVector<QString> sortedNonTerminals;
    QVector<QPair<QString, QVector<QString>>> sortedGrammar;
    QString formattedGrammar;

    QMap<QString, QMap<QString, QVector<QString>>> lltable;
    QVector<QVector<QString>> rawTable;
    QSet<QString> solutionSet;

    // ====== CONVERSATION LOGGING ==============================
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

    // ====== HELPER FUNCTIONS ==================================
    std::vector<std::string> qvectorToStdVector(const QVector<QString> &qvec);
    QVector<QString> stdVectorToQVector(const std::vector<std::string> &vec);
    QSet<QString> stdUnorderedSetToQSet(const std::unordered_set<std::string> &uset);
    std::unordered_set<std::string> qsetToStdUnorderedSet(const QSet<QString> &qset);

    void fillSortedGrammar();
};
#endif // LLTUTORWINDOW_H
