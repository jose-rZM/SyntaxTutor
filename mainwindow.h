#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "backend/grammar_factory.hpp"
#include "backend/grammar.hpp"
#include "lltutorwindow.h"
#include "tutorialmanager.h"
#include "slrtutorwindow.h"

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

private slots:
    void on_lv1Button_clicked(bool checked);

    void on_lv2Button_clicked(bool checked);

    void on_lv3Button_clicked(bool checked);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_tutorial_clicked();

private:
    Ui::MainWindow *ui;
    GrammarFactory factory;
    int level = 1;
    TutorialManager *tm = nullptr;
};
#endif // MAINWINDOW_H
