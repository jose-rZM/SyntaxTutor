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
    ui->menuAcercaDe->setObjectName("menuAcercaDe");
    ui->menuAcercaDe->setStyleSheet(R"(
  /* Esto solo afecta al menú cuyo objectName sea 'menuAcercaDe' */
  QMenu#menuAcercaDe::item {
    padding: 6px 24px;
  }
  QMenu#menuAcercaDe::item:selected {
    background-color: #00ADB5;
    color: white;
  }
  QMenu#menuAcercaDe::icon {
    padding-left: 4px;
  }
)");

    setupTutorial();

    connect(this, &MainWindow::userLevelChanged, this, [this](unsigned lvl) {
        int idx = qBound(1, static_cast<int>(lvl), 10) - 1;
        QString c = levelColors[idx];
        QString border = QColor(c).darker(120).name();

        ui->badgeNivel->setStyleSheet(QString(R"(
    QLabel {
    min-width: 24px;
    min-height: 24px;
    padding: 0px 6px;
    font-weight: bold;
    font-size: 12px;
    background-color: %1;
    color: white;
    border-radius: 12px;
    border: 1px solid %2;
    qproperty-alignment: 'AlignCenter';
}
        
    )")
                                          .arg(c)
                                          .arg(border));

        ui->badgeNivel->setText(QString::number(lvl));
        ui->progressBarNivel->setStyleSheet(QString(R"(
    QProgressBar {
        background-color: #2A2A2A;
        border: 1px solid #666666;   
        border-radius: 3px;
        min-height: 5px;
        max-height: 5px;
        text-align: center;
        color: transparent;
    }
    QProgressBar::chunk {
        background-color: %1;
        border-radius: 3px;
        margin: 0px;
    }
)")
                                                .arg(c));
    });

    loadSettings();
}

