#include "tutor_window_test.h"

#include "qt_modal_test_utils.h"
#include "slr_tutor_test_utils.h"
#include "slrtutorwindow.h"
#include "tutor_grammar_fixtures.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QPointer>
#include <QPushButton>
#include <QSignalSpy>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTest>
#include <QWizard>

#include <algorithm>

namespace {

template <typename Fn> void forEachSlrNoConflictFixture(Fn&& fn) {
    for (const auto& fixture : TutorGrammarFixtures::slrNoConflictFixtures()) {
        qInfo().noquote() << "SLR no-conflict fixture:" << fixture.name;
        fn(fixture);
    }
}

template <typename Fn> void forEachSlrConflictFixture(Fn&& fn) {
    for (const auto& fixture : TutorGrammarFixtures::slrConflictFixtures()) {
        qInfo().noquote() << "SLR conflict fixture:" << fixture.name;
        fn(fixture);
    }
}

template <typename Fn> void forEachSlrCbEpsilonFixture(Fn&& fn) {
    for (const auto& fixture : TutorGrammarFixtures::slrFixturesWithCbEpsilon()) {
        qInfo().noquote() << "SLR CB-epsilon fixture:" << fixture.name;
        fn(fixture);
    }
}

template <typename Fn> void forEachSlrCbNonEpsilonFixture(Fn&& fn) {
    for (const auto& fixture : TutorGrammarFixtures::slrFixturesWithCbNonEpsilon()) {
        qInfo().noquote() << "SLR CB-non-epsilon fixture:" << fixture.name;
        fn(fixture);
    }
}

QString flexibleCommaAnswer(const QString& answer) {
    QStringList parts = answer.split(',', Qt::SkipEmptyParts);
    std::reverse(parts.begin(), parts.end());
    for (QString& part : parts) {
        part = QString(" %1 ").arg(part.trimmed());
    }
    return parts.join(" , ");
}

QString flexibleIdCountAnswer(const QString& answer) {
    QStringList parts = answer.split(',', Qt::SkipEmptyParts);
    std::reverse(parts.begin(), parts.end());
    for (QString& part : parts) {
        const QStringList kv = part.split(':', Qt::SkipEmptyParts);
        if (kv.size() == 2) {
            part = QString(" %1 : %2 ").arg(kv[0].trimmed(), kv[1].trimmed());
        }
    }
    return parts.join(" , ");
}

QString flexibleMultilineArrowAnswer(const QString& answer) {
    QStringList lines = answer.split('\n', Qt::SkipEmptyParts);
    for (QString& line : lines) {
        line = line.trimmed().replace("->", "  ->  ");
        line.replace('.', ". ");
        line = QString("  %1  ").arg(line);
    }
    return lines.join('\n');
}

QVector<QVector<QString>> flexibleSlrTable(const QVector<QVector<QString>>& raw) {
    QVector<QVector<QString>> formatted = raw;
    for (int row = 0; row < formatted.size(); ++row) {
        for (int col = 0; col < formatted[row].size(); ++col) {
            QString& cell = formatted[row][col];
            if (!cell.isEmpty()) {
                cell = QString("  %1  ").arg(cell);
            }
        }
    }
    return formatted;
}

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

// -----------------------------------------------------------------------------
// Case: SLR1-TC-01
// Summary:
//   Verifies that an SLR(1) session can be opened with a fixed grammar and
//   without guided tutorial mode.
//
// Situation:
//   SLR(1) tutor freshly created with no-conflict fixtures and `tm=nullptr`.
//
// Action:
//   The test instantiates the tutor without further interaction.
//
// Expected:
//   The tutor starts in state A and both counters begin at zero.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrCreatesTutorWithNullTutorialManager() {
    forEachSlrNoConflictFixture([](const auto& fixture) {
        SLRTutorWindow tutor(fixture.grammar, nullptr);

        QCOMPARE(tutor.currentStateForTest(), QString("A"));
        QCOMPARE(tutor.rightCountForTest(), 0);
        QCOMPARE(tutor.wrongCountForTest(), 0);
    });
}

// -----------------------------------------------------------------------------
// Case: SLR1-TC-03
// Summary:
//   Exercises the full error branch from state A until the tutor returns to B.
//
// Situation:
//   SLR(1) tutor at startup using no-conflict SLR fixtures.
//
// Action:
//   The user fails and then fixes the axiom, the symbol, the rules, the closure,
//   and the regenerated initial state in sequence.
//
// Expected:
//   The tutor walks through A1, A2, A3, A4 and A' before entering B.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrStateAErrorPathAdvancesThroughAprime() {
    forEachSlrNoConflictFixture([](const auto& fixture) {
        SLRTutorWindow tutor(fixture.grammar, nullptr);

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
    });
}

