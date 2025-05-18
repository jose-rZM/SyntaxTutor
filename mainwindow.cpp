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
    if (tm)
        delete tm;

    tm = new TutorialManager(this);

    // Paso 1: explicación de botones LL(1) y SLR(1)
    tm->addStep(ui->pushButton, "<h3>LL(1)</h3><p>Pulsa para lanzar el tutor LL(1).</p>");
    tm->addStep(ui->pushButton_2, "<h3>SLR(1)</h3><p>Pulsa para lanzar el tutor SLR(1).</p>");

    // Paso 2: explicación de niveles
    tm->addStep(ui->lv1Button,
                "<p>También puedes seleccionar el nivel de dificultad (1, 2 o 3).</p>");

    // Paso 3: LL(1)
    tm->addStep(ui->pushButton, "<p>Ahora se abrirá la ventana LL(1).</p>");
    tm->addStep(nullptr, "");

    connect(tm, &TutorialManager::stepStarted, this, [this](int idx) {
        if (idx == 4) {
            // 1) Abrir LLTutorWindow…
            Grammar grammarLL = factory.GenLL1Grammar(1);
            auto *llTutor = new LLTutorWindow(grammarLL, tm, nullptr);
            llTutor->setAttribute(Qt::WA_DeleteOnClose);
            llTutor->show();

            // Detectar cierre manual de LL
            QMetaObject::Connection llManualCloseConn = connect(llTutor,
                                                                &QObject::destroyed,
                                                                this,
                                                                [=]() { restartTutorial(); });

            // 2) Cuando termine LL(1), preparamos SLR(1)
            connect(tm, &TutorialManager::tutorialFinished, this, [=]() mutable {
                // a) Desconecto la señal de cierre manual
                disconnect(llManualCloseConn);
                // b) Cierro LL de forma controlada
                llTutor->close();

                // c) Limpiar conexiones de tm antiguas
                disconnect(tm, nullptr, this, nullptr);
                disconnect(tm, nullptr, tm, nullptr);

                // d) Restaurar MainWindow como root
                tm->setRootWindow(this);

                // e) Pasos de SLR
                tm->clearSteps();
                tm->addStep(ui->pushButton_2, "<h3>SLR(1)</h3><p>Pasemos al tutor SLR(1).</p>");
                tm->addStep(ui->lv3Button,
                            "<p>Esta vez se usará una gramática más compleja (Nivel 3).</p>");
                tm->addStep(ui->pushButton_2, "<p>Ahora se abrirá el tutor SLR(1).</p>");
                tm->addStep(nullptr, "");

                // f) Arranco el tutorial de SLR
                tm->start();

                // g) Conexiones para el bloque SLR:
                QMetaObject::Connection slrStepConn;
                QMetaObject::Connection slrFinishConn;

                // g.1) Al llegar al paso 3 abrimos SLR
                slrStepConn = connect(tm, &TutorialManager::stepStarted, this, [=](int idx2) mutable {
                    if (idx2 == 3) {
                        // i) Abrimos SLR
                        Grammar grammarSLR = factory.GenSLR1Grammar(3);
                        auto *slrTutor = new SLRTutorWindow(grammarSLR, tm, nullptr);
                        slrTutor->setAttribute(Qt::WA_DeleteOnClose);
                        slrTutor->show();

                        // ii) Detectar cierre manual de SLR
                        QMetaObject::Connection slrManualCloseConn = connect(slrTutor,
                                                                             &QObject::destroyed,
                                                                             this,
                                                                             [=]() {
                                                                                 restartTutorial();
                                                                             });

                        // iii) Cuando termine SLR(1)
                        slrFinishConn
                            = connect(tm, &TutorialManager::tutorialFinished, this, [=]() mutable {
                                  // – Desconecto cierre manual
                                  disconnect(slrManualCloseConn);
                                  // – Cierro SLR de forma controlada
                                  slrTutor->close();
                                  // – Desconecto handlers de este bloque
                                  disconnect(slrStepConn);
                                  disconnect(slrFinishConn);
                                  // – Restaurar MainWindow y preparar paso final
                                  tm->setRootWindow(this);
                                  tm->clearSteps();
                                  tm->addStep(this,
                                              "<h2>¡Tutorial completado!</h2>"
                                              "<p>Ya puedes comenzar a aprender sobre "
                                              "analizadores sintácticos. "
                                              "Pulsa el botón 'Tutorial' para repetir "
                                              "cuando quieras.</p>");
                                  // – Al acabar este paso, reiniciamos totalmente
                                  connect(tm, &TutorialManager::tutorialFinished, this, [=]() {
                                      restartTutorial();
                                  });
                                  tm->start();
                              });

                        // iv) Posponer highlight hasta que SLR esté mapeada
                        QTimer::singleShot(50, [=]() {
                            tm->setRootWindow(slrTutor);
                            tm->nextStep();
                        });
                    }
                });
            });

            // 3) Posponer el highlight de LL
            QTimer::singleShot(50, [=]() {
                tm->setRootWindow(llTutor);
                tm->nextStep();
            });
        }
    });
}

void MainWindow::restartTutorial()
{
    if (tm) {
        // 0) Si hay un overlay vivo, lo quitamos
        tm->hideOverlay();
        tm->clearSteps();

        // 1) Desconectamos TODO lo que pudiera quedarnos
        disconnect(tm, nullptr, this, nullptr);
        disconnect(tm, nullptr, tm, nullptr);
    }
    // 3) Volvemos a configurar un tutorial nuevo
    setupTutorial();
}
