#include "mainwindow.h"
#include "tutorialmanager.h"
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

    setupTutorial();
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
#ifdef QT_DEBUG
    grammar.Debug();
#endif
    this->setEnabled(false);
    LLTutorWindow *tutor = new LLTutorWindow(grammar, nullptr, this);
    tutor->setAttribute(Qt::WA_DeleteOnClose);
    connect(tutor, &QWidget::destroyed, this, [this]() { this->setEnabled(true); });
    tutor->show();
}

void MainWindow::on_pushButton_2_clicked()
{
    Grammar grammar = factory.GenSLR1Grammar(level);
#ifdef QT_DEBUG
    grammar.Debug();
#endif
    this->setEnabled(false);
    SLRTutorWindow *tutor = new SLRTutorWindow(grammar, nullptr, this);
    tutor->setAttribute(Qt::WA_DeleteOnClose);
    connect(tutor, &QWidget::destroyed, this, [this]() { this->setEnabled(true); });
    tutor->show();
}

void MainWindow::on_tutorial_clicked()
{
    if (tm) {
        tm->start();
    }
}

void MainWindow::setupTutorial()
{
    tm = new TutorialManager(this);

    // Paso 1: explicación de botones LL(1) y SLR(1)
    tm->addStep(ui->pushButton, "<h3>LL(1)</h3><p>Pulsa para lanzar el tutor LL(1).</p>");
    tm->addStep(ui->pushButton_2, "<h3>SLR(1)</h3><p>Pulsa para lanzar el tutor SLR(1).</p>");

    // Paso 2: explicación de niveles
    tm->addStep(ui->lv1Button,
                "<p>También puedes seleccionar el nivel de dificultad (1, 2 o 3).</p>");

    // Paso 3: LL(1)
    tm->addStep(ui->pushButton, "<p>Ahora se abrirá la ventana LL(1).</p>");

    connect(tm, &TutorialManager::stepStarted, this, [this](int idx) {
        if (idx == 3) {
            // 1) Abre LL
            Grammar grammarLL = factory.GenLL1Grammar(1);
            auto *llTutor = new LLTutorWindow(grammarLL, tm, nullptr);
            llTutor->setAttribute(Qt::WA_DeleteOnClose);
            llTutor->show();

            // 2) Preparar SLR
            connect(tm, &TutorialManager::tutorialFinished, this, [this, llTutor]() {
                llTutor->close();

                disconnect(tm, &TutorialManager::stepStarted, this, nullptr);
                disconnect(tm, &TutorialManager::tutorialFinished, this, nullptr);

                tm->setRootWindow(this);

                tm->clearSteps();
                tm->addStep(ui->pushButton_2, "<h3>SLR(1)</h3><p>Pasemos al tutor SLR(1).</p>");
                tm->addStep(ui->lv3Button,
                            "<p>Esta vez se usará una gramática más compleja (Nivel 3).</p>");
                tm->addStep(ui->pushButton_2, "<p>Ahora se abrirá el tutor SLR(1).</p>");

                // a) Arranca el tutorial de SLR
                tm->start();

                // b) Abrir SLR
                connect(tm, &TutorialManager::stepStarted, this, [this](int idx2) {
                    if (idx2 == 2) {
                        Grammar grammarSLR = factory.GenSLR1Grammar(3);
                        auto *slrTutor = new SLRTutorWindow(grammarSLR, tm, nullptr);
                        slrTutor->setAttribute(Qt::WA_DeleteOnClose);
                        slrTutor->show();
                        QTimer::singleShot(50, [=]() {
                            tm->setRootWindow(slrTutor);
                            tm->nextStep();
                        });
                    }
                });

                // c) Acaba SLR
                connect(tm, &TutorialManager::tutorialFinished, this, [this]() {
                    disconnect(tm, &TutorialManager::stepStarted, this, nullptr);
                    disconnect(tm, &TutorialManager::tutorialFinished, this, nullptr);

                    tm->setRootWindow(this);
                    tm->clearSteps();
                    tm->addStep(this,
                                "<h2>¡Tutorial completado!</h2><p>Ya puedes comenzar a aprender "
                                "sobre analizadores sintácticos. Puedes lanzar el tutorial de "
                                "nuevo presionando el botón correspondiente.</p>");
                    connect(tm, &TutorialManager::tutorialFinished, this, [this]() {
                        tm->clearSteps();
                        delete tm;
                        setupTutorial();
                    });
                    tm->start();
                });
            });

            QTimer::singleShot(50, [this, llTutor]() {
                tm->setRootWindow(llTutor);
                tm->nextStep();
            });
        }
    });
}
