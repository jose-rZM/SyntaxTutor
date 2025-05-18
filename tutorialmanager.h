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
    QWidget *target;  // el widget a resaltar
    QString htmlText; // contenido HTML explicativo
};

class TutorialManager : public QObject
{
    Q_OBJECT
public:
    TutorialManager(QWidget *rootWindow);
    void addStep(QWidget *target, const QString &htmlText);
    void start(); // comienza el tour
    void setRootWindow(QWidget *newRoot);
    void clearSteps();
    void hideOverlay();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

signals:
    /// Se emite justo antes de mostrar el overlay del paso `index`
    void stepStarted(int index);
    /// Se emite cuando se acaba el tutorial
    void tutorialFinished();

public slots:
    void nextStep();

private:
    void showOverlay();
    void repositionOverlay();

    QWidget *m_root; // mainWindow o la ventana que arranque el tour
    QVector<TutorialStep> m_steps;
    int m_index = -1;

    // overlays y frame de resalte
    QWidget *m_overlay = nullptr;
    QFrame *m_highlight = nullptr;
    QTextBrowser *m_textBox = nullptr;
    QPushButton *m_nextBtn = nullptr;
};

#endif // TUTORIALMANAGER_H
