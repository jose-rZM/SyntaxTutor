#include "tutor_window_test.h"

#include "ll1_tutor_test_utils.h"
#include "lltutorwindow.h"
#include "qt_modal_test_utils.h"
#include "tutor_grammar_fixtures.h"
#include "tutor_scenario.h"

#include <QColor>
#include <QCoreApplication>
#include <QFileInfo>
#include <QPushButton>
#include <QSignalSpy>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTest>

namespace {

void answerRemainingRulesUntilStateC(LLTutorWindow& tutor, const Grammar& grammar) {
    while (tutor.currentStateForTest() == "B") {
        tutor.setAnswerForTest(Ll1TutorTestUtils::predictionSymbolsAnswer(
            grammar, tutor.currentRuleAntecedentForTest(),
            tutor.currentRuleConsequentForTest()));
        tutor.submitForTest();
    }
}

LLTableDialog* waitForTableDialog() {
    return QtModalTestUtils::waitForVisibleTopLevelWidget<LLTableDialog>();
}

void driveTutorToCPrime(LLTutorWindow& tutor, const Grammar& grammar) {
    LLTableDialog* dialog = waitForTableDialog();
    auto*          table = dialog->findChild<QTableWidget*>("llTableWidget");
    QVERIFY(table != nullptr);

    const QVector<QVector<QString>> wrongTable =
        QtModalTestUtils::buildWrongTable(grammar, table);

    for (int attempt = 0; attempt < 3; ++attempt) {
        QtModalTestUtils::scheduleMessageBoxResponse(QMessageBox::Ok);
        QtModalTestUtils::submitLlTableDialog(dialog, wrongTable);
        QTRY_COMPARE(tutor.currentStateForTest(), QString("C"));
        QVERIFY(dialog->isVisible());
    }

    QtModalTestUtils::scheduleMessageBoxResponse(QMessageBox::Ok);
    QtModalTestUtils::submitLlTableDialog(dialog, wrongTable);
    QTRY_COMPARE(tutor.currentStateForTest(), QString("C"));
    QVERIFY(dialog->isVisible());

    QtModalTestUtils::submitLlTableDialog(dialog, wrongTable);
    QTRY_COMPARE(tutor.currentStateForTest(), QString("C'"));
}

QPushButton* waitForTutorButton(LLTutorWindow& tutor, const QString& objectName) {
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

} // namespace

void TutorWindowTest::createsTutorWithNullTutorialManager() {
    const Grammar grammar = TutorGrammarFixtures::makeLl1SimpleGrammar();

    LLTutorWindow tutor(grammar, nullptr);

    QCOMPARE(tutor.currentStateForTest(), QString("A"));
    QCOMPARE(tutor.rightCountForTest(), 0);
    QCOMPARE(tutor.wrongCountForTest(), 0);
}

void TutorWindowTest::stateAErrorPathAdvancesThroughAStates() {
    const Grammar grammar = TutorGrammarFixtures::makeLl1SimpleGrammar();

    LLTutorWindow tutor(grammar, nullptr);

    runScenario(tutor,
                {{"1,1", "A1", 0, 1},
                 {"99", "A1", 0, 2},
                 {Ll1TutorTestUtils::nonTerminalCountAnswer(grammar), "A2", 1,
                  2},
                 {"99", "A2", 1, 3},
                 {Ll1TutorTestUtils::terminalCountAnswer(grammar), "A'", 2, 3},
                 {"1,1", "B", 2, 4}});
}

void TutorWindowTest::stateACorrectPathAdvancesToB() {
    const Grammar grammar = TutorGrammarFixtures::makeLl1SimpleGrammar();

    LLTutorWindow tutor(grammar, nullptr);

    runScenario(tutor, {{Ll1TutorTestUtils::tableSizeAnswer(grammar), "B", 1,
                         0}});
}

void TutorWindowTest::stateBAxiomBranchSkipsB2() {
    const Grammar grammar = TutorGrammarFixtures::makeLl1SimpleGrammar();

    LLTutorWindow tutor(grammar, nullptr);

    runScenario(tutor, {{Ll1TutorTestUtils::tableSizeAnswer(grammar), "B", 1,
                         0}});

    const QString firstAntecedent = tutor.currentRuleAntecedentForTest();
    QCOMPARE(firstAntecedent, QString("S"));

    runScenario(tutor,
                {{"x", "B1", 1, 1},
                 {Ll1TutorTestUtils::cabAnswer(grammar, firstAntecedent,
                                               tutor.currentRuleConsequentForTest()),
                  "B'", 2, 1}});
}

void TutorWindowTest::stateBDirectAndFallbackPathsUpdateCounters() {
    const Grammar grammar = TutorGrammarFixtures::makeLl1SimpleGrammar();

    LLTutorWindow tutor(grammar, nullptr);

    runScenario(tutor, {{Ll1TutorTestUtils::tableSizeAnswer(grammar), "B", 1,
                         0}});

    const QString firstRuleAntecedent = tutor.currentRuleAntecedentForTest();
    const QString firstRulePrediction = Ll1TutorTestUtils::predictionSymbolsAnswer(
        grammar, firstRuleAntecedent, tutor.currentRuleConsequentForTest());

    runScenario(tutor, {{firstRulePrediction, "B", 2, 0}});

    const QString currentAntecedent = tutor.currentRuleAntecedentForTest();
    const QStringList currentConsequent = tutor.currentRuleConsequentForTest();

    runScenario(tutor,
                {{"x", "B1", 2, 1},
                 {Ll1TutorTestUtils::cabAnswer(grammar, currentAntecedent,
                                               currentConsequent),
                  "B2", 3, 1},
                 {Ll1TutorTestUtils::followAnswer(grammar, currentAntecedent),
                  "B'", 4, 1},
                 {"x", "B", 4, 2}});
}

void TutorWindowTest::stateBWrongAnswersStayInB1AndB2() {
    const Grammar grammar = TutorGrammarFixtures::makeLl1SimpleGrammar();

    LLTutorWindow tutor(grammar, nullptr);

    runScenario(tutor, {{Ll1TutorTestUtils::tableSizeAnswer(grammar), "B", 1,
                         0}});

    const QString firstAntecedent = tutor.currentRuleAntecedentForTest();
    const QString firstPrediction = Ll1TutorTestUtils::predictionSymbolsAnswer(
        grammar, firstAntecedent, tutor.currentRuleConsequentForTest());
    runScenario(tutor, {{firstPrediction, "B", 2, 0}});

    const QString nonAxiomAntecedent = tutor.currentRuleAntecedentForTest();
    const QStringList nonAxiomConsequent = tutor.currentRuleConsequentForTest();

    runScenario(tutor,
                {{"x", "B1", 2, 1},
                 {"x", "B1", 2, 2},
                 {Ll1TutorTestUtils::cabAnswer(grammar, nonAxiomAntecedent,
                                               nonAxiomConsequent),
                  "B2", 3, 2},
                 {"x", "B2", 3, 3},
                 {Ll1TutorTestUtils::followAnswer(grammar, nonAxiomAntecedent),
                  "B'", 4, 3}});
}

void TutorWindowTest::exportsConversationToPdf() {
    const Grammar grammar = TutorGrammarFixtures::makeLl1EpsilonGrammar();

    LLTutorWindow tutor(grammar, nullptr);
    tutor.setAnswerForTest(Ll1TutorTestUtils::tableSizeAnswer(grammar));
    tutor.submitForTest();

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString pdfPath = tempDir.filePath("ll_tutor_test.pdf");
    tutor.exportConversationToPdf(pdfPath);

    QFileInfo pdfInfo(pdfPath);
    QVERIFY(pdfInfo.exists());
    QVERIFY(pdfInfo.size() > 0);
}

void TutorWindowTest::stateCCorrectPathOpensExportActions() {
    const Grammar grammar = TutorGrammarFixtures::makeLl1SimpleGrammar();

    LLTutorWindow tutor(grammar, nullptr);

    runScenario(tutor, {{Ll1TutorTestUtils::tableSizeAnswer(grammar), "B", 1,
                         0}});
    answerRemainingRulesUntilStateC(tutor, grammar);
    QCOMPARE(tutor.currentStateForTest(), QString("C"));

    LLTableDialog* dialog = waitForTableDialog();
    auto*          table = dialog->findChild<QTableWidget*>("llTableWidget");
    QVERIFY(table != nullptr);

    QtModalTestUtils::submitLlTableDialog(
        dialog, QtModalTestUtils::buildExpectedTable(grammar, table));

    QTRY_COMPARE(tutor.currentStateForTest(), QString("fin"));
    QVERIFY(waitForTutorButton(tutor, "llTutorExportPdfButton") != nullptr);
    QVERIFY(waitForTutorButton(tutor, "llTutorExitButton") != nullptr);
}

void TutorWindowTest::stateCWrongAttemptsReachCPrimeAndRecover() {
    const Grammar grammar = TutorGrammarFixtures::makeLl1SimpleGrammar();

    LLTutorWindow tutor(grammar, nullptr);

    runScenario(tutor, {{Ll1TutorTestUtils::tableSizeAnswer(grammar), "B", 1,
                         0}});
    answerRemainingRulesUntilStateC(tutor, grammar);
    QCOMPARE(tutor.currentStateForTest(), QString("C"));

    LLTableDialog* dialog = waitForTableDialog();
    auto* table = dialog->findChild<QTableWidget*>("llTableWidget");
    QVERIFY(table != nullptr);

    const QVector<QVector<QString>> expectedTable =
        QtModalTestUtils::buildExpectedTable(grammar, table);
    const QVector<QVector<QString>> wrongTable =
        QtModalTestUtils::buildWrongTable(grammar, table);
    const int wrongRow = QtModalTestUtils::firstNonEmptyCellRow(expectedTable);
    const int wrongCol = QtModalTestUtils::firstNonEmptyCellCol(expectedTable);
    QVERIFY(wrongRow >= 0);
    QVERIFY(wrongCol >= 0);

    for (int attempt = 0; attempt < 3; ++attempt) {
        QtModalTestUtils::scheduleMessageBoxResponse(QMessageBox::Ok);
        QtModalTestUtils::submitLlTableDialog(dialog, wrongTable);
        QTRY_COMPARE(tutor.currentStateForTest(), QString("C"));
        QCOMPARE(tutor.wrongCountForTest(), 0);
        QCOMPARE(QtModalTestUtils::cellBackground(table, wrongRow, wrongCol),
                 QColor("#d9534f"));
        QVERIFY(dialog->isVisible());
    }

    QtModalTestUtils::scheduleMessageBoxResponse(QMessageBox::Ok);
    QtModalTestUtils::submitLlTableDialog(dialog, wrongTable);
    QTRY_COMPARE(tutor.currentStateForTest(), QString("C"));
    QCOMPARE(tutor.wrongCountForTest(), 0);
    QVERIFY(dialog->isVisible());

    QtModalTestUtils::submitLlTableDialog(dialog, wrongTable);
    QTRY_COMPARE(tutor.currentStateForTest(), QString("C'"));
    QCOMPARE(tutor.wrongCountForTest(), 1);

    dialog = waitForTableDialog();
    table  = dialog->findChild<QTableWidget*>("llTableWidget");
    QVERIFY(table != nullptr);

    QtModalTestUtils::submitLlTableDialog(dialog, wrongTable);
    QTRY_COMPARE(tutor.currentStateForTest(), QString("C'"));
    QCOMPARE(tutor.wrongCountForTest(), 2);

    dialog = waitForTableDialog();
    table  = dialog->findChild<QTableWidget*>("llTableWidget");
    QVERIFY(table != nullptr);

    QtModalTestUtils::submitLlTableDialog(dialog, expectedTable);
    QTRY_COMPARE(tutor.currentStateForTest(), QString("fin"));
}

void TutorWindowTest::tableDialogCancelNoReopensAndYesRequestsExit() {
    const Grammar grammar = TutorGrammarFixtures::makeLl1SimpleGrammar();

    LLTutorWindow tutor(grammar, nullptr);
    QSignalSpy    exitSpy(&tutor, &LLTutorWindow::exitRequested);

    runScenario(tutor, {{Ll1TutorTestUtils::tableSizeAnswer(grammar), "B", 1,
                         0}});
    answerRemainingRulesUntilStateC(tutor, grammar);

    LLTableDialog* dialog = waitForTableDialog();
    QVERIFY(dialog != nullptr);

    QtModalTestUtils::scheduleMessageBoxResponse(QMessageBox::No);
    dialog->reject();

    LLTableDialog* reopenedDialog = waitForTableDialog();
    QVERIFY(reopenedDialog != nullptr);
    QVERIFY(reopenedDialog != dialog);
    QCOMPARE(exitSpy.count(), 0);
    QCOMPARE(tutor.currentStateForTest(), QString("C"));

    QtModalTestUtils::scheduleMessageBoxResponse(QMessageBox::Yes);
    reopenedDialog->reject();

    QTRY_COMPARE(exitSpy.count(), 1);
    const QList<QVariant> arguments = exitSpy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), false);
}

