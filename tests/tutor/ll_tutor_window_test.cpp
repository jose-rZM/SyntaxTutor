#include "tutor_window_test.h"

#include "examreportdialog.h"
#include "ll1_tutor_test_utils.h"
#include "lltutorwindow.h"
#include "llwizard.h"
#include "qt_modal_test_utils.h"
#include "tutor_grammar_fixtures.h"
#include "tutor_scenario.h"

#include <QColor>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QSignalSpy>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTest>

#include <algorithm>

namespace {

template <typename Fn> void forEachLlFixture(Fn&& fn) {
    for (const auto& fixture : TutorGrammarFixtures::llFixtures()) {
        qInfo().noquote() << "LL fixture:" << fixture.name;
        fn(fixture);
    }
}

QString spacedCommaVariant(const QString& answer) {
    QStringList parts = answer.split(',', Qt::SkipEmptyParts);
    std::reverse(parts.begin(), parts.end());
    for (QString& part : parts) {
        part = QString(" %1 ").arg(part.trimmed());
    }
    return parts.join(", ");
}

QString spacedTableSizeVariant(const QString& answer) {
    const QStringList parts = answer.split(',', Qt::KeepEmptyParts);
    if (parts.size() != 2) {
        return answer;
    }
    return QString("  %1 , %2  ").arg(parts.at(0).trimmed(), parts.at(1).trimmed());
}

QVector<QVector<QString>> flexibleLlTable(const QVector<QVector<QString>>& raw) {
    QVector<QVector<QString>> formatted = raw;
    for (int row = 0; row < formatted.size(); ++row) {
        for (int col = 0; col < formatted[row].size(); ++col) {
            QString& cell = formatted[row][col];
            if (cell.isEmpty()) {
                continue;
            }
            if (cell.contains(' ')) {
                cell = QString("  %1  ").arg(cell.replace(' ', "   "));
            } else {
                cell = QString("  %1  ").arg(cell);
            }
        }
    }
    return formatted;
}

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

// -----------------------------------------------------------------------------
// Case: LL1-TC-01
// Summary:
//   Verifies that an LL(1) session can be opened with a fixed grammar and
//   without guided tutorial mode.
//
// Situation:
//   LL(1) tutor freshly created with a valid LL fixture and `tm=nullptr`.
//
// Action:
//   The test instantiates the tutor without any further interaction.
//
// Expected:
//   The tutor starts in state A and both counters begin at zero.
// -----------------------------------------------------------------------------
void TutorWindowTest::createsTutorWithNullTutorialManager() {
    forEachLlFixture([](const auto& fixture) {
        LLTutorWindow tutor(fixture.grammar, nullptr);

        QCOMPARE(tutor.currentStateForTest(), QString("A"));
        QCOMPARE(tutor.rightCountForTest(), 0);
        QCOMPARE(tutor.wrongCountForTest(), 0);
    });
}

// -----------------------------------------------------------------------------
// Case: LL1-TC-02
// Summary:
//   Exercises the full error path from state A until the tutor returns to the
//   size question and then moves into block B.
//
// Situation:
//   LL(1) tutor at startup, using several general-purpose LL fixtures.
//
// Action:
//   The user misses the table size, misses and fixes the non-terminal and
//   terminal counts, and then misses the final size prompt again.
//
// Expected:
//   The tutor advances through A1, A2 and A', updates the counters, and enters
//   B at the end.
// -----------------------------------------------------------------------------
void TutorWindowTest::stateAErrorPathAdvancesThroughAStates() {
    forEachLlFixture([](const auto& fixture) {
        const Grammar grammar = fixture.grammar;
        LLTutorWindow tutor(grammar, nullptr);

        runScenario(tutor,
                    {{"1,1", "A1", 0, 1},
                     {"99", "A1", 0, 2},
                     {Ll1TutorTestUtils::nonTerminalCountAnswer(grammar), "A2", 1,
                      2},
                     {"99", "A2", 1, 3},
                     {Ll1TutorTestUtils::terminalCountAnswer(grammar), "A'", 2, 3},
                     {"1,1", "B", 2, 4}});
    });
}

// -----------------------------------------------------------------------------
// Case: LL1-TC-03
// Summary:
//   Checks the direct happy path when the user gets the LL(1) table size right
//   on the first try.
//
// Situation:
//   LL(1) tutor freshly opened with several general-purpose LL fixtures.
//
// Action:
//   The user enters the correct table size immediately.
//
// Expected:
//   State A is resolved without visiting A1/A2 and the tutor enters B.
// -----------------------------------------------------------------------------
void TutorWindowTest::stateACorrectPathAdvancesToB() {
    forEachLlFixture([](const auto& fixture) {
        const Grammar grammar = fixture.grammar;
        LLTutorWindow tutor(grammar, nullptr);

        runScenario(tutor, {{Ll1TutorTestUtils::tableSizeAnswer(grammar), "B", 1,
                             0}});
    });
}

// -----------------------------------------------------------------------------
// Case: LL1-TC-05
// Summary:
//   Validates the axiom-specific branch in phase B, where the tutor skips the
//   FOLLOW question.
//
// Situation:
//   LL(1) tutor in B on the axiom rule from the simple fixture.
//
// Action:
//   The user misses the prediction symbols and then answers the FIRST/CAB set
//   for the axiom rule correctly.
//
// Expected:
//   The tutor enters B1 and then jumps directly to B' without visiting B2.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Case: LL1-TC-04, LL1-TC-06
// Summary:
//   Covers both the direct correct answer in B and the full support branch for a
//   non-axiom rule.
//
// Situation:
//   LL(1) tutor already in B after a correct resolution of block A.
//
// Action:
//   The user first answers one rule directly, then misses SD on another rule and
//   fixes CAB and SIG before returning to SD.
//
// Expected:
//   The counters reflect right and wrong answers, and the flow walks through B,
//   B1, B2 and B' correctly.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Case: LL1-TC-04
// Summary:
//   Checks that B1 and B2 keep the user on the same step while the answer stays
//   wrong.
//
// Situation:
//   LL(1) tutor on a non-axiom rule after the first B step for that rule has
//   already been resolved.
//
// Action:
//   The user fails twice in B1, fixes CAB, fails in B2 and then fixes SIG.
//
// Expected:
//   The tutor stays in B1 and B2 when appropriate and only advances once the
//   answer is corrected.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Case: Internal
// Summary:
//   Verifies PDF export of the conversation without depending on the tutor's
//   final flow.
//
// Situation:
//   LL(1) tutor using a fixture with epsilon and an already initialized
//   conversation.
//
// Action:
//   The test exports directly to a temporary path.
//
// Expected:
//   A PDF is generated, it exists, and it is not empty.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Case: LL1-TC-07
// Summary:
//   Checks that a correct LL(1) table reaches the final state and exposes the
//   tutor's final actions.
//
// Situation:
//   LL(1) tutor in state C with the table dialog open.
//
// Action:
//   The user fills the table correctly and presses `Finalizar`.
//
// Expected:
//   The tutor enters `fin` and the `Exportar PDF` and `Salir` buttons appear.
// -----------------------------------------------------------------------------
void TutorWindowTest::stateCCorrectPathOpensExportActions() {
    forEachLlFixture([](const auto& fixture) {
        const Grammar grammar = fixture.grammar;
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
    });
}

// -----------------------------------------------------------------------------
// Case: LL1-TC-08
// Summary:
//   Exercises the LL(1) table retry limit, the transition into C', and final
//   recovery with a correct table.
//
// Situation:
//   LL(1) tutor in C with the table dialog open on the simple fixture.
//
// Action:
//   The user submits several wrong tables until the retry limit is reached,
//   fails again in C', and finally fixes the table.
//
// Expected:
//   The expected messages and highlights appear, the tutor enters C' at the
//   limit, and reaches `fin` when the table is corrected.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Case: LL1-TC-09
// Summary:
//   Validates cancellation of the table dialog in C, both when continuing and
//   when leaving the tutor.
//
// Situation:
//   LL(1) tutor in C with the table dialog open.
//
// Action:
//   The user closes the dialog, answers `No` to the confirmation, closes it
//   again, and then answers `Yes`.
//
// Expected:
//   The dialog first reopens and the flow continues; then the tutor emits the
//   exit request.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Case: LL1-TC-09
// Summary:
//   Repeats dialog cancellation validation after the tutor has already moved
//   into state C'.
//
// Situation:
//   LL(1) tutor in C' after exhausting the allowed table attempts.
//
// Action:
//   The user closes the dialog, first continues, and then confirms exit.
//
// Expected:
//   The dialog reopens when appropriate and the tutor requests exit when the
//   user confirms.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Case: LL1-TC-07
// Summary:
//   Verifies real PDF export from the final LL(1) tutor action.
//
// Situation:
//   LL(1) tutor already in the final state after solving the table correctly.
//
// Action:
//   The user presses `Exportar PDF` and the test injects a temporary path.
//
// Expected:
//   A valid non-empty PDF is created at the selected path.
// -----------------------------------------------------------------------------
void TutorWindowTest::exportButtonExportsPdf() {
    forEachLlFixture([](const auto& fixture) {
        const Grammar grammar = fixture.grammar;
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
        const QString pdfPath = tempDir.filePath(
            QString("%1-export.pdf").arg(fixture.name));

        tutor.setNextExportFilePathForTest(pdfPath);
        auto* exportButton = waitForTutorButton(tutor, "llTutorExportPdfButton");
        QTest::mouseClick(exportButton, Qt::LeftButton);

        QTRY_VERIFY(QFileInfo::exists(pdfPath));
        QFileInfo pdfInfo(pdfPath);
        QVERIFY(pdfInfo.size() > 0);
    });
}

// -----------------------------------------------------------------------------
// Case: LL1-TC-07
// Summary:
//   Checks the clean exit path from the finished tutor without exporting the
//   conversation.
//
// Situation:
//   LL(1) tutor already in `fin`, with the final actions visible.
//
// Action:
//   The user presses `Salir`.
//
// Expected:
//   The tutor emits an exit request while applying the session results.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Case: Internal
// Summary:
//   Minimal visual smoke test to ensure the LL(1) window shows its essential
//   widgets and accepts a basic answer.
//
// Situation:
//   LL(1) tutor open with several general-purpose fixtures.
//
// Action:
//   The test shows the window, finds key widgets, and submits one simple correct
//   answer.
//
// Expected:
//   The UI appears without crashing and the tutor advances from A to B.
// -----------------------------------------------------------------------------
void TutorWindowTest::smokeGuiFindsCoreWidgets() {
    forEachLlFixture([](const auto& fixture) {
        const Grammar grammar = fixture.grammar;
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
    });
}

// -----------------------------------------------------------------------------
// Case: Internal
// Summary:
//   Ensures that the LL(1) tutor accepts reasonable formatting variants in user
//   input when the answer is otherwise correct.
//
// Situation:
//   LL(1) tutor using several general fixtures for table-size and set-based
//   questions.
//
// Action:
//   The user answers with additional spaces and common typing variants around
//   commas.
//
// Expected:
//   Correct answers are still accepted and the flow continues.
// -----------------------------------------------------------------------------
void TutorWindowTest::llAcceptsFlexibleUserFormatting() {
    forEachLlFixture([](const auto& fixture) {
        const Grammar grammar = fixture.grammar;
        LLTutorWindow tutor(grammar, nullptr);

        tutor.setAnswerForTest(
            spacedTableSizeVariant(Ll1TutorTestUtils::tableSizeAnswer(grammar)));
        tutor.submitForTest();
        QCOMPARE(tutor.currentStateForTest(), QString("B"));

        const QString prediction = Ll1TutorTestUtils::predictionSymbolsAnswer(
            grammar, tutor.currentRuleAntecedentForTest(),
            tutor.currentRuleConsequentForTest());
        tutor.setAnswerForTest(spacedCommaVariant(prediction));
        tutor.submitForTest();
        QVERIFY(tutor.rightCountForTest() >= 2);
    });
}

// -----------------------------------------------------------------------------
// Case: Internal
// Summary:
//   Checks that the LL(1) table accepts correct content even when the user types
//   extra spaces inside the cells.
//
// Situation:
//   LL(1) tutor in C with the table dialog open for several fixtures.
//
// Action:
//   The user fills the correct table with extra padding and spacing.
//
// Expected:
//   The table is still accepted and the tutor reaches the final state.
// -----------------------------------------------------------------------------
void TutorWindowTest::llAcceptsFlexibleTableCellFormatting() {
    forEachLlFixture([](const auto& fixture) {
        const Grammar grammar = fixture.grammar;
        LLTutorWindow tutor(grammar, nullptr);

        runScenario(tutor, {{Ll1TutorTestUtils::tableSizeAnswer(grammar), "B", 1,
                             0}});
        answerRemainingRulesUntilStateC(tutor, grammar);

        LLTableDialog* dialog = waitForTableDialog();
        auto*          table = dialog->findChild<QTableWidget*>("llTableWidget");
        QVERIFY(table != nullptr);

        const QVector<QVector<QString>> expected =
            QtModalTestUtils::buildExpectedTable(grammar, table);
        QtModalTestUtils::submitLlTableDialog(dialog, flexibleLlTable(expected));
        QTRY_COMPARE(tutor.currentStateForTest(), QString("fin"));
    });
}

// -----------------------------------------------------------------------------
// Case: LL1-TC-17
// Summary:
//   Verifies exam mode: wrong answers never branch into error states, no
//   feedback counters are visible, and an all-wrong exam grades 0 with the
//   report dialog shown at the end.
//
// Situation:
//   LL(1) tutor created in exam mode on the simple fixture.
//
// Action:
//   The user answers every question and the final table incorrectly.
//
// Expected:
//   The state machine walks A -> B -> C -> fin without visiting A1/B1/B2,
//   the exam session records every item as wrong, the grade is 0, and the
//   exam report dialog opens.
// -----------------------------------------------------------------------------
void TutorWindowTest::llExamModeWrongAnswersFollowMainPathAndGradeZero() {
    const Grammar grammar = TutorGrammarFixtures::makeLl1SimpleGrammar();
    LLTutorWindow tutor(grammar, nullptr, nullptr, true);

    QVERIFY(tutor.findChild<QLabel*>("cntRight")->isHidden());
    QVERIFY(tutor.findChild<QLabel*>("cntWrong")->isHidden());
    QVERIFY(tutor.findChild<QLabel*>("tick")->isHidden());
    QVERIFY(tutor.findChild<QLabel*>("cross")->isHidden());

    // A wrong answer advances to B directly: no A1 sub-question in exam mode.
    tutor.setAnswerForTest("999,999");
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("B"));
    QCOMPARE(tutor.wrongCountForTest(), 1);

