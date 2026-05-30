#include "tutor_window_test.h"

#include "mainwindow.h"
#include "lltutorwindow.h"
#include "qt_modal_test_utils.h"
#include "slrtutorwindow.h"

#include <QCoreApplication>
#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QTest>
#include <QTextBrowser>
#include <QTranslator>

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
