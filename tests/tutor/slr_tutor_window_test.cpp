#include "tutor_window_test.h"

#include "appshortcuts.h"
#include "automatonview.h"
#include "grammar_parser.hpp"
#include "automatonviewerdialog.h"
#include "examreportdialog.h"
#include "qt_modal_test_utils.h"
#include "slr_tutor_test_utils.h"
#include "slrtutorwindow.h"
#include "slrwizard.h"
#include "tutor_grammar_fixtures.h"

#include <QCoreApplication>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QDebug>
#include <QFileInfo>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QSignalSpy>
#include <QTabWidget>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTest>
#include <algorithm>

namespace {

// A fully revealed automaton for @p grammar, laid out as the tutor does.
AutomatonView* buildRevealedAutomaton(const Grammar& grammar) {
    SLR1Parser parser(grammar);
    parser.MakeParser();
    QVector<AutomatonStateInfo>      states;
    QVector<AutomatonTransitionInfo> transitions;
    for (const state& st : parser.states_) {
        states.push_back(
            {st.id_, QString::fromStdString(parser.PrintItems(st.items_))});
    }
    for (const auto& [from, row] : parser.transitions_) {
        for (const auto& [symbol, to] : row) {
            transitions.push_back({from, QString::fromStdString(symbol), to});
        }
    }
    auto* view = new AutomatonView;
    view->resize(900, 560);
    view->setAutomaton(states, transitions);
    view->revealAll();
    view->show();
    QCoreApplication::processEvents();
    return view;
}

QGraphicsSimpleTextItem* nodeLabel(AutomatonView* view, const QString& name) {
    for (QGraphicsItem* item : view->scene()->items()) {
        auto* text = dynamic_cast<QGraphicsSimpleTextItem*>(item);
        if (text != nullptr && text->text() == name &&
            text->parentItem() == nullptr) {
            return text;
        }
    }
    return nullptr;
}

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

SLRWizard* waitForWizard() {
    return QtModalTestUtils::waitForVisibleTopLevelWidget<SLRWizard>();
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
// Case: SLR1-TC-LR0-ACCEPT
// Summary:
//   The accept item S -> A · $ is not a reduction, so a state holding it next
//   to an item that shifts has no LR(0) conflict.
//
// Situation:
//   slr-list, whose only such state is { S -> A · $, A -> A · c B }, and the
//   left-recursive expression grammar, with two real shift/reduce states and
//   one accept state.
//
// Expected:
//   No conflict for slr-list and exactly two for the expression grammar. The
//   answer used to depend on the order the items were iterated in, which
//   differs between standard libraries.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrLr0ConflictsIgnoreTheAcceptItem() {
    SLRTutorWindow list(TutorGrammarFixtures::makeSlrListGrammar(), nullptr);
    QVERIFY(list.solutionForF().isEmpty());

    SLRTutorWindow expression(TutorGrammarFixtures::makeSlrExpressionGrammar(),
                              nullptr);
    QCOMPARE(expression.solutionForF().size(), 2);
}

// -----------------------------------------------------------------------------
// Case: SLR1-TC-LR0-RR
// Summary:
//   A state with two complete items has a reduce-reduce conflict in LR(0),
//   just as a complete and a shifting item have a shift-reduce one.
//
// Situation:
//   slr-reduce-choice: after "a e" the state is { B -> e . , D -> e . },
//   with FOLLOW(B) = {c} and FOLLOW(D) = {d}.
//
// Expected:
//   F lists that state, and F-A asks for the reduce terminals of both items.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrReduceReduceIsAnLr0Conflict() {
    SLRTutorWindow tutor(TutorGrammarFixtures::makeSlrReduceChoiceGrammar(),
                         nullptr);
    QCOMPARE(tutor.solutionForF().size(), 1);

