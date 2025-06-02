#include "grammar.hpp"
#include "grammar_factory.hpp"
#include "ll1_parser.hpp"
#include "slr1_parser.hpp"
#include <algorithm>
#include <ranges>
#include <gtest/gtest.h>
namespace testing {
namespace internal {
template <> void PrintTo(const Lr0Item& item, std::ostream* os) {
    *os << item.ToString();
}
} // namespace internal
} // namespace testing

void SortProductions(Grammar& grammar) {
    for (auto& [nt, productions] : grammar.g_) {
        std::sort(productions.begin(), productions.end());
    }
}

TEST(GrammarFactoryTest, Lv1GrammarIsOneBaseGrammar) {
    GrammarFactory factory;

    factory.Init();
    Grammar g = factory.PickOne(1);

    ASSERT_FALSE(g.g_.empty());
    ASSERT_GE(g.g_.size(), 2);
    g.g_.erase("S");
    bool ret = std::ranges::any_of(factory.items, [&g](const GrammarFactory::FactoryItem &item) {
        return item.g_ == g.g_;
    });
    ASSERT_TRUE(ret);
}

TEST(GrammarFactoryTest, Lv2GrammarHaveSizeGt3)
{
    GrammarFactory factory;

    factory.Init();
    Grammar g = factory.PickOne(2);

    ASSERT_FALSE(g.g_.empty());
    ASSERT_GE(g.g_.size(), 3);
    ASSERT_TRUE(g.g_.contains("A"));
    ASSERT_TRUE(g.g_.contains("B"));
}

TEST(GrammarFactoryTest, Lv3GrammarHaveSizeGt4)
{
    GrammarFactory factory;

    factory.Init();
    Grammar g = factory.PickOne(3);

    ASSERT_FALSE(g.g_.empty());
    ASSERT_GE(g.g_.size(), 4);
    ASSERT_TRUE(g.g_.contains("A"));
    ASSERT_TRUE(g.g_.contains("B"));
    ASSERT_TRUE(g.g_.contains("C"));
}

TEST(GrammarTest, IsInfinite_WhenGrammarIsInfinite) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a", "A"});

    bool result = factory.IsInfinite(g);

    EXPECT_TRUE(result);
}

TEST(GrammarTest, IsInfinite_WhenGrammarIsNotInfinite) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {"b"});

    bool result = factory.IsInfinite(g);

    EXPECT_FALSE(result);
}

TEST(GrammarTest, HasUnreachableSymbols_WhenGrammarHasUnreachableSymbols) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a", "b"});
    g.AddProduction("B", {"c"});

    bool result = factory.HasUnreachableSymbols(g);

    EXPECT_TRUE(result);
}

TEST(GrammarTest, HasUnreachableSymbols_WhenGrammarHasNoUnreachableSymbols) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a", "b", "B"});
    g.AddProduction("B", {"c"});

    bool result = factory.HasUnreachableSymbols(g);

    EXPECT_FALSE(result);
}

TEST(GrammarTest, NullableSymbols_WhenNoSymbolsAreNullable) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("S'", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"S'", g.st_.EOL_});
    g.AddProduction("S'", {"A", "B"});
    g.AddProduction("A", {"a"});
    g.AddProduction("B", {"b"});

    std::unordered_set<std::string> nullable = factory.NullableSymbols(g);
    std::unordered_set<std::string> expected{};
    EXPECT_EQ(nullable, expected);
}

TEST(GrammarTest, HasLeftDirectRecursion_WhenGrammarHasLeftRecursion) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"A", "a"});
    g.AddProduction("A", {"b"});

    bool result = factory.HasDirectLeftRecursion(g);

    EXPECT_TRUE(result);
}

TEST(GrammarTest, NullableSymbols_WhenAllSymbolsAreNullable) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("S'", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"A", "B"});
    g.AddProduction("A", {g.st_.EPSILON_});
    g.AddProduction("B", {g.st_.EPSILON_});

    std::unordered_set<std::string> nullable = factory.NullableSymbols(g);
    std::unordered_set<std::string> expected{"S", "A", "B"};
    EXPECT_EQ(nullable, expected);
}

TEST(GrammarTest, NullableSymbols_WhenOnlyCertainSymbolsAreNullable) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("S'", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"A", "B"});
    g.AddProduction("A", {"a"});
    g.AddProduction("B", {g.st_.EPSILON_});

    std::unordered_set<std::string> nullable = factory.NullableSymbols(g);
    std::unordered_set<std::string> expected{"B"};
    EXPECT_EQ(nullable, expected);
}

TEST(GrammarTest, NullableSymbols_WhenNestedNullableSymbols) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("S'", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"S'", g.st_.EOL_});
    g.AddProduction("S'", {"A", "B"});
    g.AddProduction("A", {"C"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {g.st_.EPSILON_});

    std::unordered_set<std::string> nullable = factory.NullableSymbols(g);
    std::unordered_set<std::string> expected{"S'", "S", "A", "B", "C"};
    EXPECT_EQ(nullable, expected);
}

TEST(GrammarTest, NullableSymbols_WhenMixedNullableAndNonNullableSymbols) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("S'", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"S'", g.st_.EOL_});
    g.AddProduction("S'", {"A", "B"});
    g.AddProduction("A", {"a"});
    g.AddProduction("B", {"C"});
    g.AddProduction("C", {g.st_.EPSILON_});

    std::unordered_set<std::string> nullable = factory.NullableSymbols(g);
    std::unordered_set<std::string> expected{"B", "C"};
    EXPECT_EQ(nullable, expected);
}

TEST(GrammarTest, NullableSymbols_ComplexGrammar) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("S'", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("D", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"S'", g.st_.EOL_});
    g.AddProduction("S'", {"A", "B"});
    g.AddProduction("A", {"C", "D"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"a"});
    g.AddProduction("D", {g.st_.EPSILON_});

    std::unordered_set<std::string> nullable = factory.NullableSymbols(g);
    std::unordered_set<std::string> expected{"B", "D"};
    EXPECT_EQ(nullable, expected);
}

TEST(GrammarTest, NullableSymbols_RecursiveNullableSymbols) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("S'", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"S'", g.st_.EOL_});
    g.AddProduction("S'", {"A"});
    g.AddProduction("A", {"A", "B"});
    g.AddProduction("A", {g.st_.EPSILON_});
    g.AddProduction("B", {g.st_.EPSILON_});

    std::unordered_set<std::string> nullable = factory.NullableSymbols(g);
    std::unordered_set<std::string> expected{"S'", "S", "A", "B"};
    EXPECT_EQ(nullable, expected);
}

TEST(GrammarTest, NullableSymbols_DeeplyNestedAndRecursive) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("S'", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("D", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"S'", g.st_.EOL_});
    g.AddProduction("S'", {"A"});
    g.AddProduction("A", {"B", "C"});
    g.AddProduction("B", {"D", "E"});
    g.AddProduction("C", {"C", "A"});
    g.AddProduction("C", {g.st_.EPSILON_});
    g.AddProduction("D", {"D", "B"});
    g.AddProduction("D", {g.st_.EPSILON_});
    g.AddProduction("E", {g.st_.EPSILON_});

    std::unordered_set<std::string> nullable = factory.NullableSymbols(g);
    std::unordered_set<std::string> expected{"S'", "S", "A", "B",
                                             "C",  "D", "E"};
    EXPECT_EQ(nullable, expected);
}

