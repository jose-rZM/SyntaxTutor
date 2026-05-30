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

QList<NamedGrammarFixture> llFixtures();
QList<NamedGrammarFixture> slrNoConflictFixtures();
QList<NamedGrammarFixture> slrConflictFixtures();
QList<NamedGrammarFixture> slrFixturesWithCbEpsilon();
QList<NamedGrammarFixture> slrFixturesWithCbNonEpsilon();

} // namespace TutorGrammarFixtures