// -----------------------------------------------------------------------------
// Case: SLR1-TC-02
// Summary:
//   Checks the direct happy path where the initial LR(0) state is answered
//   correctly on the first try.
//
// Situation:
//   SLR(1) tutor freshly opened with no-conflict fixtures.
//
// Action:
//   The user enters I0 correctly.
//
// Expected:
//   The tutor enters B and records the expected correct answer.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrStateACorrectPathAdvancesToB() {
    forEachSlrNoConflictFixture([](const auto& fixture) {
        SLRTutorWindow tutor(fixture.grammar, nullptr);
        tutor.setAnswerForTest(SlrTutorTestUtils::correctAnswerForCurrentState(tutor));
        tutor.submitForTest();

        QCOMPARE(tutor.currentStateForTest(), QString("B"));
        QCOMPARE(tutor.rightCountForTest(), 1);
        QCOMPARE(tutor.wrongCountForTest(), 0);
    });
}

// -----------------------------------------------------------------------------
// Case: SLR1-TC-04
// Summary:
//   Validates the current behavior where B and C still advance even if the user
//   answers incorrectly.
//
// Situation:
//   SLR(1) tutor already in B after building the initial state correctly.
//
// Action:
//   The user answers incorrectly for both the number of generated states and the
//   number of items in the inspected state.
//
// Expected:
//   The tutor still advances from B to C and from C to CA, while incrementing
//   the wrong counter.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Case: SLR1-TC-05
// Summary:
//   Checks that CA repeats the same question while the set of symbols after the
//   dot is still wrong.
//
// Situation:
//   SLR(1) tutor in CA after entering the analysis of an LR(0) state.
//
// Action:
//   The user fails once and then corrects the requested set.
//
// Expected:
//   The tutor stays in CA until the answer is correct, then moves to CB or back
//   to B as appropriate.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Case: SLR1-TC-06
// Summary:
//   Validates the CB branch where the analyzed symbol is `EPSILON` and the only
//   valid answer is an empty input.
//
// Situation:
//   SLR(1) tutor in CB on a transition whose expected result is empty.
//
// Action:
//   The user first types a non-empty answer; then, in a second session, leaves
//   the response empty.
//
// Expected:
//   The non-empty answer counts as wrong and the empty answer is accepted.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrStateCBEpsilonBranchAcceptsOnlyEmpty() {
    forEachSlrCbEpsilonFixture([](const auto& fixture) {
        const Grammar grammar = fixture.grammar;

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
    });
}

// -----------------------------------------------------------------------------
// Case: SLR1-TC-07
// Summary:
//   Checks the CB branch for a real symbol, where the user must provide the item
//   set of the destination state.
//
// Situation:
//   SLR(1) tutor in CB on a non-empty transition.
//
// Action:
//   One session answers incorrectly; another answers correctly.
//
// Expected:
//   The flow advances in both cases according to the real behavior, but only the
//   correct answer counts as a success.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrStateCBNonEpsilonBranchAdvancesOnWrongAndCorrect() {
    forEachSlrCbNonEpsilonFixture([](const auto& fixture) {
        const Grammar grammar = fixture.grammar;

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
    });
}

// -----------------------------------------------------------------------------
// Case: SLR1-TC-08
// Summary:
//   Automatically traverses the B -> C -> CA -> CB loop until there are no more
//   pending states and the tutor reaches D.
//
// Situation:
//   SLR(1) tutor after resolving A correctly.
//
// Action:
//   The test keeps answering the construction loop steps correctly.
//
// Expected:
//   The tutor finishes the LR(0) collection and enters D.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrDriveThroughCollectionUntilD() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    driveSlrTutorToState(tutor, "D");
    QCOMPARE(tutor.currentStateForTest(), QString("D"));
}