TEST(GrammarTest, NullableSymbols_MixedWithComplexDependencies) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("S'", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("D", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("F", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"S'", g.st_.EOL_});
    g.AddProduction("S'", {"A", "B"});
    g.AddProduction("A", {"C", "D"});
    g.AddProduction("B", {"E", "F"});
    g.AddProduction("C", {"a"});
    g.AddProduction("D", {g.st_.EPSILON_});
    g.AddProduction("E", {"E", "B"});
    g.AddProduction("E", {g.st_.EPSILON_});
    g.AddProduction("F", {"b"});

    std::unordered_set<std::string> nullable = factory.NullableSymbols(g);
    std::unordered_set<std::string> expected{"D", "E"};
    EXPECT_EQ(nullable, expected);
}

TEST(GrammarTest, NullableSymbols_CyclicDependencies) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("S'", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("D", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"S'", g.st_.EOL_});
    g.AddProduction("S'", {"A"});

    g.AddProduction("A", {"B", "C"});
    g.AddProduction("B", {"D", "A"});
    g.AddProduction("C", {g.st_.EPSILON_});
    g.AddProduction("D", {"D", "B"});
    g.AddProduction("D", {g.st_.EPSILON_});

    std::unordered_set<std::string> nullable = factory.NullableSymbols(g);
    std::unordered_set<std::string> expected{"C", "D"};
    EXPECT_EQ(nullable, expected);
}

TEST(GrammarTest, HasIndirectLeftRecursion_NoIndirectLeftRecursion) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});

    g.AddProduction("A", {"a", "B"});
    g.AddProduction("B", {"b"});

    bool result = factory.HasIndirectLeftRecursion(g);

    EXPECT_FALSE(result);
}

TEST(GrammarTest, HasIndirectLeftRecursion_SimpleIndirectLeftRecursion) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});

    g.AddProduction("A", {"B", "a"});
    g.AddProduction("B", {"A", "a"});

    bool result = factory.HasIndirectLeftRecursion(g);

    EXPECT_TRUE(result);
}

TEST(GrammarTest, HasIndirectLeftRecursion_MultipleStepsIndirectLeftRecursion) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("a", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"B", "a"});
    g.AddProduction("B", {"C", "a"});
    g.AddProduction("C", {"A", "a"});

    bool result = factory.HasIndirectLeftRecursion(g);

    EXPECT_TRUE(result);
}

TEST(GrammarTest,
     HasIndirectLeftRecursion_ComplexRulesNoIndirectLeftRecursion) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"B", "C"});
    g.AddProduction("B", {"a"});
    g.AddProduction("C", {"b"});

    bool result = factory.HasIndirectLeftRecursion(g);

    EXPECT_FALSE(result);
}

TEST(GrammarTest,
     HasIndirectLeftRecursion_MultipleCyclesIndirectLeftRecursion) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("a", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"B", "a"});
    g.AddProduction("B", {"C", "a"});
    g.AddProduction("C", {"A", "a"});
    g.AddProduction("C", {"B", "a"});

    bool result = factory.HasIndirectLeftRecursion(g);

    EXPECT_TRUE(result);
}

TEST(GrammarTest,
     HasIndirectLeftRecursion_NullableSymbolEnablesIndirectLeftRecursion) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("d", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});

    g.AddProduction("A", {"B", "C", "d"});
    g.AddProduction("B", {"d"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"A"});

    bool result = factory.HasIndirectLeftRecursion(g);

    EXPECT_TRUE(result);
}

TEST(GrammarTest,
     HasIndirectLeftRecursion_NullableSymbolsNoIndirectLeftRecursion) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("d", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"B", "C", "d"});
    g.AddProduction("B", {"d"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"d"});

    bool result = factory.HasIndirectLeftRecursion(g);

    EXPECT_FALSE(result);
}

TEST(
    GrammarTest,
    HasIndirectLeftRecursion_ComplexNullableSymbolsEnableIndirectLeftRecursion) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("D", false);
    g.st_.PutSymbol("d", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"B", "C", "d"});
    g.AddProduction("B", {"D"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"A"});
    g.AddProduction("D", {"d"});
    g.AddProduction("D", {g.st_.EPSILON_});

    bool result = factory.HasIndirectLeftRecursion(g);

    EXPECT_TRUE(result);
}

TEST(GrammarTest,
     HasIndirectLeftRecursion_NullableSymbolChainEnablesIndirectLeftRecursion) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("D", false);
    g.st_.PutSymbol("d", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"B", "C", "d"});
    g.AddProduction("B", {"D"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"A"});
    g.AddProduction("D", {"d"});
    g.AddProduction("D", {g.st_.EPSILON_});

    bool result = factory.HasIndirectLeftRecursion(g);

    EXPECT_TRUE(result);
}

TEST(GrammarTest, HasLeftDirectRecursion_WhenGrammarHasNoLeftRecursion) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {"b"});

    bool result = factory.HasDirectLeftRecursion(g);

    EXPECT_FALSE(result);
}

TEST(GrammarTest, RemoveDirectLeftRecursion) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});

    g.AddProduction("A", {"A", "a"});
    g.AddProduction("A", {"b"});

    Grammar original = g;
    factory.RemoveLeftRecursion(g);

    EXPECT_NE(original.g_, g.g_);
    EXPECT_FALSE(factory.HasDirectLeftRecursion(g));
    EXPECT_NE(g.g_.find("A'"), g.g_.end());
}

TEST(GrammarTest, RemoveDirectLeftRecursion_WhenThereIsNoLeftRecursion) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});

    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {"b"});

    Grammar original = g;
    factory.RemoveLeftRecursion(g);

    EXPECT_EQ(original.g_, g.g_);
}

TEST(GrammarTest, LeftFactorize_Basic) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a", "b", "B"});
    g.AddProduction("A", {"a", "b", "c"});

    factory.LeftFactorize(g);

    Grammar g_factorized;

    g_factorized.st_.PutSymbol("S", false);
    g_factorized.st_.PutSymbol("A", false);
    g_factorized.st_.PutSymbol("A'", false);
    g_factorized.st_.PutSymbol("a", true);
    g_factorized.st_.PutSymbol("b", true);
    g_factorized.st_.PutSymbol("c", true);

    g_factorized.axiom_ = "S";

    g_factorized.AddProduction("S", {"A", g.st_.EOL_});
    g_factorized.AddProduction("A", {"a", "b", "A'"});
    g_factorized.AddProduction("A'", {"c"});
    g_factorized.AddProduction("A'", {"B"});

    SortProductions(g);
    SortProductions(g_factorized);

    EXPECT_EQ(g.g_.size(), 3);
    EXPECT_EQ(g.g_["S"].size(), 1);
    EXPECT_EQ(g.g_["A"].size(), 1);
    EXPECT_EQ(g.g_["A'"].size(), 2);
    EXPECT_EQ(g.g_, g_factorized.g_);
}

TEST(GrammarTest, LeftFactorize_MultipleCommonPrefixes) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol("d", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a", "b", "B"});
    g.AddProduction("A", {"a", "b", "c"});
    g.AddProduction("A", {"a", "b", "d"});

    factory.LeftFactorize(g);

    // Expected grammar after left factorization:
    // S -> A EOL
    // A -> a b A'
    // A' -> B
    // A' -> c
    // A' -> d

    Grammar g_factorized;

    g_factorized.st_.PutSymbol("S", false);
    g_factorized.st_.PutSymbol("A", false);
    g_factorized.st_.PutSymbol("A'", false);
    g_factorized.st_.PutSymbol("a", true);
    g_factorized.st_.PutSymbol("b", true);
    g_factorized.st_.PutSymbol("c", true);
    g_factorized.st_.PutSymbol("d", true);

    g_factorized.axiom_ = "S";

    g_factorized.AddProduction("S", {"A", g.st_.EOL_});
    g_factorized.AddProduction("A", {"a", "b", "A'"});
    g_factorized.AddProduction("A'", {"B"});
    g_factorized.AddProduction("A'", {"c"});
    g_factorized.AddProduction("A'", {"d"});

    SortProductions(g);
    SortProductions(g_factorized);

    EXPECT_EQ(g.g_.size(), 3);
    EXPECT_EQ(g.g_["S"].size(), 1);
    EXPECT_EQ(g.g_["A"].size(), 1);
    EXPECT_EQ(g.g_["A'"].size(), 3);
    EXPECT_EQ(g.g_, g_factorized.g_);
}

