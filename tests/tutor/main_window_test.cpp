#include "tutor_window_test.h"

#include "grammareditordialog.h"
#include "mainwindow.h"
#include "lltutorwindow.h"
#include "qt_modal_test_utils.h"
#include "slrtutorwindow.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QTest>
#include <QTextBrowser>
#include <QTranslator>
#include <algorithm>

namespace {

QSettings testAppSettings() {
    return QSettings("UMA-Test", "SyntaxTutor-Test");
}

void clearTestAppSettings() {
    QSettings settings = testAppSettings();
    settings.clear();
    settings.sync();
}

QDialog* findInfoDialog() {
    for (QWidget* widget : QApplication::topLevelWidgets()) {
        if (auto* dialog = qobject_cast<QDialog*>(widget);
            dialog != nullptr && dialog->objectName() == "infoDialog" &&
            dialog->isVisible()) {
            return dialog;
        }
    }
    return nullptr;
}

QPushButton* findVisibleTutorialNextButton() {
    for (QWidget* widget : QApplication::topLevelWidgets()) {
        for (QPushButton* button : widget->findChildren<QPushButton*>()) {
            if (button->objectName() == "tutorialNextButton" &&
                button->isVisible()) {
                return button;
            }
        }
    }
    return nullptr;
}

void clickInfoDialogClose(QDialog* dialog) {
    QVERIFY(dialog != nullptr);
    QPushButton* closeButton = QtModalTestUtils::findButtonByText(dialog, "Cerrar");
    if (closeButton == nullptr) {
        closeButton = QtModalTestUtils::findButtonByText(dialog, "Close");
    }
    QVERIFY(closeButton != nullptr);
    QTest::mouseClick(closeButton, Qt::LeftButton);
}

void installLanguageTranslator(QTranslator& translator, const QString& langCode) {
    QVERIFY(translator.load(langCode == "en" ? ":/translations/st_en.qm"
                                               : ":/translations/st_es.qm"));
    qApp->installTranslator(&translator);
}

} // namespace

// -----------------------------------------------------------------------------
// Case: MAIN-TC-01
// Summary:
//   Verifies that the main window appears correctly with its primary entry
//   controls and gamification indicators visible.
//
// Situation:
//   Application freshly opened on the home screen with empty test settings.
//
// Action:
//   The test shows `MainWindow` and locates the main buttons, progress bar,
//   level badge, and score label.
//
// Expected:
//   The window is visible without errors, the controls are enabled, and the
//   initial state is consistent.
// -----------------------------------------------------------------------------
void TutorWindowTest::mainInitialUiIsVisibleAndEnabled() {
    clearTestAppSettings();

    MainWindow window;
    window.show();
    QCoreApplication::processEvents();

    auto* llButton = window.findChild<QPushButton*>("pushButton");
    auto* slrButton = window.findChild<QPushButton*>("pushButton_2");
    auto* tutorialButton = window.findChild<QPushButton*>("tutorial");
    auto* languageButton = window.findChild<QPushButton*>("idiom");
    auto* scoreLabel = window.findChild<QLabel*>("labelScore");
    auto* levelBadge = window.findChild<QLabel*>("badgeNivel");
    auto* progressBar = window.findChild<QProgressBar*>("progressBarNivel");

    QVERIFY(llButton != nullptr && llButton->isEnabled());
    QVERIFY(slrButton != nullptr && slrButton->isEnabled());
    QVERIFY(tutorialButton != nullptr && tutorialButton->isEnabled());
    QVERIFY(languageButton != nullptr && languageButton->isEnabled());
    QVERIFY(scoreLabel != nullptr && scoreLabel->isVisible());
    QVERIFY(levelBadge != nullptr && levelBadge->isVisible());
    QVERIFY(progressBar != nullptr && progressBar->isVisible());
    QCOMPARE(levelBadge->text(), QString("1"));
    QCOMPARE(scoreLabel->text(), QString("Puntos: 0"));
    QCOMPARE(progressBar->value(), 0);
}