    driveSlrTutorToState(tutor, "FA");
    QCOMPARE(tutor.solutionForFA(),
             QSet<QString>({QStringLiteral("c"), QStringLiteral("d")}));
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
// Case: SLR1-TC-17A
// Summary:
//   Verifies that guided mode uses the custom wizard navigation and can be
//   exited without going backwards.
//
// Situation:
//   SLR(1) tutor in H with the table dialog visible.
//
// Action:
//   The user opens guided mode and then exits through the wizard cancel button.
//
// Expected:
//   The guided dialog uses the custom step layout and closes back to the table
//   dialog.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrGuidedModeWizardUsesCustomNavigationAndAllowsExit() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    SLRTutorWindow tutor(grammar, nullptr);
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    driveSlrTutorToH(tutor);

    SLRTableDialog* dialog = waitForSlrTableDialog();
    auto*           table = dialog->findChild<QTableWidget*>("slrTableWidget");
    auto* guidedButton =
        dialog->findChild<QPushButton*>("slrTableGuidedButton");
    auto* submitButton =
        dialog->findChild<QPushButton*>("slrTableSubmitButton");
    QVERIFY(table != nullptr);
    QVERIFY(guidedButton != nullptr);
    QVERIFY(submitButton != nullptr);

    QtModalTestUtils::requestSlrGuidedMode(
        dialog, QVector<QVector<QString>>(table->rowCount(),
                                          QVector<QString>(table->columnCount())));
    SLRWizard* wizard = waitForWizard();
    QVERIFY(wizard != nullptr);
    QPointer<SLRWizard> wizardGuard(wizard);
    auto* page = wizard->currentPage();
    QVERIFY(page != nullptr);

    QVERIFY(!guidedButton->isEnabled());
    QVERIFY(!submitButton->isEnabled());

    auto* titleLabel = wizard->findChild<QLabel*>("slrWizardTitle");
    QVERIFY(titleLabel != nullptr);
    QVERIFY(!titleLabel->text().isEmpty());

    auto* stepCounter = wizard->findChild<QLabel*>("slrWizardStepCounter");
    QVERIFY(stepCounter != nullptr);
    QVERIFY(!stepCounter->text().isEmpty());

    auto* feedbackLabel = page->findChild<QLabel*>("slrWizardFeedbackLabel");
    QVERIFY(feedbackLabel != nullptr);
    QVERIFY(feedbackLabel->text().isEmpty());

    auto* edit = page->findChild<QLineEdit*>("slrWizardAnswerEdit");
    QVERIFY(edit != nullptr);
    edit->setText(QStringLiteral("s"));
    QApplication::processEvents();

    QVERIFY(feedbackLabel->isVisible());
    QVERIFY(!feedbackLabel->text().isEmpty());

    edit->setText(page->expectedForTest());
    QApplication::processEvents();
    QPointer<SLRWizardPage> firstPage(page);
    QTest::keyClick(edit, Qt::Key_Return);
    QTRY_VERIFY(firstPage == nullptr || wizard->currentPage() != firstPage);

    auto* cancelButton = wizard->closeButton();
    QVERIFY(cancelButton != nullptr);
    QVERIFY(cancelButton->isVisibleTo(wizard));

    QTest::mouseClick(cancelButton, Qt::LeftButton);
    QTRY_VERIFY(wizardGuard == nullptr || !wizardGuard->isVisible());
    QVERIFY(guidedButton->isEnabled());
    QVERIFY(submitButton->isEnabled());
    QVERIFY(dialog->isVisible());
    QCOMPARE(tutor.currentStateForTest(), QString("H"));
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
    auto* guidedButton =
        dialog->findChild<QPushButton*>("slrTableGuidedButton");
    auto* submitButton =
        dialog->findChild<QPushButton*>("slrTableSubmitButton");
    QVERIFY(table != nullptr);
    QVERIFY(guidedButton != nullptr);
    QVERIFY(submitButton != nullptr);

    QtModalTestUtils::requestSlrGuidedMode(
        dialog, QVector<QVector<QString>>(table->rowCount(),
                                          QVector<QString>(table->columnCount())));
    SLRWizard* wizard = waitForWizard();
    QVERIFY(wizard != nullptr);
    QPointer<SLRWizard> wizardGuard(wizard);

    QVERIFY(!guidedButton->isEnabled());
    QVERIFY(!submitButton->isEnabled());