TEST(GrammarTest, LeftFactorize_WithEpsilon) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a", "b", "B"});
    g.AddProduction("A", {"a", "b", g.st_.EPSILON_});

    factory.LeftFactorize(g);

    // Expected grammar after left factorization:
    // S -> A EOL
    // A -> a b A'
    // A' -> B
    // A' -> ε

    Grammar g_factorized;

    g_factorized.st_.PutSymbol("S", false);
    g_factorized.st_.PutSymbol("A", false);
    g_factorized.st_.PutSymbol("A'", false);
    g_factorized.st_.PutSymbol("a", true);
    g_factorized.st_.PutSymbol("b", true);
    g_factorized.st_.PutSymbol(g.st_.EPSILON_, true);

    g_factorized.axiom_ = "S";

    g_factorized.AddProduction("S", {"A", g.st_.EOL_});
    g_factorized.AddProduction("A", {"a", "b", "A'"});
    g_factorized.AddProduction("A'", {"B"});
    g_factorized.AddProduction("A'", {g.st_.EPSILON_});

    SortProductions(g);
    SortProductions(g_factorized);

    EXPECT_EQ(g.g_.size(), 3);
    EXPECT_EQ(g.g_["S"].size(), 1);
    EXPECT_EQ(g.g_["A"].size(), 1);
    EXPECT_EQ(g.g_["A'"].size(), 2);
    EXPECT_EQ(g.g_, g_factorized.g_);
}

TEST(GrammarTest, LeftFactorize_NoCommonPrefix) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a", "b"});
    g.AddProduction("A", {"c"});

    factory.LeftFactorize(g);

    // Expected grammar (no changes):
    // S -> A EOL
    // A -> a b
    // A -> c

    Grammar g_factorized;

    g_factorized.st_.PutSymbol("S", false);
    g_factorized.st_.PutSymbol("A", false);
    g_factorized.st_.PutSymbol("a", true);
    g_factorized.st_.PutSymbol("b", true);
    g_factorized.st_.PutSymbol("c", true);

    g_factorized.axiom_ = "S";

    g_factorized.AddProduction("S", {"A", g.st_.EOL_});
    g_factorized.AddProduction("A", {"a", "b"});
    g_factorized.AddProduction("A", {"c"});

    SortProductions(g);
    SortProductions(g_factorized);

    EXPECT_EQ(g.g_.size(), 2);
    EXPECT_EQ(g.g_["S"].size(), 1);
    EXPECT_EQ(g.g_["A"].size(), 2);
    EXPECT_EQ(g.g_, g_factorized.g_);
}

TEST(GrammarTest, LeftFactorize_MultipleCommonPrefixes2) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol("d", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a", "b", "B"});
    g.AddProduction("A", {"a", "b", "c"});
    g.AddProduction("A", {"a", "b", "d"});

    factory.LeftFactorize(g);

    // Expected grammar after left factorization:
    // S -> A EOL
    // A -> a b A'
    // A' -> B
    // A' -> c
    // A' -> d

    Grammar g_factorized;

    g_factorized.st_.PutSymbol("S", false);
    g_factorized.st_.PutSymbol("A", false);
    g_factorized.st_.PutSymbol("A'", false);
    g_factorized.st_.PutSymbol("a", true);
    g_factorized.st_.PutSymbol("b", true);
    g_factorized.st_.PutSymbol("c", true);
    g_factorized.st_.PutSymbol("d", true);

    g_factorized.axiom_ = "S";

    g_factorized.AddProduction("S", {"A", g.st_.EOL_});
    g_factorized.AddProduction("A", {"a", "b", "A'"});
    g_factorized.AddProduction("A'", {"B"});
    g_factorized.AddProduction("A'", {"c"});
    g_factorized.AddProduction("A'", {"d"});

    SortProductions(g);
    SortProductions(g_factorized);

    EXPECT_EQ(g.g_.size(), 3);
    EXPECT_EQ(g.g_["S"].size(), 1);
    EXPECT_EQ(g.g_["A"].size(), 1);
    EXPECT_EQ(g.g_["A'"].size(), 3);
    EXPECT_EQ(g.g_, g_factorized.g_);
}

TEST(GrammarTest, LeftFactorize_NestedCommonPrefixes) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol("d", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a", "b", "B"});
    g.AddProduction("A", {"a", "b", "c"});
    g.AddProduction("A", {"a", "b", "d"});
    g.AddProduction("B", {"a", "b", "c"});
    g.AddProduction("B", {"a", "b", "d"});

    factory.LeftFactorize(g);

    // Expected grammar after left factorization:
    // S -> A EOL
    // A -> a b A'
    // A' -> B
    // A' -> c
    // A' -> d
    // B -> a b B'
    // B' -> c
    // B' -> d

    Grammar g_factorized;

    g_factorized.st_.PutSymbol("S", false);
    g_factorized.st_.PutSymbol("A", false);
    g_factorized.st_.PutSymbol("A'", false);
    g_factorized.st_.PutSymbol("B", false);
    g_factorized.st_.PutSymbol("B'", false);
    g_factorized.st_.PutSymbol("a", true);
    g_factorized.st_.PutSymbol("b", true);
    g_factorized.st_.PutSymbol("c", true);
    g_factorized.st_.PutSymbol("d", true);

    g_factorized.axiom_ = "S";

    g_factorized.AddProduction("S", {"A", g.st_.EOL_});
    g_factorized.AddProduction("A", {"a", "b", "A'"});
    g_factorized.AddProduction("A'", {"B"});
    g_factorized.AddProduction("A'", {"c"});
    g_factorized.AddProduction("A'", {"d"});
    g_factorized.AddProduction("B", {"a", "b", "B'"});
    g_factorized.AddProduction("B'", {"c"});
    g_factorized.AddProduction("B'", {"d"});

    SortProductions(g);
    SortProductions(g_factorized);

    EXPECT_EQ(g.g_.size(), 5);
    EXPECT_EQ(g.g_["S"].size(), 1);
    EXPECT_EQ(g.g_["A"].size(), 1);
    EXPECT_EQ(g.g_["A'"].size(), 3);
    EXPECT_EQ(g.g_["B"].size(), 1);
    EXPECT_EQ(g.g_["B'"].size(), 2);
    EXPECT_EQ(g.g_, g_factorized.g_);
}

TEST(GrammarTest, LeftFactorize_SingleProduction) {
    Grammar        g;
    GrammarFactory factory;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a"});

    factory.LeftFactorize(g);

    // Expected grammar (no changes):
    // S -> A EOL
    // A -> a

    Grammar g_factorized;

    g_factorized.st_.PutSymbol("S", false);
    g_factorized.st_.PutSymbol("A", false);
    g_factorized.st_.PutSymbol("a", true);

    g_factorized.axiom_ = "S";

    g_factorized.AddProduction("S", {"A", g.st_.EOL_});
    g_factorized.AddProduction("A", {"a"});

    SortProductions(g);
    SortProductions(g_factorized);

    EXPECT_EQ(g.g_.size(), 2);
    EXPECT_EQ(g.g_["S"].size(), 1);
    EXPECT_EQ(g.g_["A"].size(), 1);
    EXPECT_EQ(g.g_, g_factorized.g_);
}

