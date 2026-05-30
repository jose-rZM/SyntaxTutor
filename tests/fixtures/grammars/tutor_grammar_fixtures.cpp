#include "tutor_grammar_fixtures.h"

#include <unordered_map>

namespace TutorGrammarFixtures {

Grammar makeLl1SimpleGrammar() {
    return Grammar({{"A", {{"a", "B"}, {"b"}}}, {"B", {{"c"}}}});
}

Grammar makeLl1EpsilonGrammar() {
    return Grammar({{"A", {{"B"}}}, {"B", {{"b"}, {"EPSILON"}}}});
}

Grammar makeSlrSimpleGrammar() {
    return Grammar({{"A", {{"a", "A"}, {"b"}}}});
}

Grammar makeSlrConflictGrammar() {
    return Grammar({{"A", {{"a"}, {"a", "A"}}}});
}

} // namespace TutorGrammarFixtures
