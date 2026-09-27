#pragma once

#include "grammar.hpp"

#include <QList>
#include <QString>

namespace TutorGrammarFixtures {

struct NamedGrammarFixture {
    QString name;
    Grammar grammar;
};

Grammar makeLl1SimpleGrammar();
Grammar makeLl1EpsilonGrammar();
Grammar makeLl1BranchingGrammar();
Grammar makeSlrSimpleGrammar();
Grammar makeSlrChainGrammar();
Grammar makeSlrConflictGrammar();

// Larger grammars, closer to what a student meets in class. Each one is
// there for a specific difficulty; see the definitions.
Grammar makeLl1ExpressionGrammar();
Grammar makeLl1NullableChainGrammar();
Grammar makeLl1FollowPropagationGrammar();
Grammar makeSlrExpressionGrammar();
Grammar makeSlrReduceChoiceGrammar();
Grammar makeSlrEpsilonGrammar();
Grammar makeSlrListGrammar();

QList<NamedGrammarFixture> llFixtures();
QList<NamedGrammarFixture> slrNoConflictFixtures();
QList<NamedGrammarFixture> slrConflictFixtures();
QList<NamedGrammarFixture> slrFixturesWithCbEpsilon();
QList<NamedGrammarFixture> slrFixturesWithCbNonEpsilon();

} // namespace TutorGrammarFixtures