TEST(LL1__Test, CreateLL1Table_NoConflict) {
    std::unordered_map<std::string, std::vector<production>> G = {
        {"A", {{"a"}}},
        {"B", {{"b"}}}
    };
    Grammar g(G);
    LL1Parser ll1(g);

    bool ok = ll1.CreateLL1Table();
    EXPECT_TRUE(ok);
}

TEST(LL1__Test, CreateLL1Table_DirectConflict) {
    std::unordered_map<std::string, std::vector<production>> G = {
        {"A", {{"a"}, {"a"}}}
    };
    Grammar g(G);
    LL1Parser ll1(g);

    bool ok = ll1.CreateLL1Table();
    EXPECT_FALSE(ok);
}

TEST(LL1__Test, First_EmptyRuleYieldsEpsilon) {
    Grammar g;
    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    ll1.First({}, result);

    EXPECT_TRUE(result.count(g.st_.EPSILON_));
    EXPECT_EQ(result.size(), 1u);
}

TEST(LL1__Test, First_RuleStartingWithEOLYieldsEpsilon) {
    Grammar g;
    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    ll1.First({{"$"}}, result);

    EXPECT_TRUE(result.count(g.st_.EPSILON_));
    EXPECT_EQ(result.size(), 1u);
}

TEST(LL1__Test, First_LeadingEpsilonSkipsToNext) {
    Grammar g;
    g.st_.PutSymbol("a", true);

    LL1Parser ll1(g);
    std::unordered_set<std::string> result;
    ll1.First({{"EPSILON", "a"}}, result);

    EXPECT_TRUE(result.count("a"));
    EXPECT_FALSE(result.count(g.st_.EPSILON_));
}

TEST(LL1__Test, First_SingleTerminal) {
    Grammar g;
    g.st_.PutSymbol("x", true);

    LL1Parser ll1(g);
    std::unordered_set<std::string> result;
    ll1.First({{"x"}}, result);

    EXPECT_TRUE(result.count("x"));
    EXPECT_EQ(result.size(), 1u);
}

TEST(LL1__Test, First_NonterminalWithEmptyFirstSets) {
    Grammar g;
    g.st_.PutSymbol("Z", false);
    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    ll1.First({{"Z"}}, result);

    EXPECT_TRUE(result.empty());
}

TEST(LL1__Test, CreateLL1Table_UsesFollowForNullable) {
    std::unordered_map<std::string, std::vector<production>> G = {
        {"A", {{"EPSILON"}, {"a"}}}
    };

    Grammar g(G);
    LL1Parser ll1(g);

    bool ok = ll1.CreateLL1Table();
    EXPECT_TRUE(ok);
}


TEST(LL1__Test, ComputeFollowSets_Sequence) {
    Grammar g; 
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);

    g.axiom_ = "S";
    g.AddProduction("S", {"A", "B"});
    g.AddProduction("A", {"a"});
    g.AddProduction("B", {"b"});

    LL1Parser ll1(g);
    ll1.ComputeFollowSets();

    auto followA = ll1.Follow("A");
    EXPECT_EQ(followA, (std::unordered_set<std::string>{"b"}));

    auto followB = ll1.Follow("B");
    EXPECT_EQ(followB, (std::unordered_set<std::string>{g.st_.EOL_}));
}

TEST(LL1__Test, ComputeFollowSets_NullableAtEnd) {
    Grammar g;
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";
    g.AddProduction("S", {"B", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});
    g.AddProduction("B", {"b"});

    LL1Parser ll1(g);
    ll1.ComputeFollowSets();

    EXPECT_EQ(ll1.Follow("A"), (std::unordered_set<std::string>{g.st_.EOL_}));
    EXPECT_EQ(ll1.Follow("B"), (std::unordered_set<std::string>{g.st_.EOL_}));
}

TEST(LL1__Test, ComputeFollowSets_NullableInMiddle) {
    Grammar g;
    std::vector<std::string> syms{"A", "B", "C", "a", "c"};
    for (auto& sym : syms) {
        g.st_.PutSymbol(sym, sym[0]>='a' && sym[0]<='z');
        if (sym=="A"||sym=="B"||sym=="C") g.st_.non_terminals_.insert(sym);
    }
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";
    g.AddProduction("S", {"A","B","C"});
    g.AddProduction("A", {"a"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"c"});

    LL1Parser ll1(g);
    ll1.ComputeFollowSets();

    EXPECT_EQ(ll1.Follow("A"), (std::unordered_set<std::string>{"c"}));
    EXPECT_EQ(ll1.Follow("B"), (std::unordered_set<std::string>{"c"}));
    EXPECT_EQ(ll1.Follow("C"), (std::unordered_set<std::string>{g.st_.EOL_}));
}

TEST(LL1__Test, Follow_UnknownNonTerminalReturnsEmpty) {
    Grammar g;
    LL1Parser ll1(g);
    EXPECT_TRUE(ll1.Follow("X").empty());
}

