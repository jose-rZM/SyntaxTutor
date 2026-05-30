#include "tutor_window_test.h"

#include "qt_modal_test_utils.h"
#include "slr_tutor_test_utils.h"
#include "slrtutorwindow.h"
#include "tutor_grammar_fixtures.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QPointer>
#include <QPushButton>
#include <QSignalSpy>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTest>
#include <QWizard>

namespace {

QPushButton* waitForTutorButton(SLRTutorWindow& tutor, const QString& objectName) {
    const int intervalMs = 20;
    int       elapsed    = 0;

    while (elapsed <= 2000) {
        if (auto* button = tutor.findChild<QPushButton*>(objectName)) {
            return button;
        }
        QTest::qWait(intervalMs);
        elapsed += intervalMs;
    }

    return nullptr;
}

SLRTableDialog* waitForSlrTableDialog() {
    return QtModalTestUtils::waitForVisibleTopLevelWidget<SLRTableDialog>();
}

QWizard* waitForWizard() {
    return QtModalTestUtils::waitForVisibleTopLevelWidget<QWizard>();
}

void driveSlrTutorToState(SLRTutorWindow& tutor, const QString& state) {
    SlrTutorTestUtils::driveTutorToState(tutor, state);
}

void driveSlrTutorToH(SLRTutorWindow& tutor) {
    driveSlrTutorToState(tutor, "H");
    QVERIFY(waitForSlrTableDialog() != nullptr);
}

void driveSlrTutorUntilCbSymbol(SLRTutorWindow& tutor, const QString& symbol) {
    for (int guard = 0; guard < 200; ++guard) {
        const QString state = tutor.currentStateForTest();
        if (state == "CB" && tutor.currentCbSymbolForTest() == symbol) {
            return;
        }
        if (state == "D" || state == "H" || state == "fin") {
            break;
        }
        SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    }
    QCOMPARE(tutor.currentStateForTest(), QString("CB"));
    QCOMPARE(tutor.currentCbSymbolForTest(), symbol);
}

void driveSlrTutorToCbWithNonEpsilonSymbol(SLRTutorWindow& tutor) {
    for (int guard = 0; guard < 200; ++guard) {
        const QString state = tutor.currentStateForTest();
        if (state == "CB" && tutor.currentCbSymbolForTest() != "EPSILON") {
            return;
        }
        if (state == "D" || state == "H" || state == "fin") {
            break;
        }
        SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    }
    QCOMPARE(tutor.currentStateForTest(), QString("CB"));
    QVERIFY(tutor.currentCbSymbolForTest() != "EPSILON");
}

} // namespace

void TutorWindowTest::slrCreatesTutorWithNullTutorialManager() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);

    QCOMPARE(tutor.currentStateForTest(), QString("A"));
    QCOMPARE(tutor.rightCountForTest(), 0);
    QCOMPARE(tutor.wrongCountForTest(), 0);
}

void TutorWindowTest::slrStateAErrorPathAdvancesThroughAprime() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);

    tutor.setAnswerForTest("X -> .x");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("A1"));

    tutor.setAnswerForTest("X");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("A1"));

    tutor.setAnswerForTest(tutor.solutionForA1());
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("A2"));

    tutor.setAnswerForTest("X");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("A2"));

    tutor.setAnswerForTest(tutor.solutionForA2());
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("A3"));

    tutor.setAnswerForTest("X -> x");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("A3"));

    tutor.setAnswerForTest(SlrTutorTestUtils::rulesAnswer(tutor.solutionForA3()));
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("A4"));

    tutor.setAnswerForTest("X -> .x");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("A4"));

    tutor.setAnswerForTest(SlrTutorTestUtils::itemsAnswer(tutor.solutionForA4()));
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("A'"));

    tutor.setAnswerForTest("X -> .x");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("B"));
}

void TutorWindowTest::slrStateACorrectPathAdvancesToB() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    tutor.setAnswerForTest(SlrTutorTestUtils::correctAnswerForCurrentState(tutor));
    tutor.submitForTest();

    QCOMPARE(tutor.currentStateForTest(), QString("B"));
    QCOMPARE(tutor.rightCountForTest(), 1);
    QCOMPARE(tutor.wrongCountForTest(), 0);
}

void TutorWindowTest::slrStateBAndCIncorrectAnswersStillAdvance() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    QCOMPARE(tutor.currentStateForTest(), QString("B"));

    tutor.setAnswerForTest("999");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("C"));
    QCOMPARE(tutor.wrongCountForTest(), 1);

    tutor.setAnswerForTest("999");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("CA"));
    QCOMPARE(tutor.wrongCountForTest(), 2);
}

