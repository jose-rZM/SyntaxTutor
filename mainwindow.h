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
    "#2C3E50", // 1: Navy oscuro
    "#2980B9", // 2: Azul brillante
    "#16A085", // 3: Teal
    "#27AE60", // 4: Verde esmeralda
    "#8E44AD", // 5: Púrpura medio
    "#9B59B6", // 6: Púrpura claro
    "#E67E22", // 7: Naranja
    "#D35400", // 8: Naranja oscuro
    "#CD7F32", // 9: Bronce
    "#FFD700"  // 10: Oro puro
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
        unsigned clamped = qMin(lvl, MAX_LEVEL);
        if (m_userLevel == clamped)
            return;
        m_userLevel = clamped;
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

    void on_idiom_clicked();

signals:
    void userLevelChanged(unsigned lvl);

    void userLevelUp(unsigned newLevel);

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

    static constexpr unsigned MAX_LEVEL = 10;
    static constexpr unsigned MAX_SCORE = 999;

    unsigned m_userLevel = 1;
    unsigned userScore = 0;
    QSettings settings;

    const unsigned BASE_THRESHOLD = 10;
};
#endif // MAINWINDOW_H