    SlrTutorTestUtils::finishWizard(wizard);
    QTRY_VERIFY(wizardGuard == nullptr || !wizardGuard->isVisible());
    QVERIFY(guidedButton->isEnabled());
    QVERIFY(submitButton->isEnabled());
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
// -----------------------------------------------------------------------------
// Case: SLR1-TC-ORDER
// Summary:
//   A grammar the user wrote is numbered in the order its rules were typed,
//   and the reduce indices in the final table follow that same numbering.
//
// Situation:
//   A grammar parsed from text written in reverse alphabetical order, so a
//   lexicographic fallback would produce a completely different numbering
//   (S,Z,Y,X,X versus S,X,X,Y,Z). The fresh axiom is named S because the
//   grammar leaves that name free.
//
// Action:
//   The rule numbering is checked, the tutor is driven to the final table,
//   and the table is submitted with the expected actions.
//
// Expected:
//   The numbering is the written one, the table is accepted, the tutor
//   reaches `fin` and nothing was counted wrong. Acceptance is what proves
//   the `rN` indices the tutor validates against match the ones it displays.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrUserGrammarKeepsWrittenRuleOrder() {
    const GrammarParseResult parsed =
        GrammarParser::Parse("Z -> X Y .\nY -> c .\nX -> a | b .");
    QVERIFY(parsed.Ok());
    const Grammar grammar = parsed.grammar;

    const auto  numbering = SlrTutorTestUtils::buildSortedGrammar(grammar);
    QStringList numbered;
    for (const auto& rule : numbering) {
        numbered.append(QStringLiteral("%1 -> %2")
                            .arg(rule.first, rule.second.join(QLatin1Char(' '))));
    }
    const QStringList expectedNumbering{
        QStringLiteral("S -> Z $"), QStringLiteral("Z -> X Y"),
        QStringLiteral("Y -> c"), QStringLiteral("X -> a"),
        QStringLiteral("X -> b")};
    QCOMPARE(numbered, expectedNumbering);

    SLRTutorWindow tutor(grammar, nullptr);
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    driveSlrTutorToH(tutor);

    SLRTableDialog* dialog = waitForSlrTableDialog();
    QVERIFY(dialog != nullptr);
    auto* table = dialog->findChild<QTableWidget*>("slrTableWidget");
    QVERIFY(table != nullptr);

    QtModalTestUtils::submitSlrTableDialog(
        dialog, SlrTutorTestUtils::buildExpectedTable(grammar, table));

    QTRY_COMPARE(tutor.currentStateForTest(), QString("fin"));
    QCOMPARE(tutor.wrongCountForTest(), 0);
}

// -----------------------------------------------------------------------------
// Case: SLR1-TC-A2-ITEM
// Summary:
//   The A2 question shows the grammar's own initial item.
//
// Situation:
//   A grammar typed by the user, whose start symbol is Z: its initial item
//   is S -> · Z $. The question used to show a fixed S -> · A $, which only
//   matched the generated grammars, while expecting Z as the answer.
//
// Expected:
//   The question reaching A2 names S -> · Z $.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrInitialItemQuestionUsesTheGrammar() {
    const GrammarParseResult parsed =
        GrammarParser::Parse("Z -> X Y .\nY -> c .\nX -> a | b .");
    QVERIFY(parsed.Ok());

    SLRTutorWindow tutor(parsed.grammar, nullptr);
    tutor.setAnswerForTest(QStringLiteral("x"));
    tutor.submitForTest();
    QCOMPARE(tutor.currentStateForTest(), QString("A1"));
    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    QCOMPARE(tutor.currentStateForTest(), QString("A2"));

    QString question;
    for (const QLabel* label : tutor.findChildren<QLabel*>()) {
        if (label->text().contains(QStringLiteral("Dado el ítem"))) {
            question = label->text();
        }
    }
    QVERIFY2(question.contains(QStringLiteral("S -&gt; · Z $")) ||
                 question.contains(QStringLiteral("S -> · Z $")),
             qPrintable(question));
}

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
        QtModalTestUtils::scheduleMessageBoxResponse(QMessageBox::Ok);
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