    int guard = 0;
    while (tutor.currentStateForTest() == "B" && ++guard < 50) {
        tutor.setAnswerForTest("zz");
        tutor.submitForTest();
        QVERIFY(tutor.currentStateForTest() == "B" ||
                tutor.currentStateForTest() == "C");
    }
    QCOMPARE(tutor.currentStateForTest(), QString("C"));

    LLTableDialog* dialog = waitForTableDialog();
    auto*          table = dialog->findChild<QTableWidget*>("llTableWidget");
    QVERIFY(table != nullptr);
    QtModalTestUtils::submitLlTableDialog(
        dialog, QVector<QVector<QString>>(
                    table->rowCount(), QVector<QString>(table->columnCount())));

    QTRY_COMPARE(tutor.currentStateForTest(), QString("fin"));
    QVERIFY(tutor.examTotalForTest() > 0);
    QCOMPARE(tutor.examRightForTest(), 0);
    QCOMPARE(tutor.examGradeForTest(), 0.0);

    auto* report =
        QtModalTestUtils::waitForVisibleTopLevelWidget<ExamReportDialog>();
    QVERIFY(report != nullptr);
    auto* closeButton = report->findChild<QPushButton*>("examReportCloseButton");
    QVERIFY(closeButton != nullptr);
    QTest::mouseClick(closeButton, Qt::LeftButton);
}

