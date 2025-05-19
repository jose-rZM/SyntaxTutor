#ifndef TUTORIALMANAGER_H
#define TUTORIALMANAGER_H

#include <QEvent>
#include <QFrame>
#include <QObject>
#include <QPushButton>
#include <QString>
#include <QTextBrowser>
#include <QVector>
#include <QWidget>

struct TutorialStep
{
    QWidget *target;
    QString htmlText;
};

class TutorialManager : public QObject
{
    Q_OBJECT
public:
    TutorialManager(QWidget *rootWindow);
    void addStep(QWidget *target, const QString &htmlText);
    void start();
    void setRootWindow(QWidget *newRoot);
    void clearSteps();
    void hideOverlay();
    void finishLL1();
    void finishSLR1();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

signals:
    void stepStarted(int index);

    void tutorialFinished();

    void ll1Finished();

    void slr1Finished();

public slots:
    void nextStep();

private:
    void showOverlay();
    void repositionOverlay();

    QWidget *m_root;
    QVector<TutorialStep> m_steps;
    int m_index = -1;

    QWidget *m_overlay = nullptr;
    QFrame *m_highlight = nullptr;
    QTextBrowser *m_textBox = nullptr;
    QPushButton *m_nextBtn = nullptr;
};

#endif // TUTORIALMANAGER_H