TEST(LL1__Test, FirstSet) {
    Grammar g;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});

    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {"b"});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{"a", "b"};
    ll1.First({{"A", g.st_.EOL_}}, result);
    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithNullableSymbols) {
    Grammar g;

    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});

    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{"a", g.st_.EPSILON_};
    ll1.First({{"A", g.st_.EOL_}}, result);
    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetMultipleSymbols) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "B", g.st_.EOL_});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{"a", "b", g.st_.EPSILON_};
    ll1.First({{"A", "B"}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetEndingWithNullable) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{"a", g.st_.EPSILON_};
    ll1.First({{"A", g.st_.EOL_}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithAllSymbols) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("D", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol("d", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a", "B", "D"});
    g.AddProduction("A", {"C", "B"});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"d", "B", "c"});
    g.AddProduction("C", {g.st_.EPSILON_});
    g.AddProduction("D", {"a", "B"});
    g.AddProduction("D", {"d"});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{"a", "b", "d"};
    ll1.First({{"A", "B", "D", "C", "D"}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithOneSymbolAndEpsilon) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("D", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol("d", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a", "B", "D"});
    g.AddProduction("A", {"C", "B"});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"d", "B", "c"});
    g.AddProduction("C", {g.st_.EPSILON_});
    g.AddProduction("D", {"a", "B"});
    g.AddProduction("D", {"d"});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{"a", "b", "d", g.st_.EPSILON_};
    ll1.First({{"A"}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithMultipleSymbols2) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("E'", false);
    g.st_.PutSymbol("T", false);
    g.st_.PutSymbol("+", true);
    g.st_.PutSymbol("(", true);
    g.st_.PutSymbol(")", true);
    g.st_.PutSymbol("n", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"T", "E'"});
    g.AddProduction("E'", {"+", "T", "E'"});
    g.AddProduction("E'", {g.st_.EPSILON_});
    g.AddProduction("T", {"(", "E", ")"});
    g.AddProduction("T", {"n"});
    g.AddProduction("T", {g.st_.EPSILON_});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{"(", "n", "+", g.st_.EPSILON_};
    ll1.First({{"T", "E'"}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithTerminalSymbolAtTheBeginning) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("E'", false);
    g.st_.PutSymbol("T", false);
    g.st_.PutSymbol("+", true);
    g.st_.PutSymbol("(", true);
    g.st_.PutSymbol(")", true);
    g.st_.PutSymbol("n", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"T", "E'"});
    g.AddProduction("E'", {"+", "T", "E'"});
    g.AddProduction("E'", {g.st_.EPSILON_});
    g.AddProduction("T", {"(", "E", ")"});
    g.AddProduction("T", {"n"});
    g.AddProduction("T", {g.st_.EPSILON_});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{"+"};
    ll1.First({{"+", "T", "E'"}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithOnlyEpsilon) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("E'", false);
    g.st_.PutSymbol("T", false);
    g.st_.PutSymbol("+", true);
    g.st_.PutSymbol("(", true);
    g.st_.PutSymbol(")", true);
    g.st_.PutSymbol("n", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"T", "E'"});
    g.AddProduction("E'", {"+", "T", "E'"});
    g.AddProduction("E'", {g.st_.EPSILON_});
    g.AddProduction("T", {"(", "E", ")"});
    g.AddProduction("T", {"n"});
    g.AddProduction("T", {g.st_.EPSILON_});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{g.st_.EPSILON_};
    ll1.First({{g.st_.EPSILON_}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithEpsilon2) {
    Grammar g;
    g.st_.PutSymbol("S'", false);
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A'", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("d", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S'";

    g.AddProduction("S'", {"S", g.st_.EOL_});
    g.AddProduction("S", {"A", "B", "C"});
    g.AddProduction("A", {"a", "A", "a"});
    g.AddProduction("A", {"B", "d"});
    g.AddProduction("B", {"b"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {g.st_.EPSILON_});
    g.AddProduction("C", {"c"});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{"b", "c", g.st_.EPSILON_};
    ll1.First({{"B", "C"}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithNestedNullableSymbols) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "B", g.st_.EOL_});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{"a", "b", g.st_.EPSILON_};
    ll1.First({{"A", "B"}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithMultipleNullableSymbols) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "B", g.st_.EOL_});
    g.AddProduction("A", {g.st_.EPSILON_});
    g.AddProduction("B", {g.st_.EPSILON_});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{g.st_.EPSILON_};
    ll1.First({{"A", "B"}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithTerminalAtEnd) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "b", g.st_.EOL_});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{"a", "b"};
    ll1.First({{"A", "b"}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithIndirectLeftRecursion) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"B", "a"});
    g.AddProduction("B", {"A", "b"});
    g.AddProduction("B", {"a"});

    LL1Parser ll1(g);
    ll1.ComputeFirstSets();

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{"a"};
    ll1.First({{"A"}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FirstSetWithComplexNullableSymbols) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "B", "C", g.st_.EOL_});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"c", "C"});
    g.AddProduction("C", {g.st_.EPSILON_});

    LL1Parser ll1(g);
    ll1.ComputeFirstSets();

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{"a", "b", "c", g.st_.EPSILON_};
    ll1.First({{"A", "B", "C"}}, result);

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, AllFirstSets) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("D", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol("d", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a", "B", "D"});
    g.AddProduction("A", {"C", "B"});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"d", "B", "c"});
    g.AddProduction("C", {g.st_.EPSILON_});
    g.AddProduction("D", {"a", "B"});
    g.AddProduction("D", {"d"});

    LL1Parser ll1(g);

    std::unordered_map<std::string, std::unordered_set<std::string>> expected{
        {"S", {"a", "b", "d", g.st_.EPSILON_}},
        {"A", {"a", "b", "d", g.st_.EPSILON_}},
        {"B", {"b", g.st_.EPSILON_}},
        {"C", {"d", g.st_.EPSILON_}},
        {"D", {"a", "d"}}};

    EXPECT_EQ(ll1.first_sets_, expected);
}

TEST(LL1__Test, FollowSet2) {
    Grammar g;
    g.st_.PutSymbol("S'", false);
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A'", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("d", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S'";

    g.AddProduction("S'", {"S", g.st_.EOL_});
    g.AddProduction("S", {"A", "B", "C"});
    g.AddProduction("A", {"a", "A", "a"});
    g.AddProduction("A", {"B", "d"});
    g.AddProduction("B", {"b"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {g.st_.EPSILON_});
    g.AddProduction("C", {"c"});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{"b", "c", "a", g.st_.EOL_};
    result = ll1.Follow("A");

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FollowTest1) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("E'", false);
    g.st_.PutSymbol("T", false);
    g.st_.PutSymbol("+", true);
    g.st_.PutSymbol("(", true);
    g.st_.PutSymbol(")", true);
    g.st_.PutSymbol("n", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"T", "E'"});
    g.AddProduction("E'", {"+", "T", "E'"});
    g.AddProduction("E'", {g.st_.EPSILON_});
    g.AddProduction("T", {"(", "E", ")"});
    g.AddProduction("T", {"n"});
    g.AddProduction("T", {g.st_.EPSILON_});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{g.st_.EOL_, ")"};
    result = ll1.Follow("E");

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FollowTest2) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("E'", false);
    g.st_.PutSymbol("T", false);
    g.st_.PutSymbol("+", true);
    g.st_.PutSymbol("(", true);
    g.st_.PutSymbol(")", true);
    g.st_.PutSymbol("n", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"T", "E'"});
    g.AddProduction("E'", {"+", "T", "E'"});
    g.AddProduction("E'", {g.st_.EPSILON_});
    g.AddProduction("T", {"(", "E", ")"});
    g.AddProduction("T", {"n"});
    g.AddProduction("T", {g.st_.EPSILON_});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{g.st_.EOL_, ")"};
    result = ll1.Follow("E'");

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FollowSetWithNestedProductions) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "B", g.st_.EOL_});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{"b", g.st_.EOL_};
    result = ll1.Follow("A");

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FollowSetWithMultipleOccurrences) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "B", g.st_.EOL_});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});

    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    std::unordered_set<std::string> expected{"b", g.st_.EOL_};
    result = ll1.Follow("A");

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FollowSetWithIndirectLeftRecursion) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"B", "a"});
    g.AddProduction("B", {"A", "b"});
    g.AddProduction("B", {"a"});

    LL1Parser ll1(g);
    ll1.ComputeFirstSets();
    ll1.ComputeFollowSets();

    std::unordered_set<std::string> result = ll1.Follow("A");
    std::unordered_set<std::string> expected{"b", g.st_.EOL_};

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, FollowSetWithMultipleNullableSymbols) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "B", "C", g.st_.EOL_});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"c", "C"});
    g.AddProduction("C", {g.st_.EPSILON_});

    LL1Parser ll1(g);
    ll1.ComputeFirstSets();
    ll1.ComputeFollowSets();

    std::unordered_set<std::string> result = ll1.Follow("B");
    std::unordered_set<std::string> expected{"c", g.st_.EOL_};

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, AllFollowSets) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("E'", false);
    g.st_.PutSymbol("T", false);
    g.st_.PutSymbol("+", true);
    g.st_.PutSymbol("(", true);
    g.st_.PutSymbol(")", true);
    g.st_.PutSymbol("n", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"T", "E'"});
    g.AddProduction("E'", {"+", "T", "E'"});
    g.AddProduction("E'", {g.st_.EPSILON_});
    g.AddProduction("T", {"(", "E", ")"});
    g.AddProduction("T", {"n"});
    g.AddProduction("T", {g.st_.EPSILON_});

    LL1Parser ll1(g);

    std::unordered_map<std::string, std::unordered_set<std::string>> result;
    std::unordered_map<std::string, std::unordered_set<std::string>> expected{
        {"S", {g.st_.EOL_}},
        {"E", {")", g.st_.EOL_}},
        {"E'", {")", g.st_.EOL_}},
        {"T", {"+", ")", g.st_.EOL_}}};

    for (const std::string& nt : g.st_.non_terminals_) {
        result[nt] = ll1.Follow(nt);
    }

    EXPECT_EQ(result, expected);
}