// -----------------------------------------------------------------------------
// Case: SLR1-TC-21
// Summary:
//   Verifies SLR exam mode: wrong answers never branch into error states or
//   retry loops, the guided mode button is hidden in the table dialog, and
//   the exam report appears at the end with a partial grade.
//
// Situation:
//   SLR(1) tutor created in exam mode on the simple fixture.
//
// Action:
//   The user answers every chat question with garbage, then submits the
//   correct final table.
//
// Expected:
//   The state machine only visits main-path states (no A1..A4, E1/E2 or
//   in-place retries), the table cells score full marks while the chat
//   questions score zero, and the report dialog opens with a grade strictly
//   between 0 and 10.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrExamModeWrongAnswersFollowMainPathAndShowReport() {
    const Grammar  grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();
    SLRTutorWindow tutor(grammar, nullptr, nullptr, true);

    QVERIFY(tutor.findChild<QLabel*>("cntRight")->isHidden());
    QVERIFY(tutor.findChild<QLabel*>("cntWrong")->isHidden());

    const QSet<QString> mainPathStates = {"A", "B",  "C", "CA", "CB", "D",
                                          "E", "F",  "FA", "G", "H",  "fin"};

    // With garbage answers the exam must still walk only main-path states:
    // error sub-states or in-place retries would loop here forever, so the
    // guard doubles as the no-retry assertion.
    int guard = 0;
    while (tutor.currentStateForTest() != "H" && ++guard < 200) {
        QVERIFY2(mainPathStates.contains(tutor.currentStateForTest()),
                 qPrintable(QString("unexpected state %1")
                                .arg(tutor.currentStateForTest())));
        tutor.setAnswerForTest("zz");
        tutor.submitForTest();
    }
    QCOMPARE(tutor.currentStateForTest(), QString("H"));

    SLRTableDialog* dialog = waitForSlrTableDialog();
    QVERIFY(dialog != nullptr);
    auto* guidedButton = dialog->findChild<QPushButton*>("slrTableGuidedButton");
    QVERIFY(guidedButton != nullptr);
    QVERIFY(!guidedButton->isVisible());

    auto* table = dialog->findChild<QTableWidget*>("slrTableWidget");
    QVERIFY(table != nullptr);
    QtModalTestUtils::submitSlrTableDialog(
        dialog, SlrTutorTestUtils::buildExpectedTable(grammar, table));

    QTRY_COMPARE(tutor.currentStateForTest(), QString("fin"));
    QVERIFY(tutor.examTotalForTest() > 0);
    QVERIFY(tutor.examRightForTest() > 0);
    QVERIFY(tutor.examRightForTest() < tutor.examTotalForTest());
    QVERIFY(tutor.examGradeForTest() > 0.0);
    QVERIFY(tutor.examGradeForTest() < 10.0);

    auto* report =
        QtModalTestUtils::waitForVisibleTopLevelWidget<ExamReportDialog>();
    QVERIFY(report != nullptr);
    auto* closeButton = report->findChild<QPushButton*>("examReportCloseButton");
    QVERIFY(closeButton != nullptr);
    QTest::mouseClick(closeButton, Qt::LeftButton);
}

// -----------------------------------------------------------------------------
// Case: SLR1-TC-22
// Summary:
//   Verifies the LR(0) automaton is fully disabled in exam mode: no view, no
//   button, and the viewer cannot be opened.
//
// Situation:
//   SLR(1) tutor created in exam mode, compared against a normal tutor.
//
// Action:
//   The test inspects the automaton button and tries to open the viewer.
//
// Expected:
//   In exam mode no AutomatonView exists, the button is hidden, and no
//   viewer dialog appears; in normal mode the button exists.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrAutomatonHiddenInExamMode() {
    const Grammar  grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();
    SLRTutorWindow tutor(grammar, nullptr, nullptr, true);

    QVERIFY(tutor.findChild<AutomatonView*>() == nullptr);
    auto* button = tutor.findChild<QPushButton*>("automatonButton");
    QVERIFY(button != nullptr);
    QVERIFY(button->isHidden());

    tutor.openAutomatonViewer();
    QTest::qWait(20);
    QVERIFY(QtModalTestUtils::findVisibleTopLevelWidget<AutomatonViewerDialog>() ==
            nullptr);

    SLRTutorWindow normalTutor(grammar, nullptr);
    QVERIFY(normalTutor.findChild<AutomatonView*>() != nullptr);
    auto* normalButton =
        normalTutor.findChild<QPushButton*>("automatonButton");
    QVERIFY(normalButton != nullptr);
    QVERIFY(!normalButton->isHidden());
}