void TutorWindowTest::tableDialogCancelInCPrimeNoReopensAndYesRequestsExit() {
    const Grammar grammar = TutorGrammarFixtures::makeLl1SimpleGrammar();

    LLTutorWindow tutor(grammar, nullptr);
    QSignalSpy    exitSpy(&tutor, &LLTutorWindow::exitRequested);

    runScenario(tutor, {{Ll1TutorTestUtils::tableSizeAnswer(grammar), "B", 1,
                         0}});
    answerRemainingRulesUntilStateC(tutor, grammar);

    driveTutorToCPrime(tutor, grammar);

    LLTableDialog* dialog = waitForTableDialog();
    QVERIFY(dialog != nullptr);

    dialog = waitForTableDialog();
    QVERIFY(dialog != nullptr);

    QtModalTestUtils::scheduleMessageBoxResponse(QMessageBox::No);
    dialog->reject();

    LLTableDialog* reopenedDialog = waitForTableDialog();
    QVERIFY(reopenedDialog != nullptr);
    QVERIFY(reopenedDialog != dialog);
    QCOMPARE(exitSpy.count(), 0);
    QCOMPARE(tutor.currentStateForTest(), QString("C'"));

    QtModalTestUtils::scheduleMessageBoxResponse(QMessageBox::Yes);
    reopenedDialog->reject();

    QTRY_COMPARE(exitSpy.count(), 1);
    const QList<QVariant> arguments = exitSpy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), false);
}