TEST(LL1__Test, AllFollowSets2) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("D", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol("d", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", g.st_.EOL_});
    g.AddProduction("A", {"a", "B", "D"});
    g.AddProduction("A", {"C", "B"});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"d", "B", "c"});
    g.AddProduction("C", {g.st_.EPSILON_});
    g.AddProduction("D", {"a", "B"});
    g.AddProduction("D", {"d"});

    LL1Parser ll1(g);

    std::unordered_map<std::string, std::unordered_set<std::string>> result;
    std::unordered_map<std::string, std::unordered_set<std::string>> expected{
        {"S", {g.st_.EOL_}},
        {"A", {g.st_.EOL_}},
        {"B", {"a", "d", "c", g.st_.EOL_}},
        {"C", {"b", g.st_.EOL_}},
        {"D", {g.st_.EOL_}}};

    for (const std::string& nt : g.st_.non_terminals_) {
        result[nt] = ll1.Follow(nt);
    }

    EXPECT_EQ(result, expected);
}

TEST(SLR1_ClosureTest, BasicClosure) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("T", false);
    g.st_.PutSymbol("+", true);
    g.st_.PutSymbol("n", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"E", "+", "T"});
    g.AddProduction("E", {"T"});
    g.AddProduction("T", {"n"});

    SLR1Parser slr1(g);

    // Initial item: S -> •E EOL
    std::unordered_set<Lr0Item> items = {
        {"S", {"E", g.st_.EOL_}, g.st_.EPSILON_, g.st_.EOL_}};

    slr1.Closure(items);

    // Expected items after closure
    std::unordered_set<Lr0Item> expected = {
        {"S", {"E", g.st_.EOL_}, g.st_.EPSILON_, g.st_.EOL_},
        {"E", {"E", "+", "T"}, g.st_.EPSILON_, g.st_.EOL_},
        {"E", {"T"}, g.st_.EPSILON_, g.st_.EOL_},
        {"T", {"n"}, g.st_.EPSILON_, g.st_.EOL_}};

    EXPECT_EQ(items, expected);
}

TEST(SLR1_ClosureTest, ComplexGrammarWithNullable) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("D", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"D", g.st_.EOL_});
    g.AddProduction("D", {"A", "B", "C"});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"c", "C"});
    g.AddProduction("C", {g.st_.EPSILON_});

    SLR1Parser slr1(g);

    std::unordered_set<Lr0Item> items = {
        {"S", {"D", g.st_.EOL_}, g.st_.EPSILON_, g.st_.EOL_}};

    slr1.Closure(items);

    std::unordered_set<Lr0Item> expected = {
        {"S", {"D", g.st_.EOL_}, g.st_.EPSILON_, g.st_.EOL_},
        {"D", {"A", "B", "C"}, g.st_.EPSILON_, g.st_.EOL_},
        {"A", {"a", "A"}, g.st_.EPSILON_, g.st_.EOL_},
        {"A", {g.st_.EPSILON_}, g.st_.EPSILON_, g.st_.EOL_}};

    EXPECT_EQ(items, expected);
}

TEST(SLR1_ClosureTest, CompleteItemClosure) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("D", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"D", g.st_.EOL_});
    g.AddProduction("D", {"A", "B", "C"});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"c", "C"});
    g.AddProduction("C", {g.st_.EPSILON_});

    SLR1Parser slr1(g);

    std::unordered_set<Lr0Item> items = {
        {"A", {g.st_.EPSILON_}, g.st_.EPSILON_, g.st_.EOL_}};

    slr1.Closure(items);

    std::unordered_set<Lr0Item> expected = {
        {"A", {g.st_.EPSILON_}, 1, g.st_.EPSILON_, g.st_.EOL_}    
    };
    
    EXPECT_EQ(items, expected);
}


TEST(SLR1_ClosureTest, NoAdditionalClosureNeeded) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"a", g.st_.EOL_});

    SLR1Parser slr1(g);

    std::unordered_set<Lr0Item> items = {
        {"S", {"a", g.st_.EOL_}, g.st_.EPSILON_, g.st_.EOL_}};

    slr1.Closure(items);

    std::unordered_set<Lr0Item> expected = {
        {"S", {"a", g.st_.EOL_}, g.st_.EPSILON_, g.st_.EOL_}};

    EXPECT_EQ(items, expected);
}

TEST(SLR1_ClosureTest, DotInMiddleOfProduction) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("T", false);
    g.st_.PutSymbol("+", true);
    g.st_.PutSymbol("n", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"E", "+", "T"});
    g.AddProduction("E", {"T"});
    g.AddProduction("T", {"n"});

    SLR1Parser slr1(g);

    // Initial item: E -> E • + T
    std::unordered_set<Lr0Item> items = {
        {"E", {"E", "+", "T"}, 1, g.st_.EPSILON_, g.st_.EOL_}};

    slr1.Closure(items);

    // Should not add any new items since the dot is before a terminal
    std::unordered_set<Lr0Item> expected = {
        {"E", {"E", "+", "T"}, 1, g.st_.EPSILON_, g.st_.EOL_}};

    EXPECT_EQ(items, expected);
}

TEST(SLR1_ClosureTest, DotBeforeNonTerminal) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "B", g.st_.EOL_});
    g.AddProduction("A", {"a"});
    g.AddProduction("B", {"b"});

    SLR1Parser slr1(g);

    // Initial item: S -> A • B EOL
    std::unordered_set<Lr0Item> items = {
        {"S", {"A", "B", g.st_.EOL_}, 1, g.st_.EPSILON_, g.st_.EOL_}};

    slr1.Closure(items);

    // Should add productions for B since dot is before a non-terminal
    std::unordered_set<Lr0Item> expected = {
        {"S", {"A", "B", g.st_.EOL_}, 1, g.st_.EPSILON_, g.st_.EOL_},
        {"B", {"b"}, 0, g.st_.EPSILON_, g.st_.EOL_}};

    EXPECT_EQ(items, expected);
}

TEST(SLR1_ClosureTest, MultipleItemsWithDifferentDotPositions) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("T", false);
    g.st_.PutSymbol("F", false);
    g.st_.PutSymbol("+", true);
    g.st_.PutSymbol("*", true);
    g.st_.PutSymbol("n", true);
    g.st_.PutSymbol("(", true);
    g.st_.PutSymbol(")", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"E", "+", "T"});
    g.AddProduction("E", {"T"});
    g.AddProduction("T", {"T", "*", "F"});
    g.AddProduction("T", {"F"});
    g.AddProduction("F", {"(", "E", ")"});
    g.AddProduction("F", {"n"});

    SLR1Parser slr1(g);

    // Initial items with dots in different positions
    std::unordered_set<Lr0Item> items = {
        {"E", {"E", "+", "T"}, 1, g.st_.EPSILON_, g.st_.EOL_}, // E -> E • + T
        {"T", {"T", "*", "F"}, 0, g.st_.EPSILON_, g.st_.EOL_}, // T -> • T * F
        {"F", {"(", "E", ")"}, 2, g.st_.EPSILON_, g.st_.EOL_}  // F -> ( E • )
    };

    slr1.Closure(items);

    // Expected items after closure
    std::unordered_set<Lr0Item> expected = {
        {"E", {"E", "+", "T"}, 1, g.st_.EPSILON_, g.st_.EOL_},
        {"T", {"T", "*", "F"}, 0, g.st_.EPSILON_, g.st_.EOL_},
        {"F", {"(", "E", ")"}, 2, g.st_.EPSILON_, g.st_.EOL_},
        // From T -> • T * F
        {"T", {"F"}, 0, g.st_.EPSILON_, g.st_.EOL_},
        {"F", {"(", "E", ")"}, 0, g.st_.EPSILON_, g.st_.EOL_},
        {"F", {"n"}, 0, g.st_.EPSILON_, g.st_.EOL_},
        // From F -> ( E • )
    };

    EXPECT_EQ(items, expected);
}

