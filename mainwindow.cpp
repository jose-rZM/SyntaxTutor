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

    tm = new TutorialManager(this);

    // Paso 1: explicación de botones LL(1) y SLR(1)
    tm->addStep(ui->pushButton, "<h3>LL(1)</h3><p>Pulsa para lanzar el tutor LL(1).</p>");
    tm->addStep(ui->pushButton_2, "<h3>SLR(1)</h3><p>Pulsa para lanzar el tutor SLR(1).</p>");

    // Paso 2: explicación de niveles
    tm->addStep(ui->lv1Button, "<p>Selecciona Nivel 1, 2 o 3 para la complejidad.</p>");

    // Paso 3: vamos a la ventana LL(1)
    tm->addStep(ui->pushButton, "<p>Ahora se abrirá la ventana LL(1).</p>");

    // Paso 4: cuando llegue a este step, automáticamente abrimos la ventana
    // --- dentro de MainWindow::MainWindow(...) justo tras añadir los 4 pasos de LL ---
    // ... dentro de tu connect(tm, stepStarted) para idx == 3, tras LL(1)

    connect(tm, &TutorialManager::stepStarted, this, [=](int idx) {
        if (idx == 3) {
            // 1) Abrimos LL(1) como ventana independiente
            Grammar grammarLL = factory.GenLL1Grammar(1);
            auto *llTutor = new LLTutorWindow(grammarLL, tm, nullptr);
            llTutor->setAttribute(Qt::WA_DeleteOnClose);
            llTutor->show();

            // 2) Al acabar LL(1), preparamos SLR(1)
            connect(tm, &TutorialManager::tutorialFinished, this, [=]() {
                // a) Cerramos LL(1)
                llTutor->close();

                // b) Desconecto TODO lo viejo
                disconnect(tm, &TutorialManager::stepStarted, this, nullptr);
                disconnect(tm, &TutorialManager::tutorialFinished, this, nullptr);

                // c) ROOT vuelve a MainWindow
                tm->setRootWindow(this);

                // d) Limpio pasos y añado los de SLR
                tm->clearSteps();
                tm->addStep(ui->pushButton_2,
                            "<h3>SLR(1)</h3><p>Ahora veremos el tutor SLR(1).</p>");
                tm->addStep(ui->lv3Button, "<p>Selecciona Nivel 3 para un ejemplo avanzado.</p>");
                tm->addStep(ui->pushButton_2, "<p>En breve abriremos la ventana SLR(1).</p>");

                // e) Arranco el tutorial de SLR
                tm->start();

                // f) En el paso 2, abro SLR(1) como **ventana independiente**:
                connect(tm, &TutorialManager::stepStarted, this, [this](int idx2) {
                    if (idx2 == 2) {
                        ui->lv3Button->setChecked(true);
                        Grammar grammarSLR = factory.GenSLR1Grammar(3);
                        auto *slrTutor = new SLRTutorWindow(grammarSLR, tm, nullptr);
                        slrTutor->setAttribute(Qt::WA_DeleteOnClose);
                        slrTutor->show();

                        // Reasigno el ROOT al tutor SLR y avanzo un paso
                        tm->setRootWindow(slrTutor);
                        tm->nextStep();
                    }
                });

                // g) Y por último, tras acabar SLR(1), muestras un paso final…
                connect(tm, &TutorialManager::tutorialFinished, this, [this]() {
                    // limpio viejas conexiones otra vez
                    disconnect(tm, &TutorialManager::stepStarted, this, nullptr);
                    disconnect(tm, &TutorialManager::tutorialFinished, this, nullptr);

                    tm->setRootWindow(this);
                    tm->clearSteps();
                    tm->addStep(this, "<h2>¡Tutorial completado!</h2><p>Has visto LL → SLR.</p>");
                    connect(tm, &TutorialManager::tutorialFinished, this, [this]() {
                        tm->clearSteps(); // ¡todo listo para empezar de cero!
                        delete tm;
                        tm = new TutorialManager(this);
                    });
                    tm->start();
                });
            });

            // 3) Ahora reasigno root a LLTutor y avanzo al siguiente paso del tutorial
            QTimer::singleShot(50, [=]() {
                tm->setRootWindow(llTutor);
                tm->nextStep();
            });
        }
    });
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