// -----------------------------------------------------------------------------
// Case: LL1-TC-18
// Summary:
//   Verifies that a perfect exam scores 10 and counts every question plus
//   every expected table cell.
//
// Situation:
//   LL(1) tutor created in exam mode on each fixture.
//
// Action:
//   The user answers every question and the final table correctly.
//
// Expected:
//   The exam ends with grade 10.0 and right == total, and the report opens.
// -----------------------------------------------------------------------------
void TutorWindowTest::llExamModeAllCorrectScoresTen() {
    forEachLlFixture([](const auto& fixture) {
        const Grammar grammar = fixture.grammar;
        LLTutorWindow tutor(grammar, nullptr, nullptr, true);

        runScenario(tutor, {{Ll1TutorTestUtils::tableSizeAnswer(grammar), "B",
                             1, 0}});
        answerRemainingRulesUntilStateC(tutor, grammar);
        QCOMPARE(tutor.currentStateForTest(), QString("C"));

        LLTableDialog* dialog = waitForTableDialog();
        auto* table = dialog->findChild<QTableWidget*>("llTableWidget");
        QVERIFY(table != nullptr);
        QtModalTestUtils::submitLlTableDialog(
            dialog, QtModalTestUtils::buildExpectedTable(grammar, table));

        QTRY_COMPARE(tutor.currentStateForTest(), QString("fin"));
        QVERIFY(tutor.examTotalForTest() > 0);
        QCOMPARE(tutor.examRightForTest(), tutor.examTotalForTest());
        QCOMPARE(tutor.examGradeForTest(), 10.0);

        auto* report =
            QtModalTestUtils::waitForVisibleTopLevelWidget<ExamReportDialog>();
        QVERIFY(report != nullptr);
        auto* closeButton =
            report->findChild<QPushButton*>("examReportCloseButton");
        QVERIFY(closeButton != nullptr);
        QTest::mouseClick(closeButton, Qt::LeftButton);
        QTest::qWait(20);
    });
}