// -----------------------------------------------------------------------------
// Case: SLR1-TC-23
// Summary:
//   Verifies I0 only appears in the automaton after the student constructs
//   it, both on the direct path (A correct) and the fallback path (A').
//
// Situation:
//   Two SLR(1) tutors on the simple fixture.
//
// Action:
//   One tutor answers A correctly; the other fails A and walks the
//   A1..A4 -> A' fallback.
//
// Expected:
//   The automaton is empty before the construction and shows exactly I0
//   afterwards in both paths.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrAutomatonRevealsI0AfterInitialConstruction() {
    const Grammar grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();

    {
        SLRTutorWindow tutor(grammar, nullptr);
        auto*          view = tutor.findChild<AutomatonView*>();
        QVERIFY(view != nullptr);
        QCOMPARE(view->visibleStateCount(), 0);

        SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
        QCOMPARE(tutor.currentStateForTest(), QString("B"));
        QCOMPARE(view->visibleStateCount(), 1);
        QVERIFY(view->isStateVisible(0));
    }

    {
        SLRTutorWindow tutor(grammar, nullptr);
        auto*          view = tutor.findChild<AutomatonView*>();
        QVERIFY(view != nullptr);

        tutor.setAnswerForTest(QStringLiteral("zz"));
        tutor.submitForTest();
        QCOMPARE(tutor.currentStateForTest(), QString("A1"));
        QCOMPARE(view->visibleStateCount(), 0);

        int guard = 0;
        while (tutor.currentStateForTest() != "B" && ++guard < 20) {
            SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
            QVERIFY(tutor.currentStateForTest() == "B" ||
                    view->visibleStateCount() == 0);
        }
        QCOMPARE(tutor.currentStateForTest(), QString("B"));
        QCOMPARE(view->visibleStateCount(), 1);
        QVERIFY(view->isStateVisible(0));
    }
}

// -----------------------------------------------------------------------------
// Case: SLR1-TC-24
// Summary:
//   Verifies progressive reveal: during the LR(0) loop only constructed
//   states/transitions are visible, transitions appear on correct CB
//   answers, and reaching D switches to the fully revealed automaton.
//
// Situation:
//   SLR(1) tutor on the simple fixture driven with correct answers.
//
// Action:
//   The test walks the whole B -> C -> CA -> CB loop, sampling the
//   automaton before and after each CB answer.
//
// Expected:
//   Before the first CB answer no transition is visible and the automaton
//   is not fully revealed; transitions appear with CB answers and the
//   current state stays highlighted during the loop; at D everything is
//   visible.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrAutomatonProgressivelyRevealsAndCompletesAfterLoop() {
    const Grammar  grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();
    SLRTutorWindow tutor(grammar, nullptr);
    auto*          view = tutor.findChild<AutomatonView*>();
    QVERIFY(view != nullptr);
    QVERIFY(view->totalStateCount() > 1);

    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    QCOMPARE(view->visibleTransitionCount(), 0);

    bool sawTransitionReveal   = false;
    bool sawPartialDuringLoop  = false;
    int  guard                 = 0;
    while (tutor.currentStateForTest() != "D" && ++guard < 200) {
        const QString state = tutor.currentStateForTest();
        if (state == "C" || state == "CA" || state == "CB") {
            QCOMPARE(view->currentStateId(),
                     static_cast<int>(tutor.currentStateIdForTest()));
        }
        if (!view->isFullyRevealed()) {
            sawPartialDuringLoop = true;
        }

        const int transitionsBefore = view->visibleTransitionCount();
        const bool nonEpsilonCb =
            state == "CB" && tutor.currentCbSymbolForTest() != "EPSILON";
        SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);

        if (nonEpsilonCb &&
            view->visibleTransitionCount() > transitionsBefore) {
            sawTransitionReveal = true;
        }
    }
    QCOMPARE(tutor.currentStateForTest(), QString("D"));
    QVERIFY(sawPartialDuringLoop);
    QVERIFY(sawTransitionReveal);

    // Full consultation mode after the loop.
    QVERIFY(view->isFullyRevealed());
    QCOMPARE(view->visibleStateCount(), view->totalStateCount());
    QCOMPARE(view->currentStateId(), -1);
}