void TutorWindowTest::slrStateCAWrongThenCorrect() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    tutor.setAnswerForTest(SlrTutorTestUtils::correctAnswerForCurrentState(tutor));
    tutor.submitForTest();
    tutor.setAnswerForTest(SlrTutorTestUtils::correctAnswerForCurrentState(tutor));
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("CA"));

    tutor.setAnswerForTest("z");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("CA"));
    QCOMPARE(tutor.wrongCountForTest(), 1);

    const QString nextState =
        SlrTutorTestUtils::sortedSymbolsAnswer(tutor.solutionForCA()).isEmpty()
            ? QString("B")
            : QString("CB");
    tutor.setAnswerForTest(SlrTutorTestUtils::correctAnswerForCurrentState(tutor));
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), nextState);
}

void TutorWindowTest::slrStateCBEpsilonBranchAcceptsOnlyEmpty() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    driveSlrTutorUntilCbSymbol(tutor, "EPSILON");

    tutor.setAnswerForTest("x");
    tutor.submitForTest();
    QCOMPARE(tutor.wrongCountForTest(), 1);
    QVERIFY(tutor.currentStateForTest() == "CB" || tutor.currentStateForTest() == "B");

    SLRTutorWindow tutor2(grammar, nullptr);
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor2);
    driveSlrTutorUntilCbSymbol(tutor2, "EPSILON");
    const int rightBefore = tutor2.rightCountForTest();
    const int wrongBefore = tutor2.wrongCountForTest();
    tutor2.setAnswerForTest(QString());
    tutor2.submitForTest();
    QCOMPARE(tutor2.rightCountForTest(), rightBefore + 1);
    QCOMPARE(tutor2.wrongCountForTest(), wrongBefore);
}

void TutorWindowTest::slrStateCBNonEpsilonBranchAdvancesOnWrongAndCorrect() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    driveSlrTutorToCbWithNonEpsilonSymbol(tutor);

    tutor.setAnswerForTest("1");
    tutor.submitForTest();
    QCOMPARE(tutor.wrongCountForTest(), 1);
    QVERIFY(tutor.currentStateForTest() == "CB" || tutor.currentStateForTest() == "B");

    SLRTutorWindow tutor2(grammar, nullptr);
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor2);
    driveSlrTutorToCbWithNonEpsilonSymbol(tutor2);
    tutor2.setAnswerForTest(SlrTutorTestUtils::correctAnswerForCurrentState(tutor2));
    tutor2.submitForTest();
    QVERIFY(tutor2.currentStateForTest() == "CB" || tutor2.currentStateForTest() == "B");
}

void TutorWindowTest::slrDriveThroughCollectionUntilD() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    driveSlrTutorToState(tutor, "D");
    QCOMPARE(tutor.currentStateForTest(), QString("D"));
}

void TutorWindowTest::slrStateDErrorPathAdvancesToE() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    driveSlrTutorToState(tutor, "D");

    tutor.setAnswerForTest("1,1");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("D1"));

    tutor.setAnswerForTest("999");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("D1"));

    tutor.setAnswerForTest(tutor.solutionForD1());
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("D2"));

    tutor.setAnswerForTest("999");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("D2"));

    tutor.setAnswerForTest(tutor.solutionForD2());
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("D'"));

    tutor.setAnswerForTest("1,1");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("E"));
}

void TutorWindowTest::slrStateEErrorPathAdvancesToF() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    driveSlrTutorToState(tutor, "E");

    tutor.setAnswerForTest("999");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("E1"));

    tutor.setAnswerForTest("999");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("E1"));

    tutor.setAnswerForTest(SlrTutorTestUtils::idSetAnswer(tutor.solutionForE1()));
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("E2"));

    tutor.setAnswerForTest("999:1");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("F"));
}

void TutorWindowTest::slrStateFNoConflictAdvancesToG() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    driveSlrTutorToState(tutor, "F");
    QCOMPARE(tutor.solutionForF().isEmpty(), true);

    tutor.setAnswerForTest(QString());
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("G"));
}

void TutorWindowTest::slrStateFConflictBranchAdvancesToFAAndThenG() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrConflictGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    driveSlrTutorToState(tutor, "F");
    QVERIFY(!tutor.solutionForF().isEmpty());

    tutor.setAnswerForTest("999");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("F"));

    tutor.setAnswerForTest(SlrTutorTestUtils::idSetAnswer(tutor.solutionForF()));
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("FA"));

    tutor.setAnswerForTest("x");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("FA"));

    while (tutor.currentStateForTest() == "FA") {
        tutor.setAnswerForTest(SlrTutorTestUtils::stringSetAnswer(tutor.solutionForFA()));
        tutor.submitForTest();
    }
    QCOMPARE(tutor.currentStateForTest(), QString("G"));
}

void TutorWindowTest::slrStateGWrongThenCorrectReachesH() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    driveSlrTutorToState(tutor, "G");

    tutor.setAnswerForTest("x");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("G"));

    driveSlrTutorToH(tutor);
}