namespace {

LLWizard* waitForLlWizard() {
    return QtModalTestUtils::waitForVisibleTopLevelWidget<LLWizard>();
}

void finishLlWizard(LLWizard* wizard) {
    QPointer<LLWizard> wizardGuard(wizard);
    QVERIFY(wizardGuard != nullptr);
    int guard = 0;
    while (wizardGuard != nullptr && wizardGuard->isVisible() &&
           ++guard < 200) {
        auto* page = wizardGuard->currentPage();
        QVERIFY(page != nullptr);
        auto* edit = page->findChild<QLineEdit*>("llWizardAnswerEdit");
        QVERIFY(edit != nullptr);
        edit->setText(page->expectedForTest());
        QApplication::processEvents();

        const bool   isFinalPage = wizardGuard->isOnLastPage();
        QPushButton* nextButton  = wizardGuard->nextButton();
        QVERIFY(nextButton != nullptr);
        QTest::mouseClick(nextButton, Qt::LeftButton);
        QApplication::processEvents();

        if (isFinalPage) {
            break;
        }
    }
}

} // namespace

// -----------------------------------------------------------------------------
// Case: LL1-TC-19
// Summary:
//   Exercises the LL(1) guided mode chrome: opening from the table dialog,
//   inline feedback, whitespace-tolerant validation, and exiting early.
//
// Situation:
//   LL(1) tutor in C with the table dialog open on the simple fixture.
//
// Action:
//   The user opens guided mode, types a wrong answer, then the expected
//   production with extra spaces, advances one page and exits.
//
// Expected:
//   The wizard freezes the table dialog while open, shows feedback for wrong
//   input, accepts the spaced variant, and returns to the table on exit.
// -----------------------------------------------------------------------------
void TutorWindowTest::llGuidedModeWizardUsesCustomNavigationAndAllowsExit() {
    const Grammar grammar = TutorGrammarFixtures::makeLl1SimpleGrammar();
    LLTutorWindow tutor(grammar, nullptr);

    runScenario(tutor, {{Ll1TutorTestUtils::tableSizeAnswer(grammar), "B", 1,
                         0}});
    answerRemainingRulesUntilStateC(tutor, grammar);

    LLTableDialog* dialog = waitForTableDialog();
    auto*          table = dialog->findChild<QTableWidget*>("llTableWidget");
    auto* guidedButton = dialog->findChild<QPushButton*>("llTableGuidedButton");
    auto* submitButton = dialog->findChild<QPushButton*>("llTableSubmitButton");
    QVERIFY(table != nullptr);
    QVERIFY(guidedButton != nullptr);
    QVERIFY(submitButton != nullptr);

    QtModalTestUtils::requestLlGuidedMode(
        dialog, QVector<QVector<QString>>(
                    table->rowCount(), QVector<QString>(table->columnCount())));
    LLWizard* wizard = waitForLlWizard();
    QVERIFY(wizard != nullptr);
    QPointer<LLWizard> wizardGuard(wizard);
    auto*              page = wizard->currentPage();
    QVERIFY(page != nullptr);

    QVERIFY(!guidedButton->isEnabled());
    QVERIFY(!submitButton->isEnabled());

    auto* titleLabel = wizard->findChild<QLabel*>("llWizardTitle");
    QVERIFY(titleLabel != nullptr);
    QVERIFY(!titleLabel->text().isEmpty());

    auto* stepCounter = wizard->findChild<QLabel*>("llWizardStepCounter");
    QVERIFY(stepCounter != nullptr);
    QVERIFY(!stepCounter->text().isEmpty());

    auto* feedbackLabel = page->findChild<QLabel*>("llWizardFeedbackLabel");
    QVERIFY(feedbackLabel != nullptr);
    QVERIFY(feedbackLabel->text().isEmpty());

    auto* edit = page->findChild<QLineEdit*>("llWizardAnswerEdit");
    QVERIFY(edit != nullptr);
    edit->setText(QStringLiteral("wrong"));
    QApplication::processEvents();
    QVERIFY(!feedbackLabel->text().isEmpty());
    QVERIFY(!wizard->nextButton()->isEnabled());

    // Whitespace-tolerant validation: pad the expected production.
    const QString spaced =
        QStringLiteral("  %1  ").arg(page->expectedForTest()).replace(' ',
                                                                      "  ");
    edit->setText(spaced);
    QApplication::processEvents();
    QVERIFY(wizard->nextButton()->isEnabled());

    QPointer<LLWizardPage> firstPage(page);
    QTest::keyClick(edit, Qt::Key_Return);
    QTRY_VERIFY(firstPage == nullptr || wizard->currentPage() != firstPage);

    QTest::mouseClick(wizard->closeButton(), Qt::LeftButton);
    QTRY_VERIFY(wizardGuard == nullptr || !wizardGuard->isVisible());
    QVERIFY(guidedButton->isEnabled());
    QVERIFY(submitButton->isEnabled());
    QVERIFY(dialog->isVisible());
    QCOMPARE(tutor.currentStateForTest(), QString("C"));
}