TEST(SLR1_ClosureTest, DotAtEndOfProduction) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "B", g.st_.EOL_});
    g.AddProduction("A", {"a"});
    g.AddProduction("B", {"b"});

    SLR1Parser slr1(g);

    // Initial item: A -> a •
    std::unordered_set<Lr0Item> items = {
        {"A", {"a"}, 1, g.st_.EPSILON_, g.st_.EOL_}};

    slr1.Closure(items);

    // Should remain unchanged since dot is at the end
    std::unordered_set<Lr0Item> expected = {
        {"A", {"a"}, 1, g.st_.EPSILON_, g.st_.EOL_}};

    EXPECT_EQ(items, expected);
}

TEST(SLR1_ClosureTest, MixedDotPositionsWithNullableSymbols) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "B", "C", g.st_.EOL_});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"c", "C"});
    g.AddProduction("C", {g.st_.EPSILON_});

    SLR1Parser slr1(g);

    // Initial items with dots in different positions
    std::unordered_set<Lr0Item> items = {
        {"S",
         {"A", "B", "C", g.st_.EOL_},
         1,
         g.st_.EPSILON_,
         g.st_.EOL_},                                     // S -> A • B C EOL
        {"A", {"a", "A"}, 1, g.st_.EPSILON_, g.st_.EOL_}, // A -> a • A
        {"B", {"b", "B"}, 0, g.st_.EPSILON_, g.st_.EOL_}  // B -> • b B
    };

    slr1.Closure(items);

    // Expected items after closure
    std::unordered_set<Lr0Item> expected = {
        {"S", {"A", "B", "C", g.st_.EOL_}, 1, g.st_.EPSILON_, g.st_.EOL_},
        {"A", {"a", "A"}, 1, g.st_.EPSILON_, g.st_.EOL_},
        {"B", {"b", "B"}, 0, g.st_.EPSILON_, g.st_.EOL_},
        // From S -> A • B C EOL
        {"B", {"b", "B"}, 0, g.st_.EPSILON_, g.st_.EOL_},
        {"B", {g.st_.EPSILON_}, 1, "C", g.st_.EOL_},
        // From A -> a • A
        {"A", {"a", "A"}, 0, g.st_.EPSILON_, g.st_.EOL_},
        {"A", {g.st_.EPSILON_}, 1, g.st_.EPSILON_, g.st_.EOL_},
        // From B -> • b B
        {"B", {"b", "B"}, 0, g.st_.EPSILON_, g.st_.EOL_},
        {"B", {g.st_.EPSILON_}, 1, g.st_.EPSILON_, g.st_.EOL_}};

    EXPECT_EQ(items, expected);
}

TEST(SLR1_Delta, DeltaFunctionToBasicItemSet) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "B", g.st_.EOL_});
    g.AddProduction("A", {"a"});
    g.AddProduction("B", {"b"});

    SLR1Parser slr1(g);

    std::unordered_set<Lr0Item> items = {
        {"S", {"A", "B", g.st_.EOL_}, 0, g.st_.EPSILON_, g.st_.EOL_}};

    auto result = slr1.Delta(items, "A");
    
    std::unordered_set<Lr0Item> expected = {
        {"S", {"A", "B", g.st_.EOL_}, 1, g.st_.EPSILON_, g.st_.EOL_},
        {"B", {"b"}, g.st_.EPSILON_, g.st_.EOL_}};
    
    EXPECT_EQ(result, expected);
}

TEST(SLR1_Delta, DeltaFunctionWhenSymbolDoesNotExists) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "B", g.st_.EOL_});
    g.AddProduction("A", {"a"});
    g.AddProduction("B", {"b"});

    SLR1Parser slr1(g);

    std::unordered_set<Lr0Item> items = {
        {"S", {"A", "B", g.st_.EOL_}, 0, g.st_.EPSILON_, g.st_.EOL_}};

    auto result = slr1.Delta(items, "Z");
    
    std::unordered_set<Lr0Item> expected = {};
    
    EXPECT_EQ(result, expected);
}

TEST(SLR1_Delta, DeltaFunctionWhenSymbolDoesNotExistsAndIsEpsilon) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "B", g.st_.EOL_});
    g.AddProduction("A", {"a"});
    g.AddProduction("B", {"b"});

    SLR1Parser slr1(g);

    std::unordered_set<Lr0Item> items = {
        {"S", {"A", "B", g.st_.EOL_}, 0, g.st_.EPSILON_, g.st_.EOL_}};

    auto result = slr1.Delta(items, g.st_.EPSILON_);
    
    std::unordered_set<Lr0Item> expected = {};
    
    EXPECT_EQ(result, expected);
}

TEST(SLR1_Delta, DeltaFunctionWhenItemIsComplete) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "B", "C", g.st_.EOL_});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"c", "C"});
    g.AddProduction("C", {g.st_.EPSILON_});

    SLR1Parser slr1(g);

    std::unordered_set<Lr0Item> items = {
        {"A", {"a", "A"}, 2, g.st_.EPSILON_, g.st_.EOL_}};

    auto result = slr1.Delta(items, "A");
    
    std::unordered_set<Lr0Item> expected = {};
    
    EXPECT_EQ(result, expected);
}

TEST(SLR1_Delta, DeltaFunctionWhenItemAmdSymbolAreEpsilon) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("C", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol("c", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";

    g.AddProduction("S", {"A", "B", "C", g.st_.EOL_});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});
    g.AddProduction("C", {"c", "C"});
    g.AddProduction("C", {g.st_.EPSILON_});

    SLR1Parser slr1(g);

    std::unordered_set<Lr0Item> items = {
        {"A", {g.st_.EPSILON_}, g.st_.EPSILON_, g.st_.EOL_}};

    auto result = slr1.Delta(items, g.st_.EPSILON_);
    
    std::unordered_set<Lr0Item> expected = {};
    
    EXPECT_EQ(result, expected);
}

TEST(SLR1_MakeInitialState, BasicGrammar) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";
    g.AddProduction("S", {"a", g.st_.EOL_});

    SLR1Parser slr1(g);
    slr1.MakeInitialState();

    const auto& states = slr1.states_;
    ASSERT_EQ(states.size(), 1);

    const auto& initial_state = *states.begin();
    EXPECT_EQ(initial_state.id_, 0);

    std::unordered_set<Lr0Item> expected_items = {
        {"S", {"a", g.st_.EOL_}, 0, g.st_.EPSILON_, g.st_.EOL_}};

    EXPECT_EQ(initial_state.items_, expected_items);
}