MainWindow::~MainWindow()
{
    saveSettings();
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

void MainWindow::loadSettings()
{
    setUserLevel(settings.value("gamification/nivel", 1).toUInt());
    userScore = settings.value("gamification/puntos", 0).toUInt();
    ui->labelScore->setText(QString("🏆 Puntos: %1").arg(userScore));

    int percent = qMin(100, static_cast<int>((userScore * 100) / thresholdFor(userLevel())));
    ui->progressBarNivel->setValue(percent);
}

void MainWindow::saveSettings()
{
    settings.setValue("gamification/nivel", userLevel());
    settings.setValue("gamification/puntos", userScore);
}

void MainWindow::handleTutorFinished(int cntRight, int cntWrong)
{
    int delta = (cntRight - cntWrong);
    userScore = static_cast<unsigned>(qMax(0, static_cast<int>(userScore) + delta));
    unsigned thr = thresholdFor(userLevel());

    while (userScore >= thr) {
        userScore -= thr;
        setUserLevel(userLevel() + 1);
        thr = thresholdFor(userLevel());
    }

    ui->labelScore->setText(QString("🏆 Puntos: %1").arg(userScore));

    int percent = qMin(100, static_cast<int>((userScore * 100) / thr));
    ui->progressBarNivel->setValue(percent);

    saveSettings();
}

void MainWindow::on_pushButton_clicked()
{
    Grammar grammar = factory.GenLL1Grammar(level);
    this->setEnabled(false);
    LLTutorWindow *tutor = new LLTutorWindow(grammar, nullptr, this);
    tutor->setAttribute(Qt::WA_DeleteOnClose);
    connect(tutor, &LLTutorWindow::sessionFinished, this, &MainWindow::handleTutorFinished);
    connect(tutor, &QWidget::destroyed, this, [this]() { this->setEnabled(true); });
    tutor->show();
}

void MainWindow::on_pushButton_2_clicked()
{
    Grammar grammar = factory.GenSLR1Grammar(level);
    grammar.TransformToAugmentedGrammar();
    this->setEnabled(false);
    SLRTutorWindow *tutor = new SLRTutorWindow(grammar, nullptr, this);
    tutor->setAttribute(Qt::WA_DeleteOnClose);
    connect(tutor, &SLRTutorWindow::sessionFinished, this, &MainWindow::handleTutorFinished);
    connect(tutor, &QWidget::destroyed, this, [this]() { this->setEnabled(true); });
    tutor->show();
}

void MainWindow::on_tutorial_clicked()
{
    if (tm) {
        delete tm;
        tm = nullptr;
        setupTutorial();
        ui->pushButton->setDisabled(true);
        ui->pushButton_2->setDisabled(true);
        ui->tutorial->setDisabled(true);
        ui->lv1Button->setDisabled(true);
        ui->lv2Button->setDisabled(true);
        ui->lv3Button->setDisabled(true);
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
            Qt::WindowFlags f = llTutor->windowFlags();
            f &= ~Qt::WindowCloseButtonHint;
            llTutor->setWindowFlags(f);
            llTutor->setAttribute(Qt::WA_DeleteOnClose);
            llTutor->show();

            // 2) Preparar SLR
            connect(tm, &TutorialManager::ll1Finished, this, [this, llTutor]() {
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
                        Qt::WindowFlags f = slrTutor->windowFlags();
                        f &= ~Qt::WindowCloseButtonHint;
                        slrTutor->setWindowFlags(f);
                        slrTutor->setAttribute(Qt::WA_DeleteOnClose);
                        slrTutor->show();
                        QTimer::singleShot(50, [this, slrTutor]() {
                            tm->setRootWindow(slrTutor);
                            tm->nextStep();
                        });
                    }
                });

                // c) Acaba SLR
                connect(tm, &TutorialManager::slr1Finished, this, [this]() {
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
                        ui->pushButton->setDisabled(false);
                        ui->pushButton_2->setDisabled(false);
                        ui->tutorial->setDisabled(false);
                        ui->lv1Button->setDisabled(false);
                        ui->lv2Button->setDisabled(false);
                        ui->lv3Button->setDisabled(false);
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
    auto *closeBtn = about.button(QMessageBox::Close);
    if (closeBtn) {
        closeBtn->setCursor(Qt::PointingHandCursor);
        closeBtn->setIcon(QIcon());
    }
    about.setStyleSheet(R"(
      QMessageBox {
        background-color: #1F1F1F;
        color: white;
        font-family: 'Noto Sans';
      }
    QMessageBox QLabel {
        color: #EEEEEE;
    }
      QMessageBox QPushButton {
        background-color: #00ADB5;
        color: #FFFFFF;
        border: none;
        padding: 6px 14px;
        border-radius: 4px;
        font-weight: bold;
      }
      QMessageBox QPushButton:hover {
        background-color: #00CED1;
      }
      QMessageBox QPushButton:pressed {
        background-color: #007F86;
      }
    )");
    about.exec();
}

void MainWindow::on_actionReferencia_LL_1_triggered()
{
    QMessageBox help(this);
    help.setWindowTitle(tr("Referencia rápida LL(1)"));
    help.setTextFormat(Qt::RichText);
    help.setText(R"(
      <h3>Referencia LL(1)</h3>
      <ul>
        <li><b>CAB(X):</b> conjunto de símbolos terminales que comienzan cadenas derivables desde X.</li>
        <li><b>SIG(A):</b> conjunto de terminales que pueden seguir a A en alguna derivación.</li>
  <li><b>Construcción de la Tabla LL(1):</b>
    <ul>
      <li>Para cada producción <code>A → α</code> y cada terminal <code>a</code> tal que 
          <code>a ∈ CAB(α)</code>, asignar  
          <code>Tabla[A][a] = "α"</code>.</li>
      <li>Si <code>ε ∈ CAB(α)</code>, entonces para cada terminal 
          <code>b ∈ SIG(A)</code> asignar  
          <code>Tabla[A][b] = "α"</code>.</li>
      <li>Si <code>ε ∈ CAB(α)</code> y <code>$ ∈ SIG(A)</code>, entonces  
          <code>Tabla[A][$] = "α"</code> (fin de cadena).</li>
      <li><b>Aceptación:</b> Por convención, no se añade entrada especial; el parser termina cuando 
          encuentra <code>$</code> en la pila y en la entrada.</li>
    </ul>
  </li>
        <li><b>Conflictos:</b> Sitios donde CAB(α) ∩ CAB(β) ≠ ∅ o ε ∈ CAB(α) y CAB(β) ∩ SIG(A) ≠ ∅.</li>
      </ul>
    )");
    help.setStandardButtons(QMessageBox::Close);
    auto *closeBtn = help.button(QMessageBox::Close);
    if (closeBtn) {
        closeBtn->setCursor(Qt::PointingHandCursor);
        closeBtn->setIcon(QIcon());
    }
    help.setStyleSheet(R"(
      QMessageBox {
        background-color: #1F1F1F;
        font-family: 'Noto Sans';
      }
    QMessageBox QLabel {
        color: #EEEEEE;
    }
      QMessageBox QPushButton {
        background-color: #00ADB5;
        color: #FFFFFF;
        border: none;
        padding: 6px 14px;
        border-radius: 4px;
        font-weight: bold;
      }
      QMessageBox QPushButton:hover {
        background-color: #00CED1;
      }
      QMessageBox QPushButton:pressed {
        background-color: #007F86;
      }
    )");
    help.exec();
}

void MainWindow::on_actionReferencia_SLR_1_triggered()
{
    QMessageBox help(this);
    help.setWindowTitle(tr("Referencia rápida SLR(1)"));
    help.setTextFormat(Qt::RichText);
    help.setText(R"(
      <h3>Referencia SLR(1)</h3>
      <ul>
        <li><b>Ítems LR(0):</b> producciones con “∙” marcando la posición de análisis.</li>
        <li><b>Cierre( I ):</b> añadir ítems B → ∙ γ para cada ítem A → α ∙ B β. Repetir hasta que no se añadan más.</li>
        <li><b>Goto( I, X ) o δ( I, X ):</b> desplazar “∙” sobre X en todos los ítems de I y calcular su cierre.</li>
<li><b>Tabla SLR(1):</b>
  <ul>
    <li><b>Acciones (Action):</b>  
      Para cada estado I y cada terminal a:
      <ul>
        <li>Si existe el ítem <code>A → α∙aβ</code> en I, entonces <code>Action[I,a] = s<sub>j</sub></code> (shift al estado j = Goto(I,a)).</li>
        <li>Si existe el ítem <code>A → α∙</code> en I, entonces <code>Action[I,a] = r<sub>k</sub></code> (reduce usando la producción k = A→α) <em>para todo</em> <code>a ∈ SIG(A)</code>.</li>
        <li><code>Action[I,$] = acc</code> si <code>S' → S∙</code> está en I (aceptación).</li>
      </ul>
    </li>
    <li><b>Transiciones (Goto):</b>  
      Para cada estado I y cada no terminal A:
      <ul>
        <li>Si <code>Goto(I,A) = J</code>, entonces <code>Goto[I,A] = J</code>.</li>
      </ul>
    </li>
  </ul>
</li>
      </ul>
    )");
    help.setStandardButtons(QMessageBox::Close);
    auto *closeBtn = help.button(QMessageBox::Close);
    if (closeBtn) {
        closeBtn->setCursor(Qt::PointingHandCursor);
        closeBtn->setIcon(QIcon());
    }
    help.setStyleSheet(R"(
      QMessageBox {
        background-color: #1F1F1F;
        color: white;
        font-family: 'Noto Sans';
      }
    QMessageBox QLabel {
        color: #EEEEEE;
    }
      QMessageBox QPushButton {
        background-color: #00ADB5;
        color: #FFFFFF;
        border: none;
        padding: 6px 14px;
        border-radius: 4px;
        font-weight: bold;
      }
      QMessageBox QPushButton:hover {
        background-color: #00CED1;
      }
      QMessageBox QPushButton:pressed {
        background-color: #007F86;
      }
    )");
    help.exec();
}