// -----------------------------------------------------------------------------
// Case: SLR1-TC-10
// Summary:
//   Exercises the error path of block D until the tutor enters E.
//
// Situation:
//   SLR(1) tutor already in D, ready to ask about table size and symbol count.
//
// Action:
//   The user fails the table size, fails and fixes the row count, fails and
//   fixes the column count, and then fails the final size prompt again.
//
// Expected:
//   The tutor moves through D1, D2 and D' and enters E at the end.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Case: SLR1-TC-12
// Summary:
//   Checks the error path for E, E1 and E2 before moving into conflict
//   analysis.
//
// Situation:
//   SLR(1) tutor in E after completing block D.
//
// Action:
//   The user fails the number of states with completed items, fails and fixes
//   the ID list, and fails the `id:n` count list.
//
// Expected:
//   The tutor walks through E1 and E2 and eventually enters F.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Case: SLR1-TC-16
// Summary:
//   Validates the no-conflict LR(0) case, where F must accept an empty response
//   and jump directly to G.
//
// Situation:
//   SLR(1) tutor in F using no-conflict fixtures.
//
// Action:
//   The user leaves the response empty when asked about conflict states.
//
// Expected:
//   The tutor accepts the response and enters G.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrStateFNoConflictAdvancesToG() {
    forEachSlrNoConflictFixture([](const auto& fixture) {
        SLRTutorWindow tutor(fixture.grammar, nullptr);
        SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
        driveSlrTutorToState(tutor, "F");
        QCOMPARE(tutor.solutionForF().isEmpty(), true);

        tutor.setAnswerForTest(QString());
        tutor.submitForTest();
        QCOMPARE(tutor.currentStateForTest(), QString("G"));
    });
}

// -----------------------------------------------------------------------------
// Case: SLR1-TC-13, SLR1-TC-14, SLR1-TC-15
// Summary:
//   Checks LR(0) conflict detection, retry behavior in F, and per-conflict
//   resolution in FA.
//
// Situation:
//   SLR(1) tutor in F using a fixture with an LR(0) conflict.
//
// Action:
//   The user first fails the conflict-state list and then resolves each conflict
//   state by correcting the answer when needed.
//
// Expected:
//   The tutor stays in F or FA while answers are wrong and moves to G once all
//   conflicts have been resolved.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrStateFConflictBranchAdvancesToFAAndThenG() {
    forEachSlrConflictFixture([](const auto& fixture) {
        SLRTutorWindow tutor(fixture.grammar, nullptr);
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
    });
}

// -----------------------------------------------------------------------------
// Case: SLR1-TC-17
// Summary:
//   Verifies that block G repeats the question after a wrong answer and reaches
//   H once reductions are answered correctly.
//
// Situation:
//   SLR(1) tutor in G with at least one reducible state still pending.
//
// Action:
//   The user fails once and then the test completes the remainder of the block
//   correctly.
//
// Expected:
//   The tutor stays in G after the failure and eventually opens H after all
//   reductions are completed.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Case: SLR1-TC-18
// Summary:
//   Checks that an incorrect SLR table does not close the dialog and that the
//   problematic cell is highlighted.
//
// Situation:
//   SLR(1) tutor in H with the table dialog open.
//
// Action:
//   The user submits a table with semantically incorrect content.
//
// Expected:
//   Feedback is shown, the dialog stays open, and incorrect cells are marked in
//   red.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Case: SLR1-TC-18
// Summary:
//   Validates guided mode for the SLR table and the return to the main table
//   dialog once it is completed.
//
// Situation:
//   SLR(1) tutor in H with the table dialog visible.
//
// Action:
//   The user presses `Modo guiado` and completes the wizard step by step.
//
// Expected:
//   The wizard closes correctly and the table dialog remains available in H.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Case: SLR1-TC-20
// Summary:
//   Verifies cancellation of the SLR table dialog when the user chooses either
//   to continue or to leave the tutor.
//
// Situation:
//   SLR(1) tutor in H with the table dialog open.
//
// Action:
//   The user closes the dialog, first answers `No` and then `Yes` in the
//   confirmation dialog.
//
// Expected:
//   The dialog reopens when continuing and the tutor requests exit when the user
//   confirms.
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Case: SLR1-TC-19
// Summary:
//   Checks the correct final SLR tutor flow, including PDF export and a clean
//   exit path.
//
// Situation:
//   SLR(1) tutor in H with no-conflict fixtures and the table ready to be
//   completed.
//
// Action:
//   The user fills the correct table, exports the PDF, and then presses `Salir`.
//
// Expected:
//   The tutor enters `fin`, generates the PDF, and emits the exit request with
//   session results.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrFinalTableCorrectPathExportsAndExits() {
    forEachSlrNoConflictFixture([](const auto& fixture) {
        const Grammar grammar = fixture.grammar;
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
        const QString pdfPath = tempDir.filePath(
            QString("%1-export.pdf").arg(fixture.name));

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
    });
}