// -----------------------------------------------------------------------------
// Case: MAIN-TC-02
// Summary:
//   Checks the language switch to English and persistence of the selected
//   preference.
//
// Situation:
//   Main window open in the default language and using isolated test settings.
//
// Action:
//   The user opens the language selector, chooses English, and confirms the
//   informational message.
//
// Expected:
//   The `en` selection is stored and a newly opened window can use that saved
//   preference.
// -----------------------------------------------------------------------------
void TutorWindowTest::mainSwitchLanguageToEnglishPersistsSelection() {
    clearTestAppSettings();

    MainWindow window;
    window.show();

    QtModalTestUtils::scheduleUntilHandled([]() {
        QDialog* dialog = findInfoDialog();
        if (dialog == nullptr) {
            return false;
        }
        auto* englishButton = dialog->findChild<QPushButton*>("languageEnglishButton");
        if (englishButton == nullptr) {
            return false;
        }
        QTest::mouseClick(englishButton, Qt::LeftButton);
        return true;
    });
    QtModalTestUtils::scheduleMessageBoxResponse(QMessageBox::Ok);
    QTest::mouseClick(window.findChild<QPushButton*>("idiom"), Qt::LeftButton);

    QCOMPARE(testAppSettings().value("lang/language").toString(), QString("en"));

    QTranslator translator;
    installLanguageTranslator(translator, "en");
    MainWindow reopened;
    reopened.show();
    auto* difficultyTitle = reopened.findChild<QLabel*>("difficultyTitle");
    QVERIFY(difficultyTitle != nullptr);
    QVERIFY(!difficultyTitle->text().isEmpty());
    qApp->removeTranslator(&translator);
}

// -----------------------------------------------------------------------------
// Case: MAIN-TC-03
// Summary:
//   Checks the language switch back to Spanish from a state where English was
//   previously selected.
//
// Situation:
//   Main window open with the `en` preference already stored in test settings.
//
// Action:
//   The user opens the language selector, chooses Spanish, and confirms the
//   informational message.
//
// Expected:
//   The `es` selection is stored and a newly opened window shows the main text
//   in Spanish again.
// -----------------------------------------------------------------------------
void TutorWindowTest::mainSwitchLanguageToSpanishPersistsSelection() {
    clearTestAppSettings();
    testAppSettings().setValue("lang/language", "en");

    QTranslator translator;
    installLanguageTranslator(translator, "en");
    MainWindow window;
    window.show();

    QtModalTestUtils::scheduleUntilHandled([]() {
        QDialog* dialog = findInfoDialog();
        if (dialog == nullptr) {
            return false;
        }
        auto* spanishButton = dialog->findChild<QPushButton*>("languageSpanishButton");
        if (spanishButton == nullptr) {
            return false;
        }
        QTest::mouseClick(spanishButton, Qt::LeftButton);
        return true;
    });
    QtModalTestUtils::scheduleMessageBoxResponse(QMessageBox::Ok);
    QTest::mouseClick(window.findChild<QPushButton*>("idiom"), Qt::LeftButton);

    QCOMPARE(testAppSettings().value("lang/language").toString(), QString("es"));
    qApp->removeTranslator(&translator);

    MainWindow reopened;
    reopened.show();
    auto* difficultyTitle = reopened.findChild<QLabel*>("difficultyTitle");
    QVERIFY(difficultyTitle != nullptr);
    QCOMPARE(difficultyTitle->text(), QString("Dificultad"));
}

// -----------------------------------------------------------------------------
// Case: MAIN-TC-04
// Summary:
//   Verifies that the About dialog presents the application's main metadata.
//
// Situation:
//   Main window open with the menu actions available.
//
// Action:
//   The user opens the `About the app` action.
//
// Expected:
//   The dialog shows author, license, and repository information.
// -----------------------------------------------------------------------------
void TutorWindowTest::mainAboutDialogShowsMetadata() {
    clearTestAppSettings();

    MainWindow window;
    window.show();

    auto* action = window.findChild<QAction*>("actionSobre_la_aplicaci_n");
    QVERIFY(action != nullptr);
    bool contentVerified = false;
    QtModalTestUtils::scheduleUntilHandled([&contentVerified]() {
        QDialog* dialog = findInfoDialog();
        if (dialog == nullptr) {
            return false;
        }
        auto* content = dialog->findChild<QTextBrowser*>("infoDialogContent");
        if (content == nullptr) {
            return false;
        }
        const QString text = content->toPlainText();
        contentVerified = text.contains("José R.") && text.contains("GPLv3") &&
                          text.contains("GitHub");
        clickInfoDialogClose(dialog);
        return true;
    });
    action->trigger();
    QVERIFY(contentVerified);
}

