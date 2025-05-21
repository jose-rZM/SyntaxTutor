#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "backend/grammar.hpp"
#include "backend/grammar_factory.hpp"
#include "lltutorwindow.h"
#include "slrtutorwindow.h"
#include "tutorialmanager.h"

static const QVector<QString> levelColors = {
    "#00B1A7", // 1
    "#1ABC9C", // 2
    "#2ECC71", // 3
    "#F1C40F", // 4
    "#F4D03F", // 5
    "#F39C12", // 6
    "#E67E22", // 7
    "#D35400", // 8
    "#E74C3C", // 9
    "#FFD700"  // 10
};

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(unsigned userLevel READ userLevel WRITE setUserLevel NOTIFY userLevelChanged)

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    unsigned thresholdFor(unsigned level) { return BASE_THRESHOLD * level; }
    unsigned userLevel() const { return m_userLevel; };
    void setUserLevel(unsigned lvl)
    {
        if (m_userLevel == lvl)
            return;
        m_userLevel = lvl;
        emit userLevelChanged(lvl);
    }
private slots:
    void on_lv1Button_clicked(bool checked);

    void on_lv2Button_clicked(bool checked);

    void on_lv3Button_clicked(bool checked);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_tutorial_clicked();

    void on_actionSobre_la_aplicaci_n_triggered();

    void on_actionReferencia_LL_1_triggered();

    void on_actionReferencia_SLR_1_triggered();

signals:
    void userLevelChanged(unsigned lvl);

private:
    void setupTutorial();

    void restartTutorial();

    void handleTutorFinished(int cntRight, int cntWrong);

    void saveSettings();

    void loadSettings();

    Ui::MainWindow *ui;
    GrammarFactory factory;
    int level = 1;
    TutorialManager *tm = nullptr;

    unsigned m_userLevel = 1;
    unsigned userScore = 0;
    QSettings settings;

    const unsigned BASE_THRESHOLD = 10;
};
#endif // MAINWINDOW_H