void TutorWindowTest::exportButtonExportsPdf() {
    const Grammar grammar = TutorGrammarFixtures::makeLl1SimpleGrammar();

    LLTutorWindow tutor(grammar, nullptr);

    runScenario(tutor, {{Ll1TutorTestUtils::tableSizeAnswer(grammar), "B", 1,
                         0}});
    answerRemainingRulesUntilStateC(tutor, grammar);

    LLTableDialog* dialog = waitForTableDialog();
    auto*          table = dialog->findChild<QTableWidget*>("llTableWidget");
    QVERIFY(table != nullptr);

    QtModalTestUtils::submitLlTableDialog(
        dialog, QtModalTestUtils::buildExpectedTable(grammar, table));

    QTRY_COMPARE(tutor.currentStateForTest(), QString("fin"));

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString pdfPath = tempDir.filePath("ll_export_via_dialog.pdf");

    tutor.setNextExportFilePathForTest(pdfPath);
    auto* exportButton = waitForTutorButton(tutor, "llTutorExportPdfButton");
    QTest::mouseClick(exportButton, Qt::LeftButton);

    QTRY_VERIFY(QFileInfo::exists(pdfPath));
    QFileInfo pdfInfo(pdfPath);
    QVERIFY(pdfInfo.size() > 0);
}