// -----------------------------------------------------------------------------
// Case: MAIN-TC-05
// Summary:
//   Checks that the LL(1) and SLR(1) quick references open and return to the
//   main flow without errors.
//
// Situation:
//   Main window open with the reference menu actions available.
//
// Action:
//   The user opens the LL(1) reference, closes it, and repeats the process for
//   the SLR(1) reference.
//
// Expected:
//   Both dialogs appear with the correct title and can be closed normally.
// -----------------------------------------------------------------------------
void TutorWindowTest::mainQuickReferencesOpen() {
    clearTestAppSettings();

    MainWindow window;
    window.show();

    auto* llRef  = window.findChild<QAction*>("actionReferencia_LL_1");
    auto* slrRef = window.findChild<QAction*>("actionReferencia_SLR_1");
    QVERIFY(llRef != nullptr);
    QVERIFY(slrRef != nullptr);

    bool llSeen = false;
    QtModalTestUtils::scheduleUntilHandled([&llSeen]() {
        QDialog* dialog = findInfoDialog();
        if (dialog == nullptr) {
            return false;
        }
        llSeen = dialog->windowTitle().contains("LL(1)");
        clickInfoDialogClose(dialog);
        return true;
    });
    llRef->trigger();
    QVERIFY(llSeen);

    bool slrSeen = false;
    QtModalTestUtils::scheduleUntilHandled([&slrSeen]() {
        QDialog* dialog = findInfoDialog();
        if (dialog == nullptr) {
            return false;
        }
        slrSeen = dialog->windowTitle().contains("SLR(1)");
        clickInfoDialogClose(dialog);
        return true;
    });
    slrRef->trigger();
    QVERIFY(slrSeen);
}

// -----------------------------------------------------------------------------
// Case: MAIN-TC-01, MAIN-TC-09
// Summary:
//   Verifies that the LL(1) and SLR(1) entry points on the home screen open the
//   corresponding tutor windows.
//
// Situation:
//   Main window in its initial state with navigation enabled.
//
// Action:
//   The user presses `LL(1)` in one window and `SLR(1)` in another.
//
// Expected:
//   The matching tutor window is created in each case.
// -----------------------------------------------------------------------------
void TutorWindowTest::mainLlAndSlrEntryPointsOpenTutors() {
    clearTestAppSettings();

    MainWindow llWindow;
    llWindow.show();
    QTest::mouseClick(llWindow.findChild<QPushButton*>("pushButton"), Qt::LeftButton);
    QTRY_VERIFY(llWindow.findChild<LLTutorWindow*>() != nullptr);

    MainWindow slrWindow;
    slrWindow.show();
    QTest::mouseClick(slrWindow.findChild<QPushButton*>("pushButton_2"), Qt::LeftButton);
    QTRY_VERIFY(slrWindow.findChild<SLRTutorWindow*>() != nullptr);
}