// -----------------------------------------------------------------------------
// Case: LL1-TC-20
// Summary:
//   Completes the LL(1) guided mode and verifies the table dialog is restored
//   with the user's snapshot.
//
// Situation:
//   LL(1) tutor in C with the table dialog open on each fixture.
//
// Action:
//   The user opens guided mode and answers every cell correctly.
//
// Expected:
//   The wizard walks every non-empty cell of the LL(1) table, closes after
//   the last page, and control returns to the table dialog still in C.
// -----------------------------------------------------------------------------
void TutorWindowTest::llGuidedModeWizardCompletesAndReturnsToTable() {
    forEachLlFixture([](const auto& fixture) {
        const Grammar grammar = fixture.grammar;
        LLTutorWindow tutor(grammar, nullptr);

        runScenario(tutor, {{Ll1TutorTestUtils::tableSizeAnswer(grammar), "B",
                             1, 0}});
        answerRemainingRulesUntilStateC(tutor, grammar);

        LLTableDialog* dialog = waitForTableDialog();
        auto* table = dialog->findChild<QTableWidget*>("llTableWidget");
        auto* guidedButton =
            dialog->findChild<QPushButton*>("llTableGuidedButton");
        auto* submitButton =
            dialog->findChild<QPushButton*>("llTableSubmitButton");
        QVERIFY(table != nullptr);
        QVERIFY(guidedButton != nullptr);
        QVERIFY(submitButton != nullptr);

        QtModalTestUtils::requestLlGuidedMode(
            dialog,
            QVector<QVector<QString>>(table->rowCount(),
                                      QVector<QString>(table->columnCount())));
        LLWizard* wizard = waitForLlWizard();
        QVERIFY(wizard != nullptr);
        QPointer<LLWizard> wizardGuard(wizard);

        QVERIFY(!guidedButton->isEnabled());
        QVERIFY(!submitButton->isEnabled());

        finishLlWizard(wizard);
        QTRY_VERIFY(wizardGuard == nullptr || !wizardGuard->isVisible());
        QVERIFY(guidedButton->isEnabled());
        QVERIFY(submitButton->isEnabled());
        QVERIFY(dialog->isVisible());
        QCOMPARE(tutor.currentStateForTest(), QString("C"));

        // The exercise still finishes normally after using the wizard.
        QtModalTestUtils::submitLlTableDialog(
            dialog, QtModalTestUtils::buildExpectedTable(grammar, table));
        QTRY_COMPARE(tutor.currentStateForTest(), QString("fin"));
    });
}

