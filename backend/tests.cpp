/*
 * SyntaxTutor - Interactive Tutorial About Syntax Analyzers
 * Copyright (C) 2025 Jose R. (jose-rzm)
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "grammar.hpp"
#include "grammar_factory.hpp"
#include "ll1_parser.hpp"
#include "slr1_parser.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <ranges>
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

TEST(GrammarTest, SimpleGrammar) {
    std::unordered_map<std::string, std::vector<production>> g(
        {{"A", {{"a"}}}});

    Grammar gr(g);

    std::unordered_map<std::string, std::vector<production>> expected;
    expected["S"] = {{"A", "$"}};
    expected["A"] = {{"a"}};

    ASSERT_EQ(gr.axiom_, "S");
    ASSERT_TRUE(gr.g_.contains("S"));
    ASSERT_EQ(gr.g_, expected);
    ASSERT_TRUE(gr.st_.terminals_.contains("a"));
    ASSERT_TRUE(gr.st_.non_terminals_.contains("A"));
    ASSERT_TRUE(gr.st_.terminals_.contains("$"));
}

TEST(GrammarTest, GrammarWithEpsilon) {
    std::unordered_map<std::string, std::vector<production>> g(
        {{"A", {{"EPSILON"}}}});

    Grammar           gr(g);
    const std::string eps = gr.st_.EPSILON_;

    std::unordered_map<std::string, std::vector<production>> expected;
    expected["S"] = {{"A", "$"}};
    expected["A"] = {{eps}};

    ASSERT_EQ(gr.axiom_, "S");
    ASSERT_EQ(gr.g_, expected);
    ASSERT_TRUE(gr.st_.terminals_.contains(eps));
    ASSERT_TRUE(gr.st_.non_terminals_.contains("A"));
}

TEST(GrammarTest, MultipleProductionsMixedSymbols) {
    std::unordered_map<std::string, std::vector<production>> g(
        {{"A", {{"a", "B"}, {"b"}}}, {"B", {{"c"}}}});

    Grammar gr(g);

    std::unordered_map<std::string, std::vector<production>> expected;
    expected["S"] = {{"A", "$"}};
    expected["A"] = {{"a", "B"}, {"b"}};
    expected["B"] = {{"c"}};

    ASSERT_EQ(gr.axiom_, "S");
    ASSERT_TRUE(gr.g_.contains("S"));
    ASSERT_EQ(gr.g_, expected);
    ASSERT_TRUE(gr.st_.terminals_.contains("a"));
    ASSERT_TRUE(gr.st_.terminals_.contains("b"));
    ASSERT_TRUE(gr.st_.terminals_.contains("c"));
    ASSERT_TRUE(gr.st_.terminals_.contains("$"));
    ASSERT_TRUE(gr.st_.non_terminals_.contains("A"));
    ASSERT_TRUE(gr.st_.non_terminals_.contains("B"));
}

TEST(GrammarTest, RecursiveArithmeticGrammar) {
    std::unordered_map<std::string, std::vector<production>> g(
        {{"A", {{"A", "p", "T"}, {"T"}}},
         {"T", {{"T", "m", "F"}, {"F"}}},
         {"F", {{"a", "A", "c"}, {"i"}}}});

    Grammar gr(g);

    std::unordered_map<std::string, std::vector<production>> expected;
    expected["S"] = {{"A", "$"}};
    expected["A"] = {{"A", "p", "T"}, {"T"}};
    expected["T"] = {{"T", "m", "F"}, {"F"}};
    expected["F"] = {{"a", "A", "c"}, {"i"}};

    ASSERT_EQ(gr.axiom_, "S");
    ASSERT_TRUE(gr.g_.contains("S"));
    ASSERT_EQ(gr.g_, expected);
    ASSERT_TRUE(gr.st_.terminals_.contains("p"));
    ASSERT_TRUE(gr.st_.terminals_.contains("m"));
    ASSERT_TRUE(gr.st_.terminals_.contains("a"));
    ASSERT_TRUE(gr.st_.terminals_.contains("c"));
    ASSERT_TRUE(gr.st_.terminals_.contains("i"));
    ASSERT_TRUE(gr.st_.terminals_.contains("$"));
    ASSERT_TRUE(gr.st_.non_terminals_.contains("A"));
    ASSERT_TRUE(gr.st_.non_terminals_.contains("T"));
    ASSERT_TRUE(gr.st_.non_terminals_.contains("F"));
}

TEST(GrammarTest, ComplexGrammarWithEpsilonAndRecursion) {
    std::unordered_map<std::string, std::vector<production>> g;
    {
        g["A"] = {{"a"}, {"EPSILON"}};
        g["B"] = {{"b", "B"}, {"b"}};
        g["C"] = {{"c"}};
    }

    Grammar           gr(g);
    const std::string eps = gr.st_.EPSILON_;

    std::unordered_map<std::string, std::vector<production>> expected;
    expected["S"] = {{"A", "$"}};
    expected["A"] = {{"a"}, {eps}};
    expected["B"] = {{"b", "B"}, {"b"}};
    expected["C"] = {{"c"}};

    ASSERT_EQ(gr.axiom_, "S");
    ASSERT_TRUE(gr.g_.contains("S"));
    ASSERT_EQ(gr.g_, expected);
    ASSERT_TRUE(gr.st_.terminals_.contains("a"));
    ASSERT_TRUE(gr.st_.terminals_.contains("b"));
    ASSERT_TRUE(gr.st_.terminals_.contains("c"));
    ASSERT_TRUE(gr.st_.terminals_.contains(eps));
    ASSERT_TRUE(gr.st_.terminals_.contains("$"));
    ASSERT_TRUE(gr.st_.non_terminals_.contains("A"));
    ASSERT_TRUE(gr.st_.non_terminals_.contains("B"));
    ASSERT_TRUE(gr.st_.non_terminals_.contains("C"));
}

TEST(GrammarTest, AxiomIsInsertedAtConstruction) {
    using production = std::vector<std::string>;
    std::unordered_map<std::string, std::vector<production>> init{
        {"A", {{"a"}, {"EPSILON"}}}};

    Grammar gr(init);

    EXPECT_EQ(gr.axiom_, "S");

    ASSERT_TRUE(gr.g_.count("S"));
    auto prods = gr.g_.at("S");
    ASSERT_EQ(prods.size(), 1u);
    EXPECT_EQ(prods[0].size(), 2u);
    EXPECT_EQ(prods[0][0], "A");
    EXPECT_EQ(prods[0][1], "$");

    EXPECT_TRUE(gr.st_.In("S"));
    EXPECT_FALSE(gr.st_.IsTerminal("S"));
}

TEST(GrammarSplitTest, SplitEpsilonReturnsSingleton) {
    std::unordered_map<std::string, std::vector<production>> g(
        {{"A", {{"a"}}}});
    Grammar           gr(g);
    const std::string eps = gr.st_.EPSILON_;

    auto result = gr.Split(eps);
    ASSERT_EQ(result.size(), 1u);
    ASSERT_EQ(result[0], eps);
}

TEST(GrammarSplitTest, SplitSingleCharSymbols) {
    std::unordered_map<std::string, std::vector<production>> g(
        {{"A", {{"a", "B"}}}, {"B", {{"b"}}}});
    Grammar gr(g);

    auto result = gr.Split("ab");
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "b");
}

TEST(GrammarSplitTest, SplitLongestMatchPrefersLongerSymbol) {
    std::unordered_map<std::string, std::vector<production>> g(
        {{"A", {{"ab"}}}});
    Grammar gr(g);

    auto result = gr.Split("ab");
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], "ab");
}

TEST(GrammarSplitTest, SplitInvalidSubstringReturnsEmpty) {
    std::unordered_map<std::string, std::vector<production>> g(
        {{"A", {{"a"}}}, {"B", {{"b"}}}});
    Grammar gr(g);

    auto result = gr.Split("ac");
    EXPECT_TRUE(result.empty());
}

TEST(GrammarSplitBranchTest, PrefersLongestAmongMultipleSymbols) {
    std::unordered_map<std::string, std::vector<production>> rules{
        {"A", {{"a"}}}, {"B", {{"ab"}}}, {"C", {{"aba"}}}};
    Grammar gr(rules);

    auto result = gr.Split("aba");
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], "aba");
}

TEST(GrammarSplitBranchTest, SplitMixSingleAndDouble) {
    std::unordered_map<std::string, std::vector<production>> rules{
        {"X", {{"x"}}}, {"Y", {{"yz"}}}};
    Grammar gr(rules);

    auto result = gr.Split("xyz");
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0], "x");
    EXPECT_EQ(result[1], "yz");
}

TEST(GrammarSplitBranchTest, SplitTrailingInvalidReturnsEmpty) {
    std::unordered_map<std::string, std::vector<production>> rules{
        {"A", {{"a"}}}, {"B", {{"b"}}}};
    Grammar gr(rules);

    auto result = gr.Split("abX");
    EXPECT_TRUE(result.empty());
}

TEST(GrammarSplitBranchTest, SplitRepeatingLongestSymbols) {
    std::unordered_map<std::string, std::vector<production>> rules{
        {"T", {{"t"}}}, {"U", {{"tu"}}}, {"V", {{"tuv"}}}};
    Grammar gr(rules);

    auto result = gr.Split("tuvtuv");
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0], "tuv");
    EXPECT_EQ(result[1], "tuv");
}

TEST(GrammarMiscTest, SetAxiomChangesAxiom) {
    std::unordered_map<std::string, std::vector<production>> g(
        {{"A", {{"a"}}}});
    Grammar gr(g);

    ASSERT_EQ(gr.axiom_, "S");
    gr.SetAxiom("X");
    EXPECT_EQ(gr.axiom_, "X");
}

TEST(GrammarMiscTest, HasEmptyProductionDetectsEpsilon) {
    std::unordered_map<std::string, std::vector<production>> g_initial(
        {{"A", {{"a"}}}, {"B", {{"b"}}}});
    Grammar           temp(g_initial);
    const std::string eps = temp.st_.EPSILON_;

    std::unordered_map<std::string, std::vector<production>> g(
        {{"A", {{eps}, {"x"}}}, {"B", {{"b"}}}});
    Grammar gr(g);

    EXPECT_TRUE(gr.HasEmptyProduction("A"));
    EXPECT_FALSE(gr.HasEmptyProduction("B"));
}

TEST(GrammarMiscTest, FilterRulesByConsequentFindsAllOccurrences) {
    std::unordered_map<std::string, std::vector<production>> g(
        {{"A", {{"a", "B"}}}, {"B", {{"b"}}}, {"C", {{"B", "c"}}}});
    Grammar gr(g);

    auto rules = gr.FilterRulesByConsequent("B");

    ASSERT_EQ(rules.size(), 2u);

    bool foundA = false, foundC = false;
    for (auto& p : rules) {
        const std::string& lhs  = p.first;
        const production&  prod = p.second;
        if (lhs == "A" && prod.size() == 2 && prod[0] == "a" &&
            prod[1] == "B") {
            foundA = true;
        }
        if (lhs == "C" && prod.size() == 2 && prod[0] == "B" &&
            prod[1] == "c") {
            foundC = true;
        }
    }
    EXPECT_TRUE(foundA);
    EXPECT_TRUE(foundC);
}

TEST(GrammarFactoryInitTest, CreatesNineBaseItems) {
    GrammarFactory factory;
    factory.Init();
    ASSERT_EQ(factory.items.size(), 9u);
}

TEST(GrammarFactoryInitTest, BaseItemsHaveExpectedProductions) {
    GrammarFactory factory;
    factory.Init();
    ASSERT_EQ(factory.items.size(), 9u);

    std::vector<std::vector<std::vector<std::string>>> expected = {
        {{"a", "b", "A"}, {"a"}},
        {{"a", "b", "A"}, {"a", "b"}},
        {{"a", "A", "b"}, {"EPSILON"}},
        {{"A", "a"}, {"EPSILON"}},
        {{"a", "A"}, {"EPSILON"}},
        {{"a", "A", "c"}, {"b"}},
        {{"a", "A", "a"}, {"b"}},
        {{"A", "a"}, {"b"}},
        {{"b", "A"}, {"a"}}};

    for (size_t i = 0; i < expected.size(); ++i) {
        const auto& map_i = factory.items[i];
        ASSERT_TRUE(map_i.g_.contains("A"));

        const auto& prods = map_i.g_.at("A");
        ASSERT_EQ(prods.size(), expected[i].size());

        for (size_t j = 0; j < prods.size(); ++j) {
            EXPECT_EQ(prods[j], expected[i][j]);
        }
    }
}

TEST(GrammarFactoryTest, Lv1GrammarIsOneBaseGrammar) {
    GrammarFactory factory;

    factory.Init();
    Grammar g = factory.PickOne(1);

    ASSERT_FALSE(g.g_.empty());
    ASSERT_GE(g.g_.size(), 2);
    g.g_.erase("S");
    bool ret = std::ranges::any_of(
        factory.items, [&g](const GrammarFactory::FactoryItem& item) {
            return item.g_ == g.g_;
        });
    ASSERT_TRUE(ret);
}

TEST(GrammarFactoryTest, Lv2GrammarHaveSizeGt3) {
    GrammarFactory factory;

    factory.Init();
    Grammar g = factory.PickOne(2);

    ASSERT_FALSE(g.g_.empty());
    ASSERT_GE(g.g_.size(), 3);
    ASSERT_TRUE(g.g_.contains("A"));
    ASSERT_TRUE(g.g_.contains("B"));
}

TEST(GrammarFactoryTest, Lv3GrammarHaveSizeGt4) {
    GrammarFactory factory;

    factory.Init();
    Grammar g = factory.PickOne(3);

    ASSERT_FALSE(g.g_.empty());
    ASSERT_GE(g.g_.size(), 4);
    ASSERT_TRUE(g.g_.contains("A"));
    ASSERT_TRUE(g.g_.contains("B"));
    ASSERT_TRUE(g.g_.contains("C"));
}

TEST(GrammarFactoryTest, GeneratedLv1LL1GrammarIsAlwaysLL1) {
    GrammarFactory factory;

    factory.Init();
    for (int i = 0; i < 100; ++i) {
        Grammar   g = factory.GenLL1Grammar(1);
        LL1Parser ll1(g);
        ASSERT_TRUE(ll1.CreateLL1Table());
    }
}

TEST(GrammarFactoryTest, GeneratedLv2LL1GrammarIsAlwaysLL1) {
    GrammarFactory factory;

    factory.Init();
    for (int i = 0; i < 100; ++i) {
        Grammar   g = factory.GenLL1Grammar(2);
        LL1Parser ll1(g);
        ASSERT_TRUE(ll1.CreateLL1Table());
    }
}

TEST(GrammarFactoryTest, GeneratedLv3LL1GrammarIsAlwaysLL1) {
    GrammarFactory factory;

    factory.Init();
    for (int i = 0; i < 100; ++i) {
        Grammar   g = factory.GenLL1Grammar(3);
        LL1Parser ll1(g);
        ASSERT_TRUE(ll1.CreateLL1Table());
    }
}

TEST(GrammarFactoryTest, GeneratedLv4LL1GrammarIsAlwaysLL1) {
    GrammarFactory factory;

    factory.Init();
    for (int i = 0; i < 10; ++i) {
        Grammar   g = factory.GenLL1Grammar(4);
        LL1Parser ll1(g);
        ASSERT_TRUE(ll1.CreateLL1Table());
    }
}

TEST(GrammarFactoryTest, GeneratedLv5LL1GrammarIsAlwaysLL1) {
    GrammarFactory factory;

    factory.Init();
    for (int i = 0; i < 10; ++i) {
        Grammar   g = factory.GenLL1Grammar(5);
        LL1Parser ll1(g);
        ASSERT_TRUE(ll1.CreateLL1Table());
    }
}

TEST(GrammarFactoryTest, GeneratedLv6LL1GrammarIsAlwaysLL1) {
    GrammarFactory factory;

    factory.Init();
    for (int i = 0; i < 10; ++i) {
        Grammar   g = factory.GenLL1Grammar(6);
        LL1Parser ll1(g);
        ASSERT_TRUE(ll1.CreateLL1Table());
    }
}

TEST(GrammarFactoryTest, GeneratedLv7LL1GrammarIsAlwaysLL1) {
    GrammarFactory factory;

    factory.Init();
    for (int i = 0; i < 3; ++i) {
        Grammar   g = factory.GenLL1Grammar(7);
        LL1Parser ll1(g);
        ASSERT_TRUE(ll1.CreateLL1Table());
    }
}

TEST(GrammarFactoryTest, GeneratedLv1SLR1GrammarIsAlwaysSLR1) {
    GrammarFactory factory;

    factory.Init();
    for (int i = 0; i < 100; ++i) {
        Grammar    g = factory.GenSLR1Grammar(1);
        SLR1Parser slr1(g);
        ASSERT_TRUE(slr1.MakeParser());
    }
}

TEST(GrammarFactoryTest, GeneratedLv2SLR1GrammarIsAlwaysSLR1) {
    GrammarFactory factory;

    factory.Init();
    for (int i = 0; i < 100; ++i) {
        Grammar    g = factory.GenSLR1Grammar(2);
        SLR1Parser slr1(g);
        ASSERT_TRUE(slr1.MakeParser());
    }
}

TEST(GrammarFactoryTest, GeneratedLv3SLR1GrammarIsAlwaysSLR1) {
    GrammarFactory factory;

    factory.Init();
    for (int i = 0; i < 100; ++i) {
        Grammar    g = factory.GenSLR1Grammar(3);
        SLR1Parser slr1(g);
        ASSERT_TRUE(slr1.MakeParser());
    }
}

TEST(GrammarFactoryTest, GeneratedLv4SLR1GrammarIsAlwaysSLR1) {
    GrammarFactory factory;

    factory.Init();
    for (int i = 0; i < 20; ++i) {
        Grammar    g = factory.GenSLR1Grammar(4);
        SLR1Parser slr1(g);
        ASSERT_TRUE(slr1.MakeParser());
    }
}

TEST(GrammarFactoryTest, GeneratedLv5SLR1GrammarIsAlwaysSLR1) {
    GrammarFactory factory;

    factory.Init();
    for (int i = 0; i < 10; ++i) {
        Grammar    g = factory.GenSLR1Grammar(5);
        SLR1Parser slr1(g);
        ASSERT_TRUE(slr1.MakeParser());
    }
}

TEST(GrammarFactoryTest, GeneratedLv6SLR1GrammarIsAlwaysSLR1) {
    GrammarFactory factory;

    factory.Init();
    for (int i = 0; i < 5; ++i) {
        Grammar    g = factory.GenSLR1Grammar(6);
        SLR1Parser slr1(g);
        ASSERT_TRUE(slr1.MakeParser());
    }
}

TEST(GrammarFactoryTest, GeneratedLv7SLR1GrammarIsAlwaysSLR1) {
    GrammarFactory factory;

    factory.Init();
    for (int i = 0; i < 5; ++i) {
        Grammar    g = factory.GenSLR1Grammar(7);
        SLR1Parser slr1(g);
        ASSERT_TRUE(slr1.MakeParser());
    }
}

TEST(GrammarFactoryTest, NormalizeNonTerminals_Basic) {
    GrammarFactory factory;

    std::unordered_map<std::string, std::vector<production>> g{
        {"A", {{"a", "B"}, {"b"}}}, {"B", {{"c"}}}};

    GrammarFactory::FactoryItem item(g);

    factory.NormalizeNonTerminals(item, "X");

    ASSERT_EQ(item.g_.size(), 1u);
    ASSERT_TRUE(item.g_.contains("X"));
    production              p1{"a", "X"};
    production              p2{"b"};
    production              p3{"c"};
    std::vector<production> expected{p1, p2, p3};
    auto                    result = item.g_["X"];
    ASSERT_EQ(result.size(), expected.size());
    for (const auto& prod : expected) {
        auto it = std::find(result.begin(), result.end(), prod);
        EXPECT_NE(it, result.end());
    }
    EXPECT_EQ(item.st_.non_terminals_.size(), 1u);
    EXPECT_TRUE(item.st_.non_terminals_.contains("X"));
}

TEST(GrammarFactoryTest, AdjustTerminals_ReplacesCorrectly) {
    GrammarFactory factory;

    std::unordered_map<std::string, std::vector<production>> base_g{
        {"A", {{"a"}}}};
    std::unordered_map<std::string, std::vector<production>> cmb_g{
        {"B", {{"b"}}}};

    GrammarFactory::FactoryItem base(base_g);
    GrammarFactory::FactoryItem cmb(cmb_g);

    factory.AdjustTerminals(base, cmb, "X");

    ASSERT_EQ(base.g_.size(), 1u);
    ASSERT_TRUE(base.g_.contains("A"));
    ASSERT_EQ(base.g_["A"], std::vector<production>{{{"X"}}});
    ASSERT_EQ(base.st_.terminals_wtho_eol_.size(), 1u);
    ASSERT_FALSE(base.st_.terminals_wtho_eol_.contains("b"));
}

TEST(GrammarFactoryTest, Merge_AppendsProductions) {
    GrammarFactory factory;

    std::unordered_map<std::string, std::vector<production>> base_g{
        {"A", {{"a"}}}};
    std::unordered_map<std::string, std::vector<production>> cmb_g{
        {"A", {{"b"}}}};

    GrammarFactory::FactoryItem base(base_g);
    GrammarFactory::FactoryItem cmb(cmb_g);

    auto merged = factory.Merge(base, cmb);

    ASSERT_EQ(merged.size(), 1u);
    ASSERT_TRUE(merged.contains("A"));
    std::vector<production> expected{{{"a"}}, {{"b"}}};
    EXPECT_EQ(merged["A"], expected);
}

TEST(GrammarFactoryTest, NormalizeNonTerminals_MultipleSymbols) {
    GrammarFactory factory;

    std::unordered_map<std::string, std::vector<production>> g{
        {"A", {{"a", "B"}, {"b", "C"}}},
        {"B", {{"c"}}},
        {"C", {{"d", "A"}}},
    };

    GrammarFactory::FactoryItem item(g);

    factory.NormalizeNonTerminals(item, "N");

    ASSERT_EQ(item.g_.size(), 1u);
    ASSERT_TRUE(item.g_.contains("N"));
    std::vector<production> expected{{"a", "N"}, {"b", "N"}, {"c"}, {"d", "N"}};
    auto                    result = item.g_["N"];
    ASSERT_EQ(result.size(), expected.size());
    for (const auto& prod : expected) {
        auto it = std::find(result.begin(), result.end(), prod);
        EXPECT_NE(it, result.end());
    }
    EXPECT_EQ(item.st_.non_terminals_.size(), 1u);
    EXPECT_TRUE(item.st_.non_terminals_.contains("N"));
}

TEST(GrammarFactoryTest, AdjustTerminals_EarlyReturnWhenAlphabetEmpty) {
    GrammarFactory factory;

    std::unordered_map<std::string, std::vector<production>> base_g{
        {"A", {{"a"}}}};
    std::unordered_map<std::string, std::vector<production>> cmb_g;
    for (const std::string& t : std::vector<std::string>{
             "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l"}) {
        cmb_g["B"].push_back({t});
    }

    GrammarFactory::FactoryItem base(base_g);
    GrammarFactory::FactoryItem cmb(cmb_g);

    factory.AdjustTerminals(base, cmb, "X");

    ASSERT_EQ(base.g_.size(), 1u);
    ASSERT_EQ(base.g_["A"], std::vector<production>{{{"a"}}});
    ASSERT_EQ(base.st_.terminals_wtho_eol_.size(), 1u);
    EXPECT_TRUE(base.st_.terminals_wtho_eol_.contains("a"));
}

TEST(GrammarFactoryTest, AdjustTerminals_DeterministicReplacement) {
    GrammarFactory factory;

    std::unordered_map<std::string, std::vector<production>> base_g{
        {"A", {{"a"}}}};
    std::unordered_map<std::string, std::vector<production>> cmb_g;
    for (const std::string& t : std::vector<std::string>{
             "a", "b", "d", "e", "f", "g", "h", "i", "j", "k", "l"}) {
        cmb_g["B"].push_back({t});
    }

    GrammarFactory::FactoryItem base(base_g);
    GrammarFactory::FactoryItem cmb(cmb_g);

    factory.AdjustTerminals(base, cmb, "X");

    ASSERT_EQ(base.g_.size(), 1u);
    ASSERT_TRUE(base.g_.contains("A"));
    ASSERT_EQ(base.g_["A"], std::vector<production>{{{"X"}}});
    ASSERT_EQ(base.st_.terminals_wtho_eol_.size(), 1u);
    EXPECT_TRUE(base.st_.terminals_wtho_eol_.contains("c"));
    EXPECT_FALSE(base.st_.terminals_wtho_eol_.contains("a"));
}

TEST(GrammarFactoryTest, Merge_MultipleNonTerminals) {
    GrammarFactory factory;

    std::unordered_map<std::string, std::vector<production>> base_g{
        {"A", {{"a"}}}, {"B", {{"b"}}}};
    std::unordered_map<std::string, std::vector<production>> cmb_g{
        {"B", {{"x"}}}, {"C", {{"c"}}}, {"D", {{"d"}}}};

    GrammarFactory::FactoryItem base(base_g);
    GrammarFactory::FactoryItem cmb(cmb_g);

    auto merged = factory.Merge(base, cmb);

    ASSERT_EQ(merged.size(), 4u);
    EXPECT_EQ(merged["A"], std::vector<production>{{"a"}});
    EXPECT_EQ(merged["B"], (std::vector<production>{{"b"}, {"x"}}));
    EXPECT_EQ(merged["C"], std::vector<production>{{"c"}});
    EXPECT_EQ(merged["D"], std::vector<production>{{"d"}});
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
        {"A", {{"a"}}}, {"B", {{"b"}}}};
    Grammar   g(G);
    LL1Parser ll1(g);

    bool ok = ll1.CreateLL1Table();
    EXPECT_TRUE(ok);
}

TEST(LL1__Test, CreateLL1Table_DirectConflict) {
    std::unordered_map<std::string, std::vector<production>> G = {
        {"A", {{"a"}, {"a"}}}};
    Grammar   g(G);
    LL1Parser ll1(g);

    bool ok = ll1.CreateLL1Table();
    EXPECT_FALSE(ok);
}

TEST(LL1__Test, First_EmptyRuleYieldsEpsilon) {
    Grammar   g;
    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    ll1.First({}, result);

    EXPECT_TRUE(result.count(g.st_.EPSILON_));
    EXPECT_EQ(result.size(), 1u);
}

TEST(LL1__Test, First_RuleStartingWithEOLYieldsEpsilon) {
    Grammar   g;
    LL1Parser ll1(g);

    std::unordered_set<std::string> result;
    ll1.First({{"$"}}, result);

    EXPECT_TRUE(result.count(g.st_.EPSILON_));
    EXPECT_EQ(result.size(), 1u);
}

TEST(LL1__Test, First_LeadingEpsilonSkipsToNext) {
    Grammar g;
    g.st_.PutSymbol("a", true);

    LL1Parser                       ll1(g);
    std::unordered_set<std::string> result;
    ll1.First({{"EPSILON", "a"}}, result);

    EXPECT_TRUE(result.count("a"));
    EXPECT_FALSE(result.count(g.st_.EPSILON_));
}

TEST(LL1__Test, First_SingleTerminal) {
    Grammar g;
    g.st_.PutSymbol("x", true);

    LL1Parser                       ll1(g);
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
        {"A", {{"EPSILON"}, {"a"}}}};

    Grammar   g(G);
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
    Grammar                  g;
    std::vector<std::string> syms{"A", "B", "C", "a", "c"};
    for (auto& sym : syms) {
        g.st_.PutSymbol(sym, sym[0] >= 'a' && sym[0] <= 'z');
        if (sym == "A" || sym == "B" || sym == "C")
            g.st_.non_terminals_.insert(sym);
    }
    g.st_.PutSymbol(g.st_.EPSILON_, true);

    g.axiom_ = "S";
    g.AddProduction("S", {"A", "B", "C"});
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
    Grammar   g;
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

TEST(LL1__Test, FirstAndFollowSetsFilledWhenEmpty) {
    GrammarFactory factory;
    factory.Init();

    Grammar   g = factory.GenLL1Grammar(1);
    LL1Parser ll1(g);
    ll1.first_sets_.clear();
    ll1.follow_sets_.clear();

    ASSERT_TRUE(ll1.first_sets_.empty());
    ASSERT_TRUE(ll1.follow_sets_.empty());

    ll1.CreateLL1Table();

    ASSERT_FALSE(ll1.first_sets_.empty());
    ASSERT_FALSE(ll1.follow_sets_.empty());
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
        {"A", {g.st_.EPSILON_}, 1, g.st_.EPSILON_, g.st_.EOL_}};

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

TEST(Lr0ItemTest, BehaviourMethods) {
    Lr0Item item{"A", {"a", "b"}, 0, "EPSILON", "$"};
    EXPECT_EQ(item.NextToDot(), "a");
    item.AdvanceDot();
    EXPECT_FALSE(item.IsComplete());
    EXPECT_EQ(item.NextToDot(), "b");
    item.AdvanceDot();
    EXPECT_TRUE(item.IsComplete());
    EXPECT_EQ(item.NextToDot(), "EPSILON");
}

TEST(SymbolTableTest, InsertAndQuery) {
    SymbolTable st;
    st.PutSymbol("A", false);
    st.PutSymbol("a", true);
    EXPECT_TRUE(st.In("A"));
    EXPECT_TRUE(st.In("a"));
    EXPECT_TRUE(st.IsTerminal("a"));
    EXPECT_FALSE(st.IsTerminal("A"));
    EXPECT_TRUE(st.IsTerminalWthoEol("a"));
}

TEST(StateTest, EqualityAndHash) {
    state s1;
    s1.id_ = 0;
    s1.items_.insert({"A", {"a"}, 0, "EPSILON", "$"});

    state s2;
    s2.id_ = 1;
    s2.items_.insert({"A", {"a"}, 0, "EPSILON", "$"});

    EXPECT_EQ(s1, s2);
    std::unordered_set<state> set;
    set.insert(s1);
    EXPECT_EQ(set.count(s2), 1u);
}

TEST(StateTest, EmptyStatesAreEqual) {
    state s1, s2;
    EXPECT_TRUE(s1 == s2);
}

TEST(StateTest, StatesDifferWhenItemsDiffer) {
    state s1, s2;
    s1.items_.insert({"A", {"a"}, 0, "EPSILON", "$"});
    EXPECT_FALSE(s1 == s2);
    s2.items_.insert({"A", {"a"}, 0, "EPSILON", "$"});
    EXPECT_TRUE(s1 == s2);
}

TEST(StateTest, HashOfEmptyStateIsZero) {
    state  s;
    size_t h = std::hash<state>()(s);
    EXPECT_EQ(h, 0u);
}

TEST(StateTest, HashOfNonEmptyStateIsConsistent) {
    state s1, s2;
    s1.items_.insert({"A", {"a"}, 0, "EPSILON", "$"});
    s1.items_.insert({"B", {"b"}, 1, "EPSILON", "$"});
    s2.items_ = s1.items_;

    size_t h1 = std::hash<state>()(s1);
    size_t h2 = std::hash<state>()(s2);
    EXPECT_NE(h1, 0u);
    EXPECT_EQ(h1, h2);
}

TEST(SLR1ParserTest, AllItemsGeneratesEveryItem) {
    Grammar g;
    g.st_.PutSymbol("S", false);
    g.st_.PutSymbol("a", true);
    g.axiom_ = "S";
    g.AddProduction("S", {"a", g.st_.EOL_});

    SLR1Parser slr1(g);
    auto       items = slr1.AllItems();

    std::unordered_set<Lr0Item> expected = {
        {"S", {"a", g.st_.EOL_}, 0, g.st_.EPSILON_, g.st_.EOL_},
        {"S", {"a", g.st_.EOL_}, 1, g.st_.EPSILON_, g.st_.EOL_},
        {"S", {"a", g.st_.EOL_}, 2, g.st_.EPSILON_, g.st_.EOL_}};
    EXPECT_EQ(items, expected);
}

TEST(GrammarFactoryHelperTest, LongestCommonPrefix) {
    GrammarFactory          factory;
    std::vector<production> prods{{"a", "b", "c"}, {"a", "b", "d"}, {"a", "b"}};
    auto                    prefix = factory.LongestCommonPrefix(prods);
    std::vector<std::string> expected{"a", "b"};
    EXPECT_EQ(prefix, expected);
}

TEST(GrammarFactoryHelperTest,
     StartsWithReturnsFalseWhenProdIsShorterThanPrefix) {
    GrammarFactory factory;
    production     prod{"a", "b"};
    EXPECT_FALSE(factory.StartsWith(prod, {"a", "b", "c"}));
    EXPECT_FALSE(factory.StartsWith(prod, {"a", "c", "b"}));
}

TEST(GrammarFactoryHelperTest, StartsWithAndGenerateNonTerminal) {
    GrammarFactory factory;
    production     prod{"a", "b", "c"};
    EXPECT_TRUE(factory.StartsWith(prod, {"a", "b"}));
    EXPECT_FALSE(factory.StartsWith(prod, {"a", "c"}));

    Grammar g;
    g.st_.PutSymbol("A", false);
    g.st_.PutSymbol("A'", false);
    std::string nt = factory.GenerateNewNonTerminal(g, "A");
    EXPECT_EQ(nt, "A''");
}

TEST(GrammarFactoryHelperTest, HasCycleDetection) {
    GrammarFactory                                                   factory;
    std::unordered_map<std::string, std::unordered_set<std::string>> cyc{
        {"A", {"B"}}, {"B", {"C"}}, {"C", {"A"}}};
    EXPECT_TRUE(factory.HasCycle(cyc));

    std::unordered_map<std::string, std::unordered_set<std::string>> acyc{
        {"A", {"B"}}, {"B", {"C"}}, {"C", {}}};
    EXPECT_FALSE(factory.HasCycle(acyc));
}

TEST(GrammarFactoryTest, HigherLevelGrammarsExist) {
    GrammarFactory factory;
    factory.Init();
    EXPECT_GE(factory.Lv4().g_.size(), 4u);
    EXPECT_GE(factory.Lv5().g_.size(), 5u);
    EXPECT_GE(factory.Lv6().g_.size(), 6u);
    EXPECT_GE(factory.Lv7().g_.size(), 7u);
}

TEST(SymbolTableTest, PutSymbol_AddsTerminal) {
    SymbolTable st;
    std::string id = "foo";

    st.PutSymbol(id, true);

    EXPECT_TRUE(st.In(id));

    EXPECT_TRUE(st.IsTerminal(id));

    EXPECT_TRUE(st.IsTerminalWthoEol(id));
}

TEST(SymbolTableTest, PutSymbol_AddsNonTerminal) {
    SymbolTable st;
    std::string id = "bar";

    st.PutSymbol(id, false);

    EXPECT_TRUE(st.In(id));

    EXPECT_FALSE(st.IsTerminal(id));

    EXPECT_FALSE(st.IsTerminalWthoEol(id));
}

TEST(SymbolTableTest, In_DetectsOnlyInserted) {
    SymbolTable st;
    EXPECT_FALSE(st.In("x"));

    st.PutSymbol("x", true);
    EXPECT_TRUE(st.In("x"));

    EXPECT_FALSE(st.In("y"));
}

TEST(SymbolTableTest, IsTerminal_OnlyTrueForTerminals) {
    SymbolTable st;
    st.PutSymbol("t", true);
    st.PutSymbol("nt", false);

    EXPECT_TRUE(st.IsTerminal("t"));
    EXPECT_FALSE(st.IsTerminal("nt"));
    EXPECT_FALSE(st.IsTerminal("missing"));
}

TEST(SymbolTableTest, IsTerminalWthoEol_ExcludesEpsilon) {
    SymbolTable st;
    st.PutSymbol(st.EPSILON_, true);

    EXPECT_TRUE(st.IsTerminal(st.EPSILON_));
    EXPECT_FALSE(st.IsTerminalWthoEol(st.EPSILON_));
}

TEST(SymbolTableTest, IsTerminalWthoEol_OnlyTrueForNonEpsilonTerminals) {
    SymbolTable st;
    st.PutSymbol("a", true);
    EXPECT_TRUE(st.IsTerminalWthoEol("a"));

    st.PutSymbol("B", false);
    EXPECT_FALSE(st.IsTerminalWthoEol("B"));
    EXPECT_FALSE(st.IsTerminalWthoEol("C"));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
