#include "mainwindow.h"
#include "tutorialmanager.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    factory.Init();
    ui->setupUi(this);
    Qt::WindowFlags f = windowFlags();
    f &= ~Qt::WindowMaximizeButtonHint;
    setWindowFlags(f);
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
        delete tm;
        tm = nullptr;
        setupTutorial();
        tm->start();
    }
}

void MainWindow::setupTutorial()
{
    tm = new TutorialManager(this);

    // Paso 1: explicación de botones LL(1) y SLR(1)
    tm->addStep(ui->pushButton, "<h3>LL(1)</h3><p>Con este botón puedes lanzar el tutor LL(1).</p>");
    tm->addStep(ui->pushButton_2, "<h3>SLR(1)</h3><p>Con este, el SLR(1).</p>");

    // Paso 2: explicación de niveles
    tm->addStep(ui->lv1Button,
                "<p>También puedes seleccionar el nivel de dificultad (1, 2 o 3). La dificultad "
                "repercute en la longitud de la gramática.</p>");

    // Paso 3: LL(1)
    tm->addStep(ui->pushButton, "<p>Ahora se abrirá la ventana LL(1).</p>");
    tm->addStep(nullptr, "");

    connect(tm, &TutorialManager::stepStarted, this, [this](int idx) {
        if (idx == 4) {
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
                tm->addStep(nullptr, "");
                // a) Arranca el tutorial de SLR
                tm->start();

                // b) Abrir SLR
                connect(tm, &TutorialManager::stepStarted, this, [this](int idx2) {
                    if (idx2 == 3) {
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
                    tm->addStep(
                        this,
                        "<h2>¡Tutorial completado!</h2><p>Ya puedes comenzar a practicar.</p>");
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

#include <QMessageBox>
#include <QPixmap>

void MainWindow::on_actionSobre_la_aplicaci_n_triggered()
{
    QMessageBox about(this);
    about.setWindowTitle(tr("Sobre la aplicación"));
    QPixmap pix(":/resources/syntaxtutor.png");
    about.setIconPixmap(pix.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    about.setTextFormat(Qt::RichText);
    about.setText("<h2>SyntaxTutor</h2>"
                  "<p><b>Versión: 1.0</b> "
                  + qApp->applicationVersion()
                  + "</p>"
                    "<p>Trabajo Fin de Grado – Analizador sintáctico interactivo.</p>"
                    "<p><b>Autor:</b> José R.</p>"
                    "<p><b>Licencia:</b> GPLv3</p>"
                    "<p>Desarrollado con <a href='https://www.qt.io/'>Qt 6</a> y C++20.</p>"
                    "<p><a href='https://github.com/jose-rZM/SyntaxTutor'>GitHub - jose-rZM</a>"
                    "<p>2025 Universidad de Málaga</p>");

    about.setStandardButtons(QMessageBox::Close);
    about.exec();
}
