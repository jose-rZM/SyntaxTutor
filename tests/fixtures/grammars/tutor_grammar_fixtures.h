#pragma once

#include "grammar.hpp"

namespace TutorGrammarFixtures {

Grammar makeLl1SimpleGrammar();
Grammar makeLl1EpsilonGrammar();
Grammar makeSlrSimpleGrammar();
Grammar makeSlrConflictGrammar();

} // namespace TutorGrammarFixtures