// -----------------------------------------------------------------------------
// Case: SLR1-TC-25
// Summary:
//   Verifies the consultation-mode highlights: conflict states in F/FA and
//   reduce states in G.
//
// Situation:
//   SLR(1) tutors on the conflict fixture (for F/FA) and the simple
//   fixture (for G).
//
// Action:
//   The test drives each tutor to F and G respectively.
//
// Expected:
//   In F the conflict states are marked; in G the reduce-capable states
//   are marked and the state under analysis is the current one.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrAutomatonHighlightsConflictAndReduceStates() {
    {
        const Grammar  grammar = TutorGrammarFixtures::makeSlrConflictGrammar();
        SLRTutorWindow tutor(grammar, nullptr);
        auto*          view = tutor.findChild<AutomatonView*>();
        QVERIFY(view != nullptr);

        SlrTutorTestUtils::driveTutorToState(tutor, "F");
        QVERIFY(view->isFullyRevealed());
        QVERIFY(!view->conflictStates().isEmpty());
        QCOMPARE(view->conflictStates(), tutor.solutionForF());
    }

    {
        const Grammar  grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();
        SLRTutorWindow tutor(grammar, nullptr);
        auto*          view = tutor.findChild<AutomatonView*>();
        QVERIFY(view != nullptr);

        SlrTutorTestUtils::driveTutorToState(tutor, "G");
        QVERIFY(view->isFullyRevealed());
        QVERIFY(view->conflictStates().isEmpty());
        QVERIFY(!view->reduceStates().isEmpty());
        QVERIFY(view->reduceStates().contains(
            static_cast<unsigned>(view->currentStateId())));
    }
}

// -----------------------------------------------------------------------------
// Case: SLR1-TC-26
// Summary:
//   Verifies the automaton button gating and that the viewer is reused, not
//   duplicated.
//
// Situation:
//   SLR(1) tutor on the simple fixture.
//
// Action:
//   The test checks the button before and after I0 is built, opens the
//   viewer twice, and closes/reopens it.
//
// Expected:
//   The button is disabled before I0 and enabled after; clicking twice keeps
//   exactly one viewer; closing then reopening yields a fresh single viewer.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrAutomatonButtonGatingAndViewerReuse() {
    const Grammar  grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();
    SLRTutorWindow tutor(grammar, nullptr);

    auto* button = tutor.findChild<QPushButton*>("automatonButton");
    QVERIFY(button != nullptr);
    QVERIFY(!button->isHidden());
    QVERIFY(!button->isEnabled()); // no I0 yet

    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    QCOMPARE(tutor.currentStateForTest(), QString("B"));
    QVERIFY(button->isEnabled()); // I0 constructed

    auto countViewers = []() {
        int count = 0;
        for (QWidget* widget : QApplication::topLevelWidgets()) {
            if (qobject_cast<AutomatonViewerDialog*>(widget) != nullptr &&
                widget->isVisible()) {
                ++count;
            }
        }
        return count;
    };

    QTest::mouseClick(button, Qt::LeftButton);
    auto* viewer =
        QtModalTestUtils::waitForVisibleTopLevelWidget<AutomatonViewerDialog>();
    QVERIFY(viewer != nullptr);
    QCOMPARE(countViewers(), 1);
    // The view moved into the viewer.
    QVERIFY(viewer->findChild<AutomatonView*>() != nullptr);
    // The zoom hint names the key the platform really uses: Qt maps Ctrl to
    // Command on macOS.
    auto* hint = viewer->findChild<QLabel*>("automatonViewerHint");
    QVERIFY(hint != nullptr);
    QVERIFY2(hint->text().startsWith(AppShortcuts::primaryModifierName() +
                                     QStringLiteral(" + ")),
             qPrintable(hint->text()));

    // Clicking again must not spawn a second viewer.
    QTest::mouseClick(button, Qt::LeftButton);
    QTest::qWait(20);
    QCOMPARE(countViewers(), 1);

    // Close and reopen: exactly one viewer again, view reclaimed in between.
    QPointer<AutomatonViewerDialog> viewerGuard(viewer);
    viewer->close();
    QTRY_VERIFY(viewerGuard == nullptr || !viewerGuard->isVisible());
    QTRY_COMPARE(countViewers(), 0);
    QVERIFY(tutor.findChild<AutomatonView*>() != nullptr); // reclaimed

    QTest::mouseClick(button, Qt::LeftButton);
    auto* reopened =
        QtModalTestUtils::waitForVisibleTopLevelWidget<AutomatonViewerDialog>();
    QVERIFY(reopened != nullptr);
    QCOMPARE(countViewers(), 1);
    reopened->close();
    QTRY_COMPARE(countViewers(), 0);
}

