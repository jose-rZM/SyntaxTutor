#include "tutor_grammar_fixtures.h"

#include <unordered_map>

namespace TutorGrammarFixtures {

Grammar makeLl1SimpleGrammar() {
    return Grammar({{"A", {{"a", "B"}, {"b"}}}, {"B", {{"c"}}}});
}

Grammar makeLl1EpsilonGrammar() {
    return Grammar({{"A", {{"B"}}}, {"B", {{"b"}, {"EPSILON"}}}});
}

Grammar makeLl1BranchingGrammar() {
    return Grammar({{"A", {{"a", "B"}, {"b", "C"}}},
                    {"B", {{"c"}, {"d"}}},
                    {"C", {{"e"}}}});
}

Grammar makeSlrSimpleGrammar() {
    return Grammar({{"A", {{"a", "A"}, {"b"}}}});
}

Grammar makeSlrChainGrammar() {
    return Grammar({{"A", {{"a", "B"}, {"b"}}}, {"B", {{"c"}}}});
}

Grammar makeSlrConflictGrammar() {
    return Grammar({{"A", {{"a"}, {"a", "A"}}}});
}

// The map constructor always adds S -> A $, so every grammar starts at A,
// and only lowercase symbols become terminals: operators and brackets are
// spelled as letters (p = +, m = *, l = '(', r = ')', i = id).

// The expression grammar with its left recursion removed (E, E', T, T', F).
// Two nullable non-terminals, and A nested between brackets, so FOLLOW(A)
// gets ')' as well as '$'.
Grammar makeLl1ExpressionGrammar() {
    return Grammar({{"A", {{"C", "B"}}},
                    {"B", {{"p", "C", "B"}, {"EPSILON"}}},
                    {"C", {{"E", "D"}}},
                    {"D", {{"m", "E", "D"}, {"EPSILON"}}},
                    {"E", {{"l", "A", "r"}, {"i"}}}});
}

// A run of optional parts before a terminal: FIRST(B C D e) has to look
// through three nullable non-terminals.
Grammar makeLl1NullableChainGrammar() {
    return Grammar({{"A", {{"B", "C", "D", "e"}, {"f"}}},
                    {"B", {{"b", "B"}, {"EPSILON"}}},
                    {"C", {{"c"}, {"EPSILON"}}},
                    {"D", {{"d"}, {"EPSILON"}}}});
}

// The axiom is nullable and appears nested in itself, so its FOLLOW set
// decides the epsilon column: FOLLOW(A) = {e, $}.
Grammar makeLl1FollowPropagationGrammar() {
    return Grammar({{"A", {{"B", "c"}, {"d", "A", "e"}, {"EPSILON"}}},
                    {"B", {{"b"}, {"f", "B"}}}});
}

// The left-recursive expression grammar: SLR(1) but not LL(1), with the
// classic shift/reduce conflicts in LR(0) that FOLLOW settles.
Grammar makeSlrExpressionGrammar() {
    return Grammar({{"A", {{"A", "p", "B"}, {"B"}}},
                    {"B", {{"B", "m", "C"}, {"C"}}},
                    {"C", {{"l", "A", "r"}, {"i"}}}});
}

// After "a e" both B -> e. and D -> e. are complete: a reduce/reduce
// conflict in LR(0) that FOLLOW(B) = {c} and FOLLOW(D) = {d} tell apart.
Grammar makeSlrReduceChoiceGrammar() {
    return Grammar({{"A", {{"a", "B", "c"}, {"a", "D", "d"}}},
                    {"B", {{"e"}}},
                    {"D", {{"e"}}}});
}

// An epsilon production: B -> . is complete already in the initial state,
// next to items that shift.
Grammar makeSlrEpsilonGrammar() {
    return Grammar({{"A", {{"B", "C"}, {"a"}}},
                    {"B", {{"b", "B"}, {"EPSILON"}}},
                    {"C", {{"c"}}}});
}

// A left-recursive list: not LL(1), and yet no LR(0) conflict at all.
Grammar makeSlrListGrammar() {
    return Grammar({{"A", {{"A", "c", "B"}, {"B"}}},
                    {"B", {{"b"}, {"d", "B"}}}});
}

QList<NamedGrammarFixture> llFixtures() {
    return {{"ll-simple", makeLl1SimpleGrammar()},
            {"ll-epsilon", makeLl1EpsilonGrammar()},
            {"ll-branching", makeLl1BranchingGrammar()},
            {"ll-expression", makeLl1ExpressionGrammar()},
            {"ll-nullable-chain", makeLl1NullableChainGrammar()},
            {"ll-follow-propagation", makeLl1FollowPropagationGrammar()}};
}

QList<NamedGrammarFixture> slrNoConflictFixtures() {
    return {{"slr-simple", makeSlrSimpleGrammar()},
            {"slr-chain", makeSlrChainGrammar()},
            {"slr-list", makeSlrListGrammar()},
            // The tutor counts shift/reduce as an LR(0) conflict, not
            // reduce/reduce: its two-reduction state goes through G.
            {"slr-reduce-choice", makeSlrReduceChoiceGrammar()}};
}

QList<NamedGrammarFixture> slrConflictFixtures() {
    return {{"slr-conflict", makeSlrConflictGrammar()},
            {"slr-expression", makeSlrExpressionGrammar()},
            {"slr-epsilon", makeSlrEpsilonGrammar()}};
}

QList<NamedGrammarFixture> slrFixturesWithCbEpsilon() {
    return {{"slr-simple", makeSlrSimpleGrammar()},
            {"slr-chain", makeSlrChainGrammar()},
            {"slr-epsilon", makeSlrEpsilonGrammar()}};
}

QList<NamedGrammarFixture> slrFixturesWithCbNonEpsilon() {
    return {{"slr-simple", makeSlrSimpleGrammar()},
            {"slr-chain", makeSlrChainGrammar()},
            {"slr-expression", makeSlrExpressionGrammar()}};
}

} // namespace TutorGrammarFixtures