// -----------------------------------------------------------------------------
// Case: MAIN-TC-06
// Summary:
//   Runs a smoke test of the full tutorial, verifying that it opens both tutors
//   and restores main navigation at the end.
//
// Situation:
//   Main window open with the tutorial available.
//
// Action:
//   The user presses `Tutorial` and advances through all steps with `Next`.
//
// Expected:
//   The tutorial visits LL(1) and SLR(1), finishes without errors, and leaves
//   the main controls enabled again.
// -----------------------------------------------------------------------------
void TutorWindowTest::mainTutorialFlowCompletesAndReenablesControls() {
    clearTestAppSettings();

    MainWindow window;
    window.show();

    auto* llButton       = window.findChild<QPushButton*>("pushButton");
    auto* slrButton      = window.findChild<QPushButton*>("pushButton_2");
    auto* tutorialButton = window.findChild<QPushButton*>("tutorial");
    QVERIFY(llButton != nullptr);
    QVERIFY(slrButton != nullptr);
    QVERIFY(tutorialButton != nullptr);

    QTest::mouseClick(tutorialButton, Qt::LeftButton);
    QVERIFY(!llButton->isEnabled());
    QVERIFY(!slrButton->isEnabled());

    bool sawLlTutor  = false;
    bool sawSlrTutor = false;
    for (int guard = 0; guard < 40 && !llButton->isEnabled(); ++guard) {
        if (window.findChild<LLTutorWindow*>() != nullptr) {
            sawLlTutor = true;
        }
        if (window.findChild<SLRTutorWindow*>() != nullptr) {
            sawSlrTutor = true;
        }
        QPushButton* nextButton = findVisibleTutorialNextButton();
        QVERIFY(nextButton != nullptr);
        QTest::mouseClick(nextButton, Qt::LeftButton);
        QTest::qWait(20);
    }

    QVERIFY(sawLlTutor);
    QVERIFY(sawSlrTutor);
    QVERIFY(llButton->isEnabled());
    QVERIFY(slrButton->isEnabled());
    QVERIFY(tutorialButton->isEnabled());
}

// -----------------------------------------------------------------------------
// Case: MAIN-TC-11
// Summary:
//   Checks that a completed session updates score, progress, and level, and
//   that those values persist when the main window is recreated.
//
// Situation:
//   Main window open with clean test settings and an LL(1) session launched from
//   the UI.
//
// Action:
//   The test simulates the tutor finishing with more correct answers than wrong
//   ones.
//
// Expected:
//   Score, progress bar, and level are updated, and the same values reappear
//   when the main window is opened again.
// -----------------------------------------------------------------------------
void TutorWindowTest::mainGamificationPersistsAcrossRestart() {
    clearTestAppSettings();

    MainWindow window;
    window.show();
    QTest::mouseClick(window.findChild<QPushButton*>("pushButton"), Qt::LeftButton);
    auto* tutor = window.findChild<LLTutorWindow*>();
    QVERIFY(tutor != nullptr);

    QVERIFY(QMetaObject::invokeMethod(tutor, "exitRequested", Qt::DirectConnection,
                                      Q_ARG(bool, true), Q_ARG(int, 12), Q_ARG(int, 0)));

    auto* levelBadge  = window.findChild<QLabel*>("badgeNivel");
    auto* scoreLabel  = window.findChild<QLabel*>("labelScore");
    auto* progressBar = window.findChild<QProgressBar*>("progressBarNivel");
    QVERIFY(levelBadge != nullptr);
    QVERIFY(scoreLabel != nullptr);
    QVERIFY(progressBar != nullptr);
    QCOMPARE(levelBadge->text(), QString("2"));
    QCOMPARE(scoreLabel->text(), QString("Puntos: 2"));
    QCOMPARE(progressBar->value(), 10);

    MainWindow reopened;
    reopened.show();
    QCOMPARE(reopened.findChild<QLabel*>("badgeNivel")->text(), QString("2"));
    QCOMPARE(reopened.findChild<QLabel*>("labelScore")->text(), QString("Puntos: 2"));
}

// -----------------------------------------------------------------------------
// Case: MAIN-TC-12
// Summary:
//   Verifies that the main window loads persisted level, score, and language
//   state correctly.
//
// Situation:
//   Test settings preloaded manually before creating `MainWindow`.
//
// Action:
//   The test opens the main window while reading the previously saved state.
//
// Expected:
//   The visible indicators match the persisted values exactly.
// -----------------------------------------------------------------------------
void TutorWindowTest::mainStatePersistenceAcrossRestart() {
    clearTestAppSettings();
    QSettings settings = testAppSettings();
    settings.setValue("gamification/level", 3);
    settings.setValue("gamification/score", 7);
    settings.setValue("lang/language", "es");
    settings.sync();

    MainWindow window;
    window.show();

    QCOMPARE(window.findChild<QLabel*>("badgeNivel")->text(), QString("3"));
    QCOMPARE(window.findChild<QLabel*>("labelScore")->text(), QString("Puntos: 7"));
    QCOMPARE(window.findChild<QProgressBar*>("progressBarNivel")->value(), 23);
}

