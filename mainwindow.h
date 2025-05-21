#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "backend/grammar.hpp"
#include "backend/grammar_factory.hpp"
#include "lltutorwindow.h"
#include "slrtutorwindow.h"
#include "tutorialmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    unsigned thresholdFor(unsigned level) { return BASE_THRESHOLD * level; }

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

    unsigned userLevel = 1;
    unsigned userScore = 0;
    QSettings settings;

    const unsigned BASE_THRESHOLD = 10;
};
#endif // MAINWINDOW_H