void TutorWindowTest::exitButtonFinishesWithoutExport() {
    const Grammar grammar = TutorGrammarFixtures::makeLl1SimpleGrammar();

    LLTutorWindow tutor(grammar, nullptr);
    QSignalSpy    exitSpy(&tutor, &LLTutorWindow::exitRequested);

    runScenario(tutor, {{Ll1TutorTestUtils::tableSizeAnswer(grammar), "B", 1,
                         0}});
    answerRemainingRulesUntilStateC(tutor, grammar);

    LLTableDialog* dialog = waitForTableDialog();
    auto*          table = dialog->findChild<QTableWidget*>("llTableWidget");
    QVERIFY(table != nullptr);

    QtModalTestUtils::submitLlTableDialog(
        dialog, QtModalTestUtils::buildExpectedTable(grammar, table));

    QTRY_COMPARE(tutor.currentStateForTest(), QString("fin"));

    auto* exitButton = waitForTutorButton(tutor, "llTutorExitButton");
    QVERIFY(exitButton != nullptr);
    QTest::mouseClick(exitButton, Qt::LeftButton);

    QTRY_COMPARE(exitSpy.count(), 1);
    const QList<QVariant> arguments = exitSpy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);
}

void TutorWindowTest::smokeGuiFindsCoreWidgets() {
    const Grammar grammar = TutorGrammarFixtures::makeLl1SimpleGrammar();

    LLTutorWindow tutor(grammar, nullptr);
    tutor.show();
    QCoreApplication::processEvents();

    QVERIFY(tutor.findChild<QWidget*>("listWidget") != nullptr);
    QVERIFY(tutor.findChild<QWidget*>("userResponse") != nullptr);
    QVERIFY(tutor.findChild<QWidget*>("confirmButton") != nullptr);
    QVERIFY(tutor.findChild<QWidget*>("backButton") != nullptr);

    tutor.setAnswerForTest(Ll1TutorTestUtils::tableSizeAnswer(grammar));
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("B"));
}