namespace {

void scheduleGrammarEditorSubmission(const QString& grammarText) {
    QtModalTestUtils::scheduleUntilHandled([grammarText]() {
        auto* dialog =
            QtModalTestUtils::findVisibleTopLevelWidget<GrammarEditorDialog>();
        if (dialog == nullptr) {
            return false;
        }

        dialog->setGrammarTextForTest(grammarText);
        if (!dialog->isGrammarValidForTest()) {
            return false;
        }

        auto* start =
            dialog->findChild<QPushButton*>("grammarEditorStartButton");
        if (start == nullptr || !start->isEnabled()) {
            return false;
        }
        QTest::mouseClick(start, Qt::LeftButton);
        return true;
    });
}

void scheduleGrammarEditorCancel() {
    QtModalTestUtils::scheduleUntilHandled([]() {
        auto* dialog =
            QtModalTestUtils::findVisibleTopLevelWidget<GrammarEditorDialog>();
        if (dialog == nullptr) {
            return false;
        }

        auto* cancel =
            dialog->findChild<QPushButton*>("grammarEditorCancelButton");
        if (cancel == nullptr) {
            return false;
        }
        QTest::mouseClick(cancel, Qt::LeftButton);
        return true;
    });
}

bool anyLabelContains(QWidget* root, const QString& needle) {
    const QList<QLabel*> labels = root->findChildren<QLabel*>();
    return std::any_of(labels.cbegin(), labels.cend(),
                       [&needle](const QLabel* label) {
                           return label->text().contains(needle);
                       });
}

} // namespace

// -----------------------------------------------------------------------------
// Case: MAIN-TC-13
// Summary:
//   Verifies that enabling "use my own grammar" disables difficulty levels and
//   that disabling it restores them.
//
// Situation:
//   Main window freshly opened on the home screen.
//
// Action:
//   The test toggles the custom grammar checkbox on and off.
//
// Expected:
//   Level radio buttons are disabled while the checkbox is checked and enabled
//   again when unchecked.
// -----------------------------------------------------------------------------
void TutorWindowTest::mainCustomGrammarToggleDisablesLevels() {
    clearTestAppSettings();

    MainWindow window;
    window.show();

    auto* check = window.findChild<QCheckBox*>("customGrammarCheck");
    QVERIFY(check != nullptr);
    QVERIFY(check->isEnabled());
    QVERIFY(!check->isChecked());

    check->setChecked(true);
    QVERIFY(!window.findChild<QRadioButton*>("lv1Button")->isEnabled());
    QVERIFY(!window.findChild<QRadioButton*>("lv2Button")->isEnabled());
    QVERIFY(!window.findChild<QRadioButton*>("lv3Button")->isEnabled());

    check->setChecked(false);
    QVERIFY(window.findChild<QRadioButton*>("lv1Button")->isEnabled());
    QVERIFY(window.findChild<QRadioButton*>("lv2Button")->isEnabled());
    QVERIFY(window.findChild<QRadioButton*>("lv3Button")->isEnabled());
}

// -----------------------------------------------------------------------------
// Case: MAIN-TC-14
// Summary:
//   Starts an LL(1) exercise with a user-written grammar that uses
//   multi-character tokens.
//
// Situation:
//   Custom grammar mode enabled on the home screen.
//
// Action:
//   The test clicks the LL(1) button, writes an LL(1) grammar in the editor
//   dialog and presses the start button.
//
// Expected:
//   The LL(1) tutor opens showing the user's symbols and the grammar text is
//   persisted for the next session.
// -----------------------------------------------------------------------------
void TutorWindowTest::mainCustomGrammarLlFlowStartsTutorWithUserGrammar() {
    clearTestAppSettings();

    MainWindow window;
    window.show();
    window.findChild<QCheckBox*>("customGrammarCheck")->setChecked(true);

    const QString grammarText =
        QStringLiteral("Expr -> id Resto .\nResto -> + id Resto | .");
    scheduleGrammarEditorSubmission(grammarText);
    QTest::mouseClick(window.findChild<QPushButton*>("pushButton"),
                      Qt::LeftButton);

    LLTutorWindow* tutor = nullptr;
    QTRY_VERIFY((tutor = window.findChild<LLTutorWindow*>()) != nullptr);
    QVERIFY(anyLabelContains(tutor, QStringLiteral("Resto")));

    QSettings settings = testAppSettings();
    QCOMPARE(settings.value("userGrammar/lastText").toString(), grammarText);
}

