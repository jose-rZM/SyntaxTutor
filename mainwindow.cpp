#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    factory.Init();
    ui->setupUi(this);
    ui->pushButton->setStyleSheet(R"(
    QPushButton {
        background-color: #00ADB5;
        color: #FFFFFF;
        border: none;
        padding: 12px 24px;
        border-radius: 10px;
        font-size: 16px;
        font-family: 'Noto Sans';
        font-weight: bold;
    }

    QPushButton:hover {
        background-color: #00CED1;
    }

    QPushButton:pressed {
        background-color: #007F86;
    }
)");

    ui->pushButton_2->setStyleSheet(R"(
    QPushButton {
        background-color: #00ADB5;
        color: #FFFFFF;
        border: none;
        padding: 12px 24px;
        border-radius: 10px;
        font-size: 16px;
        font-family: 'Noto Sans';
        font-weight: bold;
    }

    QPushButton:hover {
        background-color: #00CED1;
    }

    QPushButton:pressed {
        background-color: #007F86;
    }
)");

    ui->pushButton->setCursor(Qt::PointingHandCursor);
    ui->pushButton_2->setCursor(Qt::PointingHandCursor);

    ui->lv1Button->setStyleSheet(R"(
    QRadioButton {
        color: #EEEEEE;
        font-size: 14px;
        font-family: 'Noto Sans';
        spacing: 8px;
    }

    QRadioButton::indicator {
        width: 18px;
        height: 18px;
        border-radius: 9px;
        border: 2px solid #888888;
        background-color: #2E2E2E;
    }

    QRadioButton::indicator:checked {
        background-color: #00ADB5;
        border: 2px solid #00ADB5;
    }

    QRadioButton::hover {
        color: #FFFFFF;
    }
)");

    ui->lv2Button->setStyleSheet(R"(
    QRadioButton {
        color: #EEEEEE;
        font-size: 14px;
        font-family: 'Noto Sans';
        spacing: 8px;
    }

    QRadioButton::indicator {
        width: 18px;
        height: 18px;
        border-radius: 9px;
        border: 2px solid #888888;
        background-color: #2E2E2E;
    }

    QRadioButton::indicator:checked {
        background-color: #00ADB5;
        border: 2px solid #00ADB5;
    }

    QRadioButton::hover {
        color: #FFFFFF;
    }
)");

    ui->lv3Button->setStyleSheet(R"(
    QRadioButton {
        color: #EEEEEE;
        font-size: 14px;
        font-family: 'Noto Sans';
        spacing: 8px;
    }

    QRadioButton::indicator {
        width: 18px;
        height: 18px;
        border-radius: 9px;
        border: 2px solid #888888;
        background-color: #2E2E2E;
    }

    QRadioButton::indicator:checked {
        background-color: #00ADB5;
        border: 2px solid #00ADB5;
    }

    QRadioButton::hover {
        color: #FFFFFF;
    }
)");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_lv1Button_clicked(bool checked)
{
    if (checked)
        level = 1;
}

void MainWindow::on_lv2Button_clicked(bool checked)
{
    if (checked)
        level = 2;
}

void MainWindow::on_lv3Button_clicked(bool checked)
{
    if (checked)
        level = 3;
}

void MainWindow::on_pushButton_clicked()
{
    Grammar grammar = factory.GenLL1Grammar(level);
    grammar.Debug();
    this->setEnabled(false);
    LLTutorWindow *tutor = new LLTutorWindow(grammar, this);
    tutor->setAttribute(Qt::WA_DeleteOnClose);
    connect(tutor, &QWidget::destroyed, this, [this]() {
        this->setEnabled(true);
    });
    tutor->show();
}

void MainWindow::on_pushButton_2_clicked()
{
    Grammar grammar = factory.GenSLR1Grammar(level);
    grammar.Debug();
    this->setEnabled(false);
    SLRTutorWindow *tutor = new SLRTutorWindow(grammar, this);
    tutor->setAttribute(Qt::WA_DeleteOnClose);
    connect(tutor, &QWidget::destroyed, this, [this]() {
        this->setEnabled(true);
    });
    tutor->show();
}