// -----------------------------------------------------------------------------
// Case: LL1-TC-21
// Summary:
//   Verifies the guided mode is reachable from the C' retry table and that
//   exam mode hides the guided button.
//
// Situation:
//   LL(1) tutor driven to C' on the simple fixture; a second tutor in exam
//   mode at the C table.
//
// Action:
//   The test opens guided mode from the C' dialog, then checks the exam
//   tutor's table dialog.
//
// Expected:
//   The wizard opens in C'; in exam mode the guided button is hidden.
// -----------------------------------------------------------------------------
void TutorWindowTest::llGuidedModeAvailableInCPrimeAndHiddenInExam() {
    const Grammar grammar = TutorGrammarFixtures::makeLl1SimpleGrammar();
    {
        LLTutorWindow tutor(grammar, nullptr);
        runScenario(tutor, {{Ll1TutorTestUtils::tableSizeAnswer(grammar), "B",
                             1, 0}});
        answerRemainingRulesUntilStateC(tutor, grammar);
        driveTutorToCPrime(tutor, grammar);

        LLTableDialog* dialog = waitForTableDialog();
        auto*          table = dialog->findChild<QTableWidget*>("llTableWidget");
        auto* guidedButton =
            dialog->findChild<QPushButton*>("llTableGuidedButton");
        QVERIFY(table != nullptr);
        QVERIFY(guidedButton != nullptr);
        QVERIFY(guidedButton->isVisible());

        QtModalTestUtils::requestLlGuidedMode(
            dialog,
            QVector<QVector<QString>>(table->rowCount(),
                                      QVector<QString>(table->columnCount())));
        LLWizard* wizard = waitForLlWizard();
        QVERIFY(wizard != nullptr);
        QPointer<LLWizard> wizardGuard(wizard);
        QTest::mouseClick(wizard->closeButton(), Qt::LeftButton);
        QTRY_VERIFY(wizardGuard == nullptr || !wizardGuard->isVisible());
        QVERIFY(dialog->isVisible());

        // Leave the exercise cleanly.
        QtModalTestUtils::submitLlTableDialog(
            dialog, QtModalTestUtils::buildExpectedTable(grammar, table));
        QTRY_COMPARE(tutor.currentStateForTest(), QString("fin"));
    }

    LLTutorWindow examTutor(grammar, nullptr, nullptr, true);
    examTutor.setAnswerForTest(Ll1TutorTestUtils::tableSizeAnswer(grammar));
    examTutor.submitForTest();
    answerRemainingRulesUntilStateC(examTutor, grammar);
    QCOMPARE(examTutor.currentStateForTest(), QString("C"));

    LLTableDialog* examDialog = waitForTableDialog();
    QVERIFY(examDialog != nullptr);
    auto* examGuidedButton =
        examDialog->findChild<QPushButton*>("llTableGuidedButton");
    QVERIFY(examGuidedButton != nullptr);
    QVERIFY(!examGuidedButton->isVisible());

    auto* examTable = examDialog->findChild<QTableWidget*>("llTableWidget");
    QVERIFY(examTable != nullptr);
    QtModalTestUtils::submitLlTableDialog(
        examDialog,
        QtModalTestUtils::buildExpectedTable(grammar, examTable));
    QTRY_COMPARE(examTutor.currentStateForTest(), QString("fin"));
    auto* report =
        QtModalTestUtils::waitForVisibleTopLevelWidget<ExamReportDialog>();
    QVERIFY(report != nullptr);
    QTest::mouseClick(report->findChild<QPushButton*>("examReportCloseButton"),
                      Qt::LeftButton);
}