// -----------------------------------------------------------------------------
// Case: MAIN-TC-15
// Summary:
//   Starts an SLR(1) exercise with a user-written left-recursive grammar.
//
// Situation:
//   Custom grammar mode enabled on the home screen.
//
// Action:
//   The test clicks the SLR(1) button and submits a left-recursive grammar
//   (valid SLR(1), invalid LL(1)) through the editor dialog.
//
// Expected:
//   The SLR(1) tutor opens showing the user's symbols.
// -----------------------------------------------------------------------------
void TutorWindowTest::mainCustomGrammarSlrFlowStartsTutorWithUserGrammar() {
    clearTestAppSettings();

    MainWindow window;
    window.show();
    window.findChild<QCheckBox*>("customGrammarCheck")->setChecked(true);

    scheduleGrammarEditorSubmission(QStringLiteral("Expr -> Expr + id | id ."));
    QTest::mouseClick(window.findChild<QPushButton*>("pushButton_2"),
                      Qt::LeftButton);

    SLRTutorWindow* tutor = nullptr;
    QTRY_VERIFY((tutor = window.findChild<SLRTutorWindow*>()) != nullptr);
    QVERIFY(anyLabelContains(tutor, QStringLiteral("Expr")));
}

// -----------------------------------------------------------------------------
// Case: MAIN-TC-16
// Summary:
//   Exercises the grammar editor validation: format errors, reserved symbols,
//   the LL(1) check, and a final valid grammar.
//
// Situation:
//   Grammar editor dialog opened directly in LL(1) mode.
//
// Action:
//   The test writes several invalid grammars followed by a valid one.
//
// Expected:
//   The start button stays disabled and an error message is shown until the
//   grammar becomes valid.
// -----------------------------------------------------------------------------
void TutorWindowTest::mainCustomGrammarEditorRejectsInvalidAndNonLl1Grammars() {
    clearTestAppSettings();

    GrammarEditorDialog dialog(GrammarEditorDialog::Mode::LL1);
    dialog.show();

    auto* start  = dialog.findChild<QPushButton*>("grammarEditorStartButton");
    auto* status = dialog.findChild<QLabel*>("grammarEditorStatus");
    QVERIFY(start != nullptr);
    QVERIFY(status != nullptr);

    dialog.setGrammarTextForTest(QStringLiteral("A -> a"));
    QVERIFY(!dialog.isGrammarValidForTest());
    QVERIFY(!start->isEnabled());
    QVERIFY(!status->text().isEmpty());

    dialog.setGrammarTextForTest(QStringLiteral("A B -> a ."));
    QVERIFY(!dialog.isGrammarValidForTest());

    dialog.setGrammarTextForTest(QStringLiteral("A -> a $ ."));
    QVERIFY(!dialog.isGrammarValidForTest());

    dialog.setGrammarTextForTest(QStringLiteral("A -> a\nB -> b ."));
    QVERIFY(!dialog.isGrammarValidForTest());

    // Left recursion is fine for SLR(1) but must be rejected in LL(1) mode.
    dialog.setGrammarTextForTest(QStringLiteral("Expr -> Expr + id | id ."));
    QVERIFY(!dialog.isGrammarValidForTest());

    // Non-productive grammars never derive a terminal string.
    dialog.setGrammarTextForTest(QStringLiteral("A -> a B .\nB -> B b ."));
    QVERIFY(!dialog.isGrammarValidForTest());

    dialog.setGrammarTextForTest(QStringLiteral("A -> a A | b ."));
    QVERIFY(dialog.isGrammarValidForTest());
    QVERIFY(start->isEnabled());
}

