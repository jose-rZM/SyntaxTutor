/*
 * SyntaxTutor - Interactive Tutorial About Syntax Analyzers
 * Copyright (C) 2025 Jose R. (jose-rzm)
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "tutorialmanager.h"
#include "ui_mainwindow.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QStackedWidget>
#include <QTextBrowser>
#include <QVBoxLayout>

namespace {

#ifdef SYNTAXTUTOR_TESTING
constexpr auto kSettingsOrg = "UMA-Test";
constexpr auto kSettingsApp = "SyntaxTutor-Test";
#else
constexpr auto kSettingsOrg = "UMA";
constexpr auto kSettingsApp = "SyntaxTutor";
#endif

void showInfoDialog(QWidget* parent, const QString& windowTitle,
                    const QString& eyebrow, const QString& title,
                    const QString& html) {
    auto* dialog = new QDialog(parent);
    dialog->setObjectName("infoDialog");
    dialog->setWindowTitle(windowTitle);
    dialog->setModal(true);
    dialog->resize(620, 480);

    auto* layout = new QVBoxLayout(dialog);
    layout->setSpacing(14);
    layout->setContentsMargins(24, 22, 24, 18);

    auto* eyebrowLabel = new QLabel(eyebrow, dialog);
    eyebrowLabel->setObjectName("infoDialogEyebrow");

    auto* titleLabel = new QLabel(title, dialog);
    titleLabel->setObjectName("infoDialogTitle");
    titleLabel->setWordWrap(true);

    auto* content = new QTextBrowser(dialog);
    content->setObjectName("infoDialogContent");
    content->setOpenExternalLinks(true);
    content->setFrameShape(QFrame::NoFrame);
    content->setHtml(html);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, dialog);
    if (auto* closeBtn = buttons->button(QDialogButtonBox::Close)) {
        closeBtn->setText(QObject::tr("Cerrar"));
        closeBtn->setCursor(Qt::PointingHandCursor);
        closeBtn->setProperty("role", "primary");
        closeBtn->setIcon(QIcon());
    }

    QObject::connect(buttons, &QDialogButtonBox::rejected, dialog,
                     &QDialog::accept);

    layout->addWidget(eyebrowLabel);
    layout->addWidget(titleLabel);
    layout->addWidget(content, 1);
    layout->addWidget(buttons);

    dialog->exec();
    dialog->deleteLater();
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      settings(kSettingsOrg, kSettingsApp) {
    factory.Init();
    ui->setupUi(this);
    ui->homeEyebrow->setText(tr("Tutores interactivos"));
    ui->homeTitle->setText(tr("Elige cómo quieres practicar"));
    ui->homeSubtitle->setText(
        tr("Inicia un tutor y ajusta la "
           "dificultad de la gramática antes de empezar."));
    ui->pushButton->setText(tr("LL(1)"));
    ui->pushButton_2->setText(tr("SLR(1)"));
    ui->tutorial->setText(tr("Tutorial"));
    ui->difficultyTitle->setText(tr("Dificultad"));
    ui->lv1Button->setText(tr("Nivel 1"));
    ui->lv2Button->setText(tr("Nivel 2"));
    ui->lv3Button->setText(tr("Nivel 3"));
    ui->idiom->setText(tr("Idioma"));
    defaultWindowTitle = windowTitle();

    homePage = takeCentralWidget();
    stack    = new QStackedWidget(this);
    stack->setContentsMargins(0, 0, 0, 0);
    setCentralWidget(stack);
    stack->addWidget(homePage);
    stack->setCurrentWidget(homePage);

    Qt::WindowFlags f = windowFlags();
    f &= ~Qt::WindowMaximizeButtonHint;
    setWindowFlags(f);

    ui->pushButton->setCursor(Qt::PointingHandCursor);
    ui->pushButton_2->setCursor(Qt::PointingHandCursor);
    ui->menuAcercaDe->setObjectName("menuAcercaDe");

    setupTutorial();

    connect(this, &MainWindow::userLevelChanged, this,
            &MainWindow::applyLevelStyling);

    connect(this, &MainWindow::userLevelUp, this, [this]() {
        QPropertyAnimation* anim =
            new QPropertyAnimation(ui->badgeNivel, "geometry");
        QRect original = ui->badgeNivel->geometry();
        QRect enlarged = original.adjusted(-4, -4, 4, 4);

        anim->setDuration(150);
        anim->setKeyValueAt(0, original);
        anim->setKeyValueAt(0.5, enlarged);
        anim->setKeyValueAt(1, original);
        anim->setEasingCurve(QEasingCurve::OutBack);
        anim->start(QAbstractAnimation::DeleteWhenStopped);

        auto* glow = new QGraphicsDropShadowEffect(ui->badgeNivel);
        glow->setColor(QColor(Qt::white).lighter(130));
        glow->setOffset(0);
        glow->setBlurRadius(25);
        ui->badgeNivel->setGraphicsEffect(glow);

        QTimer::singleShot(1000, glow, [this]() {
            ui->badgeNivel->setGraphicsEffect(nullptr);
        });

        QLabel* floatLabel =
            new QLabel(tr("+1 Nivel"), ui->badgeNivel->parentWidget());
        floatLabel->setStyleSheet(R"(
    QLabel {
        font-weight: bold;
        font-size: 20px;
        background: transparent;
    }
)");
        floatLabel->adjustSize();

        QPoint badgePos   = ui->badgeNivel->geometry().topLeft();
        int    badgeWidth = ui->badgeNivel->width();
        int    x = badgePos.x() + badgeWidth / 2 - floatLabel->width() / 2;
        int    y = badgePos.y() - 10;
        floatLabel->move(x, y);
        floatLabel->show();

        QStringList rainbowColors = {"#FF0000", "#FF7F00", "#FFFF00", "#00FF00",
                                     "#0000FF", "#4B0082", "#8F00FF"};
        auto*       rainbowTimer  = new QTimer(floatLabel);
        connect(rainbowTimer, &QTimer::timeout, floatLabel,
                [floatLabel, rainbowColors, colorIndex = 0]() mutable {
                    QString color =
                        rainbowColors[colorIndex % rainbowColors.size()];
                    floatLabel->setStyleSheet(QString(R"(
        QLabel {
            font-weight: bold;
            font-size: 20px;
            background: transparent;
            color: %1;
        }
    )")
                                                  .arg(color));
                    colorIndex++;
                });
        rainbowTimer->start(100);
        QPropertyAnimation* moveAnim =
            new QPropertyAnimation(floatLabel, "pos");
        moveAnim->setDuration(1500);
        moveAnim->setStartValue(QPoint(x, y + 10));
        moveAnim->setEndValue(QPoint(x, y + 40));
        moveAnim->setEasingCurve(QEasingCurve::OutQuad);

        QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(floatLabel);
        floatLabel->setGraphicsEffect(effect);

        QPropertyAnimation* fadeAnim =
            new QPropertyAnimation(effect, "opacity");
        fadeAnim->setDuration(1500);
        fadeAnim->setStartValue(1.0);
        fadeAnim->setEndValue(0.0);

        connect(fadeAnim, &QPropertyAnimation::finished, floatLabel,
                [floatLabel, rainbowTimer]() {
                    rainbowTimer->stop();
                    floatLabel->deleteLater();
                });

        moveAnim->start(QAbstractAnimation::DeleteWhenStopped);
        fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
    });

#ifdef QT_DEBUG
    auto* debugShortcutLvlUp =
        new QShortcut(QKeySequence("Ctrl+Shift+U"), this);
    connect(debugShortcutLvlUp, &QShortcut::activated, this, [this]() {
        setUserLevel(userLevel() + 1);
        emit userLevelUp(userLevel() + 1);
    });

    auto* debugShortcutLvlDown =
        new QShortcut(QKeySequence("Ctrl+Shift+D"), this);
    connect(debugShortcutLvlDown, &QShortcut::activated, this, [this]() {
        if (userLevel() > 1)
            setUserLevel(userLevel() - 1);
    });
#endif

    loadSettings();
}

MainWindow::~MainWindow() {
    saveSettings();
    cleanupTutorPages();
    delete tm;
    delete ui;
}

void MainWindow::applyLevelStyling(unsigned lvl) {
    int     idx    = qBound(1, static_cast<int>(lvl), 10) - 1;
    QString c      = levelColors[idx];

    ui->badgeNivel->setStyleSheet(QString(R"(
    QLabel {
    min-width: 28px;
    min-height: 24px;
    padding: 0px 10px;
    font-weight: 700;
    font-size: 12px;
    background-color: rgba(%1, %2, %3, 0.18);
    color: %4;
    border-radius: 12px;
    border: none;
    qproperty-alignment: 'AlignCenter';
}       
    )")
                                      .arg(QColor(c).red())
                                      .arg(QColor(c).green())
                                      .arg(QColor(c).blue())
                                      .arg(c));

    ui->badgeNivel->setText(QString::number(lvl));
    ui->progressBarNivel->setStyleSheet(QString(R"(
    QProgressBar {
        background-color: #2A2E30;
        border: none;
        border-radius: 4px;
        min-height: 8px;
        max-height: 8px;
        text-align: center;
        color: transparent;
    }
    QProgressBar::chunk {
        background-color: %1;
        border-radius: 4px;
        margin: 0px;
    }
)")
                                            .arg(c));
}

void MainWindow::on_lv1Button_clicked(bool checked) {
    if (checked)
        level = 1;
}

void MainWindow::on_lv2Button_clicked(bool checked) {
    if (checked)
        level = 2;
}

void MainWindow::on_lv3Button_clicked(bool checked) {
    if (checked)
        level = 3;
}

void MainWindow::loadSettings() {
    unsigned prevLevel = userLevel();
    setUserLevel(settings.value("gamification/level", 1).toUInt());
    if (prevLevel == userLevel()) {
        applyLevelStyling(userLevel());
    }
    userScore = settings.value("gamification/score", 0).toUInt();
    ui->labelScore->setText(tr("Puntos: %1").arg(userScore));

    if (userLevel() >= MAX_LEVEL) {
        ui->progressBarNivel->setEnabled(false);
        ui->progressBarNivel->setValue(100);
    } else {
        ui->progressBarNivel->setEnabled(true);
        unsigned thr     = thresholdFor(userLevel());
        int      percent = qMin(100, static_cast<int>((userScore * 100) / thr));
        ui->progressBarNivel->setValue(percent);
    }
}

void MainWindow::saveSettings() {
    settings.setValue("gamification/level", userLevel());
    settings.setValue("gamification/score", userScore);
}

void MainWindow::setNavigationEnabled(bool enabled) {
    ui->pushButton->setDisabled(!enabled);
    ui->pushButton_2->setDisabled(!enabled);
    ui->tutorial->setDisabled(!enabled);
    ui->lv1Button->setDisabled(!enabled);
    ui->lv2Button->setDisabled(!enabled);
    ui->lv3Button->setDisabled(!enabled);
}

void MainWindow::showHomePage() {
    if (stack && homePage) {
        stack->setCurrentWidget(homePage);
    }
    setWindowTitle(defaultWindowTitle);
}

void MainWindow::cleanupTutorPages() {
    if (tm) {
        tm->setRootWindow(homePage);
    }

    if (llTutorPage) {
        stack->removeWidget(llTutorPage);
        llTutorPage->deleteLater();
        llTutorPage = nullptr;
    }

    if (slrTutorPage) {
        stack->removeWidget(slrTutorPage);
        slrTutorPage->deleteLater();
        slrTutorPage = nullptr;
    }
}

LLTutorWindow* MainWindow::startLLTutor(const Grammar& grammar,
                                        TutorialManager* tutorialManager) {
    if (llTutorPage) {
        stack->removeWidget(llTutorPage);
        llTutorPage->deleteLater();
    }

    llTutorPage = new LLTutorWindow(grammar, tutorialManager, stack);
    stack->addWidget(llTutorPage);
    stack->setCurrentWidget(llTutorPage);
    setWindowTitle(tr("LL(1)"));

    connect(llTutorPage, &LLTutorWindow::exitRequested, this,
            [this, tutorialManager](bool applyResults, int cntRight,
                                    int cntWrong) {
                if (applyResults && tutorialManager == nullptr) {
                    handleTutorFinished(cntRight, cntWrong);
                }

                if (tutorialManager != nullptr) {
                    abortTutorialFlow();
                    return;
                }

                showHomePage();
                if (llTutorPage) {
                    stack->removeWidget(llTutorPage);
                    llTutorPage->deleteLater();
                    llTutorPage = nullptr;
                }
            });

    return llTutorPage;
}

SLRTutorWindow* MainWindow::startSLRTutor(const Grammar& grammar,
                                          TutorialManager* tutorialManager) {
    if (slrTutorPage) {
        stack->removeWidget(slrTutorPage);
        slrTutorPage->deleteLater();
    }

    slrTutorPage = new SLRTutorWindow(grammar, tutorialManager, stack);
    stack->addWidget(slrTutorPage);
    stack->setCurrentWidget(slrTutorPage);
    setWindowTitle(tr("SLR(1)"));

    connect(slrTutorPage, &SLRTutorWindow::exitRequested, this,
            [this, tutorialManager](bool applyResults, int cntRight,
                                    int cntWrong) {
                if (applyResults && tutorialManager == nullptr) {
                    handleTutorFinished(cntRight, cntWrong);
                }

                if (tutorialManager != nullptr) {
                    abortTutorialFlow();
                    return;
                }

                showHomePage();
                if (slrTutorPage) {
                    stack->removeWidget(slrTutorPage);
                    slrTutorPage->deleteLater();
                    slrTutorPage = nullptr;
                }
            });

    return slrTutorPage;
}

void MainWindow::abortTutorialFlow() {
    cleanupTutorPages();
    showHomePage();
    setNavigationEnabled(true);

    if (tm) {
        tm->hideOverlay();
        tm->clearSteps();
        delete tm;
        tm = nullptr;
    }

    setupTutorial();
}

void MainWindow::handleTutorFinished(int cntRight, int cntWrong) {
    int delta = (cntRight - cntWrong);
    int raw   = static_cast<int>(userScore) + delta;
    userScore =
        static_cast<unsigned>(qBound(0, raw, static_cast<int>(MAX_SCORE)));

    while (userLevel() < MAX_LEVEL) {
        unsigned thr = thresholdFor(userLevel());
        if (userScore < thr)
            break;
        userScore -= thr;
        setUserLevel(userLevel() + 1);
        emit userLevelUp(userLevel());
    }

    ui->labelScore->setText(tr("Puntos: %1").arg(userScore));
    if (userLevel() >= MAX_LEVEL) {
        ui->progressBarNivel->setValue(100);
        ui->progressBarNivel->setEnabled(false);
    } else {
        unsigned thr     = thresholdFor(userLevel());
        int      percent = qMin(100, static_cast<int>((userScore * 100) / thr));
        ui->progressBarNivel->setEnabled(true);
        ui->progressBarNivel->setValue(percent);
    }

    saveSettings();
}

void MainWindow::on_pushButton_clicked() {
    Grammar grammar = factory.GenLL1Grammar(level);
    startLLTutor(grammar, nullptr);
}

void MainWindow::on_pushButton_2_clicked() {
    Grammar grammar = factory.GenSLR1Grammar(level);
    startSLRTutor(grammar, nullptr);
}

void MainWindow::on_tutorial_clicked() {
    cleanupTutorPages();
    showHomePage();

    if (tm) {
        delete tm;
        tm = nullptr;
    }

    setupTutorial();
    setNavigationEnabled(false);
    tm->start();
}

void MainWindow::setupTutorial() {
    tm = new TutorialManager(homePage);

    // Paso 1: explicación de botones LL(1) y SLR(1)
    tm->addStep(ui->pushButton, tr("<h3>LL(1)</h3><p>Con este botón puedes "
                                   "lanzar el tutor LL(1).</p>"));
    tm->addStep(ui->pushButton_2,
                tr("<h3>SLR(1)</h3><p>Con este, el SLR(1).</p>"));

    // Paso 2: explicación de niveles
    tm->addStep(ui->lv1Button,
                tr("<p>También puedes seleccionar el nivel de dificultad (1, 2 "
                   "o 3). La dificultad "
                   "repercute en la longitud de la gramática.</p>"));

    // Paso 3: LL(1)
    tm->addStep(ui->pushButton, tr("<p>Ahora se abrirá el tutor LL(1).</p>"));
    tm->addStep(nullptr, "");

    connect(tm, &TutorialManager::stepStarted, this, [this](int idx) {
        if (idx == 4) {
            Grammar grammarLL = factory.GenLL1Grammar(1);
            LLTutorWindow* llTutor = startLLTutor(grammarLL, tm);

            QTimer::singleShot(0, this, [this, llTutor]() {
                if (!tm || llTutor != llTutorPage) {
                    return;
                }
                tm->setRootWindow(llTutor);
                tm->nextStep();
            });
        }
    });

    connect(tm, &TutorialManager::ll1Finished, this, [this]() {
        tm->setRootWindow(homePage);

        if (llTutorPage) {
            stack->removeWidget(llTutorPage);
            llTutorPage->deleteLater();
            llTutorPage = nullptr;
        }

        showHomePage();
        disconnect(tm, &TutorialManager::stepStarted, this, nullptr);
        disconnect(tm, &TutorialManager::tutorialFinished, this, nullptr);

        tm->clearSteps();
        tm->addStep(ui->pushButton_2,
                    tr("<h3>SLR(1)</h3><p>Pasemos al tutor SLR(1).</p>"));
        tm->addStep(ui->lv3Button,
                    tr("<p>Esta vez se usará una gramática más compleja "
                       "(Nivel 3).</p>"));
        tm->addStep(ui->pushButton_2,
                    tr("<p>Ahora se abrirá el tutor SLR(1).</p>"));
        tm->addStep(nullptr, "");

        connect(tm, &TutorialManager::stepStarted, this, [this](int idx2) {
            if (idx2 == 3) {
                Grammar grammarSLR = factory.GenSLR1Grammar(3);
                SLRTutorWindow* slrTutor = startSLRTutor(grammarSLR, tm);

                QTimer::singleShot(0, this, [this, slrTutor]() {
                    if (!tm || slrTutor != slrTutorPage) {
                        return;
                    }
                    tm->setRootWindow(slrTutor);
                    tm->nextStep();
                });
            }
        });

        connect(tm, &TutorialManager::slr1Finished, this, [this]() {
            tm->setRootWindow(homePage);

            if (slrTutorPage) {
                stack->removeWidget(slrTutorPage);
                slrTutorPage->deleteLater();
                slrTutorPage = nullptr;
            }

            showHomePage();
            disconnect(tm, &TutorialManager::stepStarted, this, nullptr);
            disconnect(tm, &TutorialManager::tutorialFinished, this, nullptr);

            tm->clearSteps();
            tm->addStep(ui->badgeNivel,
                        tr("<h2>Nivel</h2>"
                           "<p>¡Practicar tiene recompensa! Cada vez que "
                           "resuelvas ejercicios o avances en el estudio, "
                           "ganarás puntos. Estos puntos te ayudarán a subir "
                           "de nivel: hay un total de 10. "
                           "¡Intenta llegar al máximo!</p>"));

            tm->addStep(homePage, tr("<h2>¡Tutorial completado!</h2><p>Ya "
                                     "puedes comenzar a practicar.</p>"));

            connect(tm, &TutorialManager::tutorialFinished, this, [this]() {
                if (tm) {
                    tm->clearSteps();
                    delete tm;
                    tm = nullptr;
                }
                setNavigationEnabled(true);
                setupTutorial();
            });

            tm->start();
        });

        tm->start();
    });
}

void MainWindow::on_actionSobre_la_aplicaci_n_triggered() {
    const auto versionLine =
        tr("<p><b>Versión:</b> %1</p>").arg(qApp->applicationVersion());
    showInfoDialog(
        this, tr("Sobre la aplicación"), tr("SyntaxTutor"),
        tr("Una herramienta de escritorio para practicar análisis sintáctico"),
        versionLine +
        tr("<p>Trabajo Fin de Grado – Tutorial Interactivo sobre Analizadores "
           "Sintácticos.</p>") +
        tr("<p><b>Autor:</b> José R.</p>") +
        tr("<p><b>Licencia:</b> GPLv3</p>") +
        tr("<p>Desarrollado con <a href='https://www.qt.io/'>Qt 6</a> y "
           "C++20.</p>") +
        tr("<p><a href='https://github.com/jose-rZM/SyntaxTutor'>GitHub - "
           "jose-rZM</a></p>") +
        tr("<p>2025 Universidad de Málaga</p>"));
}

void MainWindow::on_actionReferencia_LL_1_triggered() {
    showInfoDialog(
        this, tr("Referencia rápida LL(1)"), tr("LL(1)"),
        tr("Resumen de conjuntos y construcción de la tabla predictiva"),
        tr(R"(
            <h3>Conceptos clave</h3>
            <p><b>CAB(X):</b> conjunto de símbolos terminales que pueden comenzar cadenas derivables desde <code>X</code>.</p>
            <p><b>SIG(A):</b> conjunto de terminales que pueden aparecer justo después de <code>A</code> en alguna derivación.</p>
            <h3>Construcción de la tabla LL(1)</h3>
            <ul>
              <li>Para cada producción <code>A → α</code> y cada terminal <code>a ∈ CAB(α)</code>, asigna <code>Tabla[A][a] = α</code>.</li>
              <li>Si <code>ε ∈ CAB(α)</code>, para cada <code>b ∈ SIG(A)</code> asigna <code>Tabla[A][b] = α</code>.</li>
              <li>Si <code>ε ∈ CAB(α)</code> y <code>$ ∈ SIG(A)</code>, entonces <code>Tabla[A][$] = α</code>.</li>
            </ul>
            <h3>Conflictos</h3>
            <p>Aparecen cuando dos producciones compiten por la misma celda, por ejemplo si <code>CAB(α) ∩ CAB(β) ≠ ∅</code> o si una producción con <code>ε</code> invade símbolos de <code>SIG(A)</code>.</p>
        )"));
}

void MainWindow::on_actionReferencia_SLR_1_triggered() {
    showInfoDialog(
        this, tr("Referencia rápida SLR(1)"), tr("SLR(1)"),
        tr("Resumen de items LR(0), cierre, goto y tabla de análisis"),
        tr(R"(
            <h3>Conceptos clave</h3>
            <p><b>Ítems LR(0):</b> producciones con un punto que marca la posición actual del análisis.</p>
            <p><b>Cierre(I):</b> si un ítem contiene <code>∙ B</code>, añade los ítems <code>B → ∙ γ</code> correspondientes y repite hasta estabilizar.</p>
            <p><b>Goto(I, X):</b> desplaza el punto sobre <code>X</code> en todos los ítems válidos y calcula después su cierre.</p>
            <h3>Tabla SLR(1)</h3>
            <ul>
              <li><b>Action[I, a] = s<sub>j</sub></b> si existe un ítem <code>A → α ∙ a β</code> y <code>Goto(I, a) = j</code>.</li>
              <li><b>Action[I, a] = r<sub>k</sub></b> si existe un ítem completo <code>A → α ∙</code> y <code>a ∈ SIG(A)</code>.</li>
              <li><b>Action[I, $] = acc</b> cuando el estado contiene la situación de aceptación.</li>
              <li><b>Goto[I, A] = J</b> para transiciones con no terminales.</li>
            </ul>
            <h3>Conflictos</h3>
            <p>Un estado presenta conflicto cuando no puede elegirse una única acción válida, por ejemplo entre <i>shift</i> y <i>reduce</i> o entre dos reducciones distintas.</p>
        )"));
}

#include <QProcess>
void MainWindow::on_idiom_clicked() {
    QString selectedLang;

    QDialog dialog(this);
    dialog.setObjectName("infoDialog");
    dialog.setWindowTitle(tr("Idioma"));
    dialog.setModal(true);
    dialog.resize(420, 220);

    auto* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(14);
    layout->setContentsMargins(24, 22, 24, 18);

    auto* eyebrow = new QLabel(tr("Idioma"), &dialog);
    eyebrow->setObjectName("infoDialogEyebrow");

    auto* title = new QLabel(tr("Selecciona el idioma de la aplicación"), &dialog);
    title->setObjectName("infoDialogTitle");
    title->setWordWrap(true);

    auto* subtitle = new QLabel(
        tr("El cambio se aplicará al reiniciar la aplicación."), &dialog);
    subtitle->setObjectName("infoDialogSubtitle");
    subtitle->setWordWrap(true);

    auto* buttonsLayout = new QHBoxLayout;
    buttonsLayout->setSpacing(10);

    auto* btnEs = new QPushButton(tr("Español"), &dialog);
    btnEs->setObjectName("languageSpanishButton");
    btnEs->setCursor(Qt::PointingHandCursor);
    btnEs->setProperty("role", "primary");

    auto* btnEn = new QPushButton(tr("Inglés"), &dialog);
    btnEn->setObjectName("languageEnglishButton");
    btnEn->setCursor(Qt::PointingHandCursor);
    btnEn->setProperty("role", "primary");

    auto* btnCanc = new QPushButton(tr("Cancelar"), &dialog);
    btnCanc->setObjectName("languageCancelButton");
    btnCanc->setCursor(Qt::PointingHandCursor);
    btnCanc->setProperty("role", "danger");

    connect(btnEs, &QPushButton::clicked, &dialog, [&dialog, &selectedLang]() {
        selectedLang = "es";
        dialog.accept();
    });
    connect(btnEn, &QPushButton::clicked, &dialog, [&dialog, &selectedLang]() {
        selectedLang = "en";
        dialog.accept();
    });
    connect(btnCanc, &QPushButton::clicked, &dialog, &QDialog::reject);

    buttonsLayout->addWidget(btnEs);
    buttonsLayout->addWidget(btnEn);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(btnCanc);

    layout->addWidget(eyebrow);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(6);
    layout->addLayout(buttonsLayout);

    if (dialog.exec() != QDialog::Accepted || selectedLang.isEmpty()) {
        return;
    }

    QString currentLang = settings.value("lang/language", "es").toString();

    if (selectedLang != currentLang) {
        settings.setValue("lang/language", selectedLang);

        QMessageBox info(this);
        info.setWindowTitle(tr("Reiniciar requerido"));
        info.setText(tr("Para aplicar el cambio de idioma, es necesario "
                        "reiniciar la aplicación."));
        info.setStandardButtons(QMessageBox::Ok);
        info.exec();

#ifndef SYNTAXTUTOR_TESTING
        qApp->quit();
        QProcess::startDetached(qApp->applicationFilePath(), QStringList());
#endif
    }
}
