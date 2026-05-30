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

QList<NamedGrammarFixture> llFixtures() {
    return {{"ll-simple", makeLl1SimpleGrammar()},
            {"ll-epsilon", makeLl1EpsilonGrammar()},
            {"ll-branching", makeLl1BranchingGrammar()}};
}

QList<NamedGrammarFixture> slrNoConflictFixtures() {
    return {{"slr-simple", makeSlrSimpleGrammar()},
            {"slr-chain", makeSlrChainGrammar()}};
}

QList<NamedGrammarFixture> slrConflictFixtures() {
    return {{"slr-conflict", makeSlrConflictGrammar()}};
}

QList<NamedGrammarFixture> slrFixturesWithCbEpsilon() {
    return {{"slr-simple", makeSlrSimpleGrammar()},
            {"slr-chain", makeSlrChainGrammar()}};
}

QList<NamedGrammarFixture> slrFixturesWithCbNonEpsilon() {
    return {{"slr-simple", makeSlrSimpleGrammar()},
            {"slr-chain", makeSlrChainGrammar()}};
}

} // namespace TutorGrammarFixtures