// -----------------------------------------------------------------------------
// Case: SLR1-TC-27
// Summary:
//   Verifies the viewer updates live while open and switches to full
//   consultation mode once the LR(0) loop completes.
//
// Situation:
//   SLR(1) tutor on the simple fixture with the viewer opened right after I0.
//
// Action:
//   The test opens the viewer, then keeps answering correctly until D while
//   sampling the embedded view.
//
// Expected:
//   The embedded view reveals more states as the student progresses and is
//   fully revealed once the tutor reaches D.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrAutomatonViewerUpdatesLiveAndReopens() {
    const Grammar  grammar = TutorGrammarFixtures::makeSlrSimpleGrammar();
    SLRTutorWindow tutor(grammar, nullptr);

    auto* button = tutor.findChild<QPushButton*>("automatonButton");
    QVERIFY(button != nullptr);

    SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor); // build I0
    QTest::mouseClick(button, Qt::LeftButton);
    auto* viewer =
        QtModalTestUtils::waitForVisibleTopLevelWidget<AutomatonViewerDialog>();
    QVERIFY(viewer != nullptr);

    auto* view = viewer->findChild<AutomatonView*>();
    QVERIFY(view != nullptr);
    const int revealedAtOpen = view->visibleStateCount();
    QVERIFY(revealedAtOpen >= 1);

    int guard = 0;
    while (tutor.currentStateForTest() != "D" && ++guard < 200) {
        SlrTutorTestUtils::submitCorrectAnswerForCurrentState(tutor);
    }
    QCOMPARE(tutor.currentStateForTest(), QString("D"));

    // The same embedded view received the live updates.
    QVERIFY(view->visibleStateCount() >= revealedAtOpen);
    QVERIFY(view->isFullyRevealed());
    QCOMPARE(view->visibleStateCount(), view->totalStateCount());

    viewer->close();
    QTest::qWait(20);
}