// -----------------------------------------------------------------------------
// Case: Internal
// Summary:
//   Minimal visual smoke test for the SLR(1) window to ensure it appears with
//   key widgets and accepts a basic answer.
//
// Situation:
//   SLR(1) tutor open with no-conflict fixtures.
//
// Action:
//   The test shows the window, locates essential widgets, and resolves one
//   initial correct answer.
//
// Expected:
//   The UI appears without crashing and the flow advances from A to B.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrSmokeGuiFindsCoreWidgets() {
    forEachSlrNoConflictFixture([](const auto& fixture) {
        SLRTutorWindow tutor(fixture.grammar, nullptr);
        tutor.show();
        QCoreApplication::processEvents();

        QVERIFY(tutor.findChild<QWidget*>("listWidget") != nullptr);
        QVERIFY(tutor.findChild<QWidget*>("userResponse") != nullptr);
        QVERIFY(tutor.findChild<QWidget*>("confirmButton") != nullptr);
        QVERIFY(tutor.findChild<QWidget*>("backButton") != nullptr);

        tutor.setAnswerForTest(SlrTutorTestUtils::correctAnswerForCurrentState(tutor));
        tutor.submitForTest();
        QCOMPARE(tutor.currentStateForTest(), QString("B"));
    });
}

// -----------------------------------------------------------------------------
// Case: Internal
// Summary:
//   Ensures that the SLR tutor accepts common formatting variants in otherwise
//   correct user answers.
//
// Situation:
//   SLR(1) tutor using both no-conflict and conflict fixtures for item, set, ID,
//   and `id:n` style questions.
//
// Action:
//   The user answers with extra spaces, spaced arrows, and typing variations
//   around commas and colons.
//
// Expected:
//   Correct answers are still accepted and the flow progresses like a normal
//   session.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrAcceptsFlexibleUserFormatting() {
    forEachSlrNoConflictFixture([](const auto& fixture) {
        SLRTutorWindow tutor(fixture.grammar, nullptr);

        tutor.setAnswerForTest(
            flexibleMultilineArrowAnswer(SlrTutorTestUtils::correctAnswerForCurrentState(tutor)));
        tutor.submitForTest();
        QCOMPARE(tutor.currentStateForTest(), QString("B"));

        tutor.setAnswerForTest(QString(" %1 ").arg(SlrTutorTestUtils::correctAnswerForCurrentState(tutor)));
        tutor.submitForTest();
        QCOMPARE(tutor.currentStateForTest(), QString("C"));

        tutor.setAnswerForTest(QString(" %1 ").arg(SlrTutorTestUtils::correctAnswerForCurrentState(tutor)));
        tutor.submitForTest();
        QCOMPARE(tutor.currentStateForTest(), QString("CA"));

        tutor.setAnswerForTest(flexibleCommaAnswer(
            SlrTutorTestUtils::correctAnswerForCurrentState(tutor)));
        tutor.submitForTest();
        QVERIFY(tutor.currentStateForTest() == "CB" || tutor.currentStateForTest() == "B");
    });

    forEachSlrConflictFixture([](const auto& fixture) {
        SLRTutorWindow tutor(fixture.grammar, nullptr);
        SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
        driveSlrTutorToState(tutor, "E");
        tutor.setAnswerForTest("999");
        tutor.submitForTest();
        QCOMPARE(tutor.currentStateForTest(), QString("E1"));
        tutor.setAnswerForTest(flexibleCommaAnswer(
            SlrTutorTestUtils::idSetAnswer(tutor.solutionForE1())));
        tutor.submitForTest();
        QCOMPARE(tutor.currentStateForTest(), QString("E2"));

        tutor.setAnswerForTest(flexibleIdCountAnswer(
            SlrTutorTestUtils::idCountAnswer(tutor.solutionForE2())));
        tutor.submitForTest();
        QCOMPARE(tutor.currentStateForTest(), QString("F"));
    });
}

// -----------------------------------------------------------------------------
// Case: Internal
// Summary:
//   Checks that the SLR table accepts correct entries even when cells contain
//   additional spaces.
//
// Situation:
//   SLR(1) tutor in H with the table dialog open and no-conflict fixtures.
//
// Action:
//   The user fills the correct table with extra padding inside the cells.
//
// Expected:
//   The table validates successfully and the tutor reaches the final state.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrAcceptsFlexibleTableCellFormatting() {
    forEachSlrNoConflictFixture([](const auto& fixture) {
        const Grammar grammar = fixture.grammar;
        SLRTutorWindow tutor(grammar, nullptr);
        SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
        driveSlrTutorToH(tutor);

        SLRTableDialog* dialog = waitForSlrTableDialog();
        auto*           table = dialog->findChild<QTableWidget*>("slrTableWidget");
        QVERIFY(table != nullptr);

        const QVector<QVector<QString>> expected =
            SlrTutorTestUtils::buildExpectedTable(grammar, table);
        QtModalTestUtils::submitSlrTableDialog(dialog, flexibleSlrTable(expected));
        QTRY_COMPARE(tutor.currentStateForTest(), QString("fin"));
    });
}