TEST(SLR1_MakeInitialState, MultipleAxiomProductions) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";
    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"a"});
    g.AddProduction("E", {"b"});

    SLR1Parser slr1(g);
    slr1.MakeInitialState();

    const auto& states = slr1.states_;
    ASSERT_EQ(states.size(), 1);

    const auto& initial_state = *states.begin();

    std::unordered_set<Lr0Item> expected_items = {
        {"S", {"E", g.st_.EOL_}, 0, g.st_.EPSILON_, g.st_.EOL_},
        {"E", {"a"}, 0, g.st_.EOL_, g.st_.EOL_},
        {"E", {"b"}, 0, g.st_.EOL_, g.st_.EOL_}};

    EXPECT_EQ(initial_state.items_, expected_items);
}

TEST(SLR1_MakeInitialState, ComplexGrammarWithNullable) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("B", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol("b", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";
    g.AddProduction("S", {"A", "B", g.st_.EOL_});
    g.AddProduction("A", {"a", "A"});
    g.AddProduction("A", {g.st_.EPSILON_});
    g.AddProduction("B", {"b", "B"});
    g.AddProduction("B", {g.st_.EPSILON_});

    SLR1Parser slr1(g);
    slr1.MakeInitialState();

    const auto& states = slr1.states_;
    ASSERT_EQ(states.size(), 1);

    const auto& initial_state = *states.begin();

    std::unordered_set<Lr0Item> expected_items = {
        {"S", {"A", "B", g.st_.EOL_}, 0, g.st_.EPSILON_, g.st_.EOL_},
        {"A", {"a", "A"}, 0, "B", g.st_.EOL_},
        {"A", {g.st_.EPSILON_}, 1, "B", g.st_.EOL_}};

    EXPECT_EQ(initial_state.items_, expected_items);
}

TEST(SLR1_MakeInitialState, RecursiveGrammar) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("T", false);
    g.st_.PutSymbol("+", true);
    g.st_.PutSymbol("*", true);
    g.st_.PutSymbol("n", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";
    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"E", "+", "T"});
    g.AddProduction("E", {"T"});
    g.AddProduction("T", {"T", "*", "n"});
    g.AddProduction("T", {"n"});

    SLR1Parser slr1(g);
    slr1.MakeInitialState();

    const auto& states = slr1.states_;
    ASSERT_EQ(states.size(), 1);

    const auto& initial_state = *states.begin();

    std::unordered_set<Lr0Item> expected_items = {
        {"S", {"E", g.st_.EOL_}, 0, g.st_.EPSILON_, g.st_.EOL_},
        {"E", {"E", "+", "T"}, 0, g.st_.EOL_, g.st_.EOL_},
        {"E", {"T"}, 0, g.st_.EOL_, g.st_.EOL_},
        {"T", {"T", "*", "n"}, 0, "+", g.st_.EOL_},
        {"T", {"n"}, 0, "+", g.st_.EOL_}};

    EXPECT_EQ(initial_state.items_, expected_items);
}

TEST(SLR1_MakeInitialState, StateIdAndUniqueness) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";
    g.AddProduction("S", {"a", g.st_.EOL_});

    SLR1Parser slr1(g);
    slr1.MakeInitialState();

    // Calling again should not create duplicate states
    slr1.MakeInitialState();

    const auto& states = slr1.states_;
    ASSERT_EQ(states.size(), 1);

    const auto& initial_state = *states.begin();
    EXPECT_EQ(initial_state.id_, 0);

    // Verify the state is properly hashed and compared
    state duplicate;
    duplicate.id_ = 0;
    duplicate.items_.emplace("S", std::vector<std::string>{"a", g.st_.EOL_}, 0,
                             g.st_.EPSILON_, g.st_.EOL_);

    EXPECT_TRUE(states.contains(duplicate));
}

TEST(SLR1_SolveLRConflicts, AcceptAction) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";
    g.AddProduction("S", {"a", g.st_.EOL_});

    SLR1Parser slr1(g);

    state st;
    st.id_ = 0;
    st.items_.emplace("S", std::vector<std::string>{"a", g.st_.EOL_}, 2,
                      g.st_.EPSILON_, g.st_.EOL_); // Complete item

    EXPECT_TRUE(slr1.SolveLRConflicts(st));
    EXPECT_EQ(slr1.actions_[0][g.st_.EOL_].action, SLR1Parser::Action::Accept);
}

TEST(SLR1_SolveLRConflicts, BasicReduce) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";
    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"a"});

    SLR1Parser slr1(g);
    slr1.ComputeFirstSets();
    slr1.ComputeFollowSets();
    state st;
    st.id_ = 0;
    st.items_.emplace("E", std::vector<std::string>{"a"}, 1, g.st_.EPSILON_,
                      g.st_.EOL_); // Complete item

    EXPECT_TRUE(slr1.SolveLRConflicts(st));
    EXPECT_EQ(slr1.actions_[0][g.st_.EOL_].action, SLR1Parser::Action::Reduce);
}

TEST(SLR1_SolveLRConflicts, BasicShift) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";
    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"a"});

    SLR1Parser slr1(g);

    state st;
    st.id_ = 0;
    st.items_.emplace("E", std::vector<std::string>{"a"}, 0, g.st_.EPSILON_,
                      g.st_.EOL_); // Dot before terminal

    EXPECT_TRUE(slr1.SolveLRConflicts(st));
    EXPECT_EQ(slr1.actions_[0]["a"].action, SLR1Parser::Action::Shift);
}

TEST(SLR1_SolveLRConflicts, SolveShiftReduceConflict) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";
    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"a"});
    g.AddProduction("E", {"a", "E"});

    SLR1Parser slr1(g);
    slr1.ComputeFirstSets();
    slr1.ComputeFollowSets();

    state st;
    st.id_ = 0;
    st.items_.emplace("E", std::vector<std::string>{"a"}, 1, g.st_.EPSILON_,
                      g.st_.EOL_);
    st.items_.emplace("E", std::vector<std::string>{"a", "E"}, 0,
                      g.st_.EPSILON_, g.st_.EOL_);

    EXPECT_TRUE(slr1.SolveLRConflicts(st)); // Should NOT detect conflict
}

TEST(SLR1_SolveLRConflicts, ShiftReduceConflict) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("E", false);
    g.st_.PutSymbol("a", true);
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";
    g.AddProduction("S", {"E", g.st_.EOL_});
    g.AddProduction("E", {"a"});
    g.AddProduction("E", {"a", "E"});
    g.AddProduction("E", {"E", "a"});

    SLR1Parser slr1(g);
    slr1.ComputeFirstSets();
    slr1.ComputeFollowSets();

    state st;
    st.id_ = 0;

    st.items_.emplace("E", std::vector<std::string>{"a"}, 1, g.st_.EPSILON_,
                      g.st_.EOL_);
    st.items_.emplace("E", std::vector<std::string>{"a", "E"}, 1,
                      g.st_.EPSILON_, g.st_.EOL_);
    st.items_.emplace("E", std::vector<std::string>{"a"}, 0, g.st_.EPSILON_,
                      g.st_.EOL_);
    st.items_.emplace("E", std::vector<std::string>{"a", "E"}, 0,
                      g.st_.EPSILON_, g.st_.EOL_);
    st.items_.emplace("E", std::vector<std::string>{"E", "a"}, 0,
                      g.st_.EPSILON_, g.st_.EOL_);

    state st1;
    st.id_ = 1;
    st.items_.emplace("E", std::vector<std::string>{"a", "E"}, 2,
                      g.st_.EPSILON_, g.st_.EOL_);
    st.items_.emplace("E", std::vector<std::string>{"E", "a"}, 1,
                      g.st_.EPSILON_, g.st_.EOL_);

    EXPECT_FALSE(slr1.SolveLRConflicts(st));
    EXPECT_TRUE(slr1.SolveLRConflicts(st1));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
