#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    factory.Init();
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_lv1Button_clicked(bool checked)
{
    if (checked) level = 1;
}


void MainWindow::on_lv2Button_clicked(bool checked)
{
    if (checked) level = 2;
}


void MainWindow::on_lv3Button_clicked(bool checked)
{
    if (checked) level = 3;
}


void MainWindow::on_pushButton_clicked()
{
    Grammar grammar = factory.GenLL1Grammar(level);
    grammar.Debug();
    LLTutorWindow *tutor = new LLTutorWindow(grammar, this);
    tutor->show();
}


void MainWindow::on_pushButton_2_clicked()
{
    factory.GenSLR1Grammar(level);
}