void TutorWindowTest::slrStateHIncorrectTableKeepsDialogOpen() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    driveSlrTutorToH(tutor);

    SLRTableDialog* dialog = waitForSlrTableDialog();
    auto*           table = dialog->findChild<QTableWidget*>("slrTableWidget");
    QVERIFY(table != nullptr);

    const QVector<QVector<QString>> wrongTable =
        SlrTutorTestUtils::buildWrongTable(grammar, table);
    const int wrongRow = QtModalTestUtils::firstNonEmptyCellRow(wrongTable);
    const int wrongCol = QtModalTestUtils::firstNonEmptyCellCol(wrongTable);

    QtModalTestUtils::scheduleMessageBoxResponse(QMessageBox::Ok);
    QtModalTestUtils::submitSlrTableDialog(dialog, wrongTable);

    QCOMPARE(tutor.currentStateForTest(), QString("H"));
    QCOMPARE(tutor.wrongCountForTest(), 0);
    QCOMPARE(QtModalTestUtils::cellBackground(table, wrongRow, wrongCol),
             QColor("#d9534f"));
    QVERIFY(dialog->isVisible());
}

void TutorWindowTest::slrGuidedModeWizardCompletesAndReturnsToTable() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    driveSlrTutorToH(tutor);

    SLRTableDialog* dialog = waitForSlrTableDialog();
    auto*           table = dialog->findChild<QTableWidget*>("slrTableWidget");
    QVERIFY(table != nullptr);

    QtModalTestUtils::requestSlrGuidedMode(
        dialog, QVector<QVector<QString>>(table->rowCount(),
                                          QVector<QString>(table->columnCount())));
    QWizard* wizard = waitForWizard();
    QVERIFY(wizard != nullptr);
    QPointer<QWizard> wizardGuard(wizard);

    SlrTutorTestUtils::finishWizard(wizard);
    QTRY_VERIFY(wizardGuard == nullptr || !wizardGuard->isVisible());
    QVERIFY(dialog->isVisible());
    QCOMPARE(tutor.currentStateForTest(), QString("H"));
}

void TutorWindowTest::slrTableDialogCancelNoReopensAndYesRequestsExit() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    QSignalSpy     exitSpy(&tutor, &SLRTutorWindow::exitRequested);

    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    driveSlrTutorToH(tutor);

    SLRTableDialog* dialog = waitForSlrTableDialog();
    QtModalTestUtils::scheduleMessageBoxResponse(QMessageBox::No);
    dialog->reject();

    SLRTableDialog* reopenedDialog = waitForSlrTableDialog();
    QVERIFY(reopenedDialog != nullptr);
    QVERIFY(reopenedDialog != dialog);
    QCOMPARE(exitSpy.count(), 0);

    QtModalTestUtils::scheduleMessageBoxResponse(QMessageBox::Yes);
    reopenedDialog->reject();

    QTRY_COMPARE(exitSpy.count(), 1);
    QCOMPARE(exitSpy.takeFirst().at(0).toBool(), false);
}

void TutorWindowTest::slrFinalTableCorrectPathExportsAndExits() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    QSignalSpy     exitSpy(&tutor, &SLRTutorWindow::exitRequested);

    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    driveSlrTutorToH(tutor);

    SLRTableDialog* dialog = waitForSlrTableDialog();
    auto*           table = dialog->findChild<QTableWidget*>("slrTableWidget");
    QVERIFY(table != nullptr);

    QtModalTestUtils::submitSlrTableDialog(
        dialog, SlrTutorTestUtils::buildExpectedTable(grammar, table));

    QTRY_COMPARE(tutor.currentStateForTest(), QString("fin"));

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString pdfPath = tempDir.filePath("slr_tutor_test.pdf");

    tutor.setNextExportFilePathForTest(pdfPath);
    auto* exportButton = waitForTutorButton(tutor, "slrTutorExportPdfButton");
    QVERIFY(exportButton != nullptr);
    QTest::mouseClick(exportButton, Qt::LeftButton);
    QTRY_VERIFY(QFileInfo::exists(pdfPath));
    QVERIFY(QFileInfo(pdfPath).size() > 0);

    auto* exitButton = waitForTutorButton(tutor, "slrTutorExitButton");
    QVERIFY(exitButton != nullptr);
    QTest::mouseClick(exitButton, Qt::LeftButton);
    QTRY_COMPARE(exitSpy.count(), 1);
    QCOMPARE(exitSpy.takeFirst().at(0).toBool(), true);
}

void TutorWindowTest::slrSmokeGuiFindsCoreWidgets() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    tutor.show();
    QCoreApplication::processEvents();

    QVERIFY(tutor.findChild<QWidget*>("listWidget") != nullptr);
    QVERIFY(tutor.findChild<QWidget*>("userResponse") != nullptr);
    QVERIFY(tutor.findChild<QWidget*>("confirmButton") != nullptr);
    QVERIFY(tutor.findChild<QWidget*>("backButton") != nullptr);

    tutor.setAnswerForTest(SlrTutorTestUtils::correctAnswerForCurrentState(tutor));
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("B"));
}