// -----------------------------------------------------------------------------
// Case: SLR1-AUTOMATON-GEOMETRY
// Summary:
//   Every edge of the automaton is drawn as one piece: the arrowhead sits on
//   the border of a state and the line runs into it, and no line crosses a
//   state on its way.
//
// Situation:
//   Two fully revealed automata with straight, long, backward and looping
//   edges, and states lined up in a column.
//
// Expected:
//   Each arrow tip touches a state border, each line ends inside an
//   arrowhead, and no point of any line lies inside a state.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrAutomatonEdgesMeetTheirArrowsAndAvoidNodes() {
    QList<Grammar> grammars = {
        Grammar({{"A", {{"B", "A"}, {"a"}}}, {"B", {{"B", "a"}, {"b"}}}})};
    const auto fixtures = TutorGrammarFixtures::slrNoConflictFixtures() +
                          TutorGrammarFixtures::slrConflictFixtures();
    for (const auto& fixture : fixtures) {
        grammars << fixture.grammar;
    }
    for (const Grammar& grammar : std::as_const(grammars)) {
        std::unique_ptr<AutomatonView> view(buildRevealedAutomaton(grammar));

        QList<QPointF> centers;
        qreal          radius = 0;
        QList<QPolygonF> arrows;
        QList<QPainterPath> lines;
        for (QGraphicsItem* item : view->scene()->items()) {
            if (auto* circle = dynamic_cast<QGraphicsEllipseItem*>(item)) {
                centers << circle->rect().center();
                radius = circle->rect().width() / 2.0;
            } else if (auto* arrow =
                           dynamic_cast<QGraphicsPolygonItem*>(item)) {
                arrows << arrow->polygon();
            } else if (auto* path = dynamic_cast<QGraphicsPathItem*>(item);
                       path != nullptr && path->parentItem() == nullptr &&
                       path->pen().style() != Qt::NoPen) {
                lines << path->path();
            }
        }
        QVERIFY(!centers.isEmpty());
        QCOMPARE(arrows.size(), lines.size());

        const auto distance = [](const QPointF& a, const QPointF& b) {
            return std::hypot(a.x() - b.x(), a.y() - b.y());
        };
        for (const QPolygonF& arrow : std::as_const(arrows)) {
            const QPointF tip = arrow.first();
            QVERIFY2(std::ranges::any_of(centers,
                                         [&](const QPointF& c) {
                                             return qAbs(distance(tip, c) -
                                                         radius) < 1.0;
                                         }),
                     "an arrow tip is off every state border");
        }
        for (const QPainterPath& line : std::as_const(lines)) {
            const QPointF end = line.pointAtPercent(1.0);
            QVERIFY2(std::ranges::any_of(arrows,
                                         [&](const QPolygonF& arrow) {
                                             return arrow.containsPoint(
                                                 end, Qt::OddEvenFill);
                                         }),
                     "a line ends outside every arrowhead");
            for (int i = 0; i <= 100; ++i) {
                const QPointF point = line.pointAtPercent(i / 100.0);
                for (const QPointF& c : std::as_const(centers)) {
                    QVERIFY2(distance(point, c) > radius - 1.0,
                             "a line runs through a state");
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Case: SLR1-AUTOMATON-CLICK
// Summary:
//   A click on a state shows its items next to it; they stay until another
//   click or Escape. Dragging to pan the view does not open them.
//
// Expected:
//   Click opens, a second click on the same state closes, Escape closes, a
//   click on empty space closes, and a drag starting on a state opens
//   nothing.
// -----------------------------------------------------------------------------
void TutorWindowTest::slrAutomatonClickShowsStateItems() {
    std::unique_ptr<AutomatonView> view(
        buildRevealedAutomaton(TutorGrammarFixtures::makeSlrSimpleGrammar()));
    QGraphicsSimpleTextItem* label = nodeLabel(view.get(), "I1");
    QVERIFY(label != nullptr);
    const QPoint onNode =
        view->mapFromScene(label->sceneBoundingRect().center());

    QTest::mouseClick(view->viewport(), Qt::LeftButton, {}, onNode);
    QCOMPARE(view->shownStateId(), 1);
    QTest::mouseClick(view->viewport(), Qt::LeftButton, {}, onNode);
    QCOMPARE(view->shownStateId(), -1);

    QTest::mouseClick(view->viewport(), Qt::LeftButton, {}, onNode);
    QTest::keyClick(view.get(), Qt::Key_Escape);
    QCOMPARE(view->shownStateId(), -1);

    QTest::mouseClick(view->viewport(), Qt::LeftButton, {}, onNode);
    QTest::mouseClick(view->viewport(), Qt::LeftButton, {}, QPoint(3, 3));
    QCOMPARE(view->shownStateId(), -1);

    QTest::mousePress(view->viewport(), Qt::LeftButton, {}, onNode);
    QTest::mouseMove(view->viewport(), onNode + QPoint(60, 40));
    QTest::mouseRelease(view->viewport(), Qt::LeftButton, {},
                        onNode + QPoint(60, 40));
    QCOMPARE(view->shownStateId(), -1);
}