// -----------------------------------------------------------------------------
// Case: MAIN-TC-17
// Summary:
//   Cancels the grammar editor and verifies no tutor is started.
//
// Situation:
//   Custom grammar mode enabled on the home screen.
//
// Action:
//   The test clicks the LL(1) button and dismisses the editor dialog.
//
// Expected:
//   The application stays on the home page with no tutor created.
// -----------------------------------------------------------------------------
void TutorWindowTest::mainCustomGrammarEditorCancelKeepsHomePage() {
    clearTestAppSettings();

    MainWindow window;
    window.show();
    window.findChild<QCheckBox*>("customGrammarCheck")->setChecked(true);

    scheduleGrammarEditorCancel();
    QTest::mouseClick(window.findChild<QPushButton*>("pushButton"),
                      Qt::LeftButton);

    QTest::qWait(50);
    QVERIFY(window.findChild<LLTutorWindow*>() == nullptr);
    QVERIFY(window.findChild<QPushButton*>("pushButton")->isEnabled());
}

// -----------------------------------------------------------------------------
// Case: MAIN-TC-18
// Summary:
//   Verifies that the last accepted grammar is restored when the editor is
//   reopened.
//
// Situation:
//   A grammar was accepted in a previous editor session.
//
// Action:
//   The test accepts a grammar in one dialog instance and opens a new one.
//
// Expected:
//   The new editor starts pre-filled with the persisted grammar and is
//   immediately valid.
// -----------------------------------------------------------------------------
void TutorWindowTest::mainCustomGrammarEditorRestoresLastGrammar() {
    clearTestAppSettings();

    const QString grammarText = QStringLiteral("A -> a A | b .");
    {
        GrammarEditorDialog dialog(GrammarEditorDialog::Mode::LL1);
        dialog.show();
        dialog.setGrammarTextForTest(grammarText);
        QVERIFY(dialog.isGrammarValidForTest());
        QTest::mouseClick(
            dialog.findChild<QPushButton*>("grammarEditorStartButton"),
            Qt::LeftButton);
    }

    GrammarEditorDialog reopened(GrammarEditorDialog::Mode::LL1);
    QCOMPARE(reopened.findChild<QPlainTextEdit*>("grammarEditorInput")
                 ->toPlainText(),
             grammarText);
    QVERIFY(reopened.isGrammarValidForTest());
}

// -----------------------------------------------------------------------------
// Case: MAIN-TC-19
// Summary:
//   Verifies that the "Modo examen" checkbox launches tutors in exam mode.
//
// Situation:
//   Main window freshly opened on the home screen.
//
// Action:
//   The test checks the exam mode checkbox and opens the LL(1) tutor.
//
// Expected:
//   The tutor starts with the feedback counters hidden (exam mode active);
//   without the checkbox the counters are visible.
// -----------------------------------------------------------------------------
void TutorWindowTest::mainExamModeCheckboxLaunchesExamTutor() {
    clearTestAppSettings();

    MainWindow window;
    window.show();

    auto* examCheck = window.findChild<QCheckBox*>("examModeCheck");
    QVERIFY(examCheck != nullptr);
    QVERIFY(examCheck->isEnabled());
    examCheck->setChecked(true);

    QTest::mouseClick(window.findChild<QPushButton*>("pushButton"),
                      Qt::LeftButton);
    LLTutorWindow* tutor = nullptr;
    QTRY_VERIFY((tutor = window.findChild<LLTutorWindow*>()) != nullptr);
    QVERIFY(tutor->findChild<QLabel*>("cntRight")->isHidden());
    QVERIFY(tutor->findChild<QLabel*>("cntWrong")->isHidden());

    MainWindow normalWindow;
    normalWindow.show();
    QTest::mouseClick(normalWindow.findChild<QPushButton*>("pushButton"),
                      Qt::LeftButton);
    LLTutorWindow* normalTutor = nullptr;
    QTRY_VERIFY((normalTutor = normalWindow.findChild<LLTutorWindow*>()) !=
                nullptr);
    QVERIFY(!normalTutor->findChild<QLabel*>("cntRight")->isHidden());
}
