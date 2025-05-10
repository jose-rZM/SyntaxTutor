#ifndef LLTUTORWINDOW_H
#define LLTUTORWINDOW_H

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

    explicit LLTutorWindow(const Grammar& grammar, QWidget *parent = nullptr);
    ~LLTutorWindow();
    QString FormatGrammar(const Grammar& grammar);

    void addMessage(const QString& text, bool isUser);
    void addDivisorLine(const QString &stateName);
    void exportConversationToPdf(const QString &filePath);
    void showTable();
    void updateProgressPanel();
    void wrongAnimation();
    void wrongUserResponseAnimation();
    void animateLabelPop(QLabel *label);
    void animateLabelColor(QLabel *label, const QColor &flashColor);

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
    QString feedbackForC();
    // END FEEDBACK ------------------------------------------

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
    void fillSortedGrammar();
    // END HELPER FUNCTIONS ----------------

    Ui::LLTutorWindow *ui;
    Grammar grammar;
    LL1Parser ll1;

    unsigned cntRightAnswers = 0, cntWrongAnswers = 0;

    State currentState;
    QVector<QPair<QString, QVector<QString>>> sortedGrammar;
    QString formattedGrammar;
    QMap<QString, QMap<QString, QVector<QString>>> lltable;
    QVector<QVector<QString>> rawTable;
    QSet<QString> solutionSet;
    size_t currentRule = 0;
    unsigned lltries = 0;

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
};

#endif // LLTUTORWINDOW_H
