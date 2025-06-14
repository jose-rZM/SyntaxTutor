#include "grammar_factory.hpp"
#include "ll1_parser.hpp"
#include "slr1_parser.hpp"
#include <algorithm>
#include <ranges>
#include <iostream>
#include <queue>
#include <random>

void GrammarFactory::Init() {
    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"a", "b", "A"}, {"a"}}}});

    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"a", "b", "A"}, {"a", "b"}}}});

    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"a", "A", "b"}, {"EPSILON"}}}});

    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"A", "a"}, {"EPSILON"}}}});

    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"a", "A"}, {"EPSILON"}}}});

    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"a", "A", "c"}, {"b"}}}});

    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"a", "A", "a"}, {"b"}}}});

    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"A", "a"}, {"b"}}}});

    items.emplace_back(
        std::unordered_map<std::string, std::vector<std::vector<std::string>>>{
            {"A", {{"b", "A"}, {"a"}}}});
}

Grammar GrammarFactory::PickOne(int level) {
    switch (level) {
    case 1:
        return Lv1();
        break;
    case 2:
        return Lv2();
        break;
    case 3:
        return Lv3();
        break;
    case 4:
        return Lv4();
        break;
    case 5:
        return Lv5();
        break;
    case 6:
        return Lv6();
        break;
    default:
        return Lv7();
        break;
    }
}

Grammar GrammarFactory::GenLL1Grammar(int level) {
    Grammar   gr = PickOne(level);
    LL1Parser ll1(gr);
    while (IsInfinite(gr) || HasUnreachableSymbols(gr) ||
           HasDirectLeftRecursion(gr) || !ll1.CreateLL1Table()) {
        RemoveLeftRecursion(gr);
        ll1 = LL1Parser(gr);
        if (ll1.CreateLL1Table()) {
            break;
        }
        LeftFactorize(gr);
        ll1 = LL1Parser(gr);
        if (ll1.CreateLL1Table()) {
            break;
        }
        gr = PickOne(level);
    }
    return gr;
}

Grammar GrammarFactory::GenSLR1Grammar(int level) {
    Grammar    gr = PickOne(level);
    SLR1Parser slr1(gr);

    while (IsInfinite(gr) || HasUnreachableSymbols(gr) || !slr1.MakeParser()) {
        gr   = PickOne(level);
        slr1 = SLR1Parser(gr);
    }
    return gr;
}

void GrammarFactory::SanityChecks(Grammar& gr) {
    std::cout << "Sanity check (Is Infinite?) : " << IsInfinite(gr) << "\n";
    std::cout << "Sanity check (Has Unreachable Symbols?) : "
              << HasUnreachableSymbols(gr) << "\n";
    std::cout << "Sanity check (Has Direct Left Recursion?) : "
              << HasDirectLeftRecursion(gr) << "\n";
}

Grammar GrammarFactory::Lv1() {
    std::random_device                    rd;
    std::mt19937                          gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    return Grammar(items.at(dist(gen)).g_);
}

Grammar GrammarFactory::Lv2() {
    return Grammar(CreateLv2Item().g_);
}

Grammar GrammarFactory::Lv3() {
    // STEP 1 Build a random LV2 base grammar ------------------------------
    FactoryItem base = CreateLv2Item();

    // STEP 2 Choose a random LV1 grammar -------------------------------
    std::random_device                    rd;
    std::mt19937                          gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    FactoryItem                           cmb = items.at(dist(gen));

    NormalizeNonTerminals(cmb, "C");
    AdjustTerminals(base, cmb, "C");

    return Grammar(Merge(base, cmb));
}

Grammar GrammarFactory::Lv4() {
    // STEP 1 Build a random LV3 base grammar ------------------------------
    Grammar g = Lv3();
    g.g_.erase("S");
    FactoryItem base(g.g_);

    // STEP 2 Choose a random LV1 grammar -------------------------------
    std::random_device                    rd;
    std::mt19937                          gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    FactoryItem                           cmb = items.at(dist(gen));

    NormalizeNonTerminals(cmb, "D");
    AdjustTerminals(base, cmb, "D");

    return Grammar(Merge(base, cmb));
}

Grammar GrammarFactory::Lv5() {
    // STEP 1 Build a random LV3 base grammar ------------------------------
    Grammar g = Lv4();
    g.g_.erase("S");
    FactoryItem base(g.g_);

    // STEP 2 Choose a random LV1 grammar -------------------------------
    std::random_device                    rd;
    std::mt19937                          gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    FactoryItem                           cmb = items.at(dist(gen));

    NormalizeNonTerminals(cmb, "E");
    AdjustTerminals(base, cmb, "E");

    return Grammar(Merge(base, cmb));
}

Grammar GrammarFactory::Lv6() {
    // STEP 1 Build a random LV3 base grammar ------------------------------
    Grammar g = Lv5();
    g.g_.erase("S");
    FactoryItem base(g.g_);

    // STEP 2 Choose a random LV1 grammar -------------------------------
    std::random_device                    rd;
    std::mt19937                          gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    FactoryItem                           cmb = items.at(dist(gen));

    NormalizeNonTerminals(cmb, "F");
    AdjustTerminals(base, cmb, "F");

    return Grammar(Merge(base, cmb));
}

Grammar GrammarFactory::Lv7() {
    Grammar g = Lv6();
    g.g_.erase("S");
    FactoryItem base(g.g_);

    // STEP 2 Choose a random LV1 grammar -------------------------------
    std::random_device                    rd;
    std::mt19937                          gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    FactoryItem                           cmb = items.at(dist(gen));

    NormalizeNonTerminals(cmb, "G");
    AdjustTerminals(base, cmb, "G");

    return Grammar(Merge(base, cmb));
}

GrammarFactory::FactoryItem GrammarFactory::CreateLv2Item() {
    // STEP 1 Choose a random base grammar ----------------------------------
    std::random_device                    rd;
    std::mt19937                          gen(rd());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
    FactoryItem                           base = items.at(dist(gen));
    // -----------------------------------------------------

    // STEP 2 Choose a random cmb grammar such that base != cmb
    // ------------------------------
    FactoryItem cmb = items.at(dist(gen));
    while (base.g_ == cmb.g_) {
        cmb = items.at(dist(gen));
    }
    // -----------------------------------------------------

    NormalizeNonTerminals(cmb, "B");
    AdjustTerminals(base, cmb, "B");

    return FactoryItem(Merge(base, cmb));
}

bool GrammarFactory::HasUnreachableSymbols(Grammar& grammar) const {
    std::unordered_set<std::string> reachable;
    std::queue<std::string>         pending;

    pending.push(grammar.axiom_);
    reachable.insert(grammar.axiom_);

    while (!pending.empty()) {
        std::string current = pending.front();
        pending.pop();

        auto it = grammar.g_.find(current);
        if (it != grammar.g_.end()) {
            for (const auto& production : it->second) {
                for (const auto& symbol : production) {
                    if (!grammar.st_.IsTerminal(symbol) &&
                        !reachable.contains(symbol)) {
                        reachable.insert(symbol);
                        pending.push(symbol);
                    }
                }
            }
        }
    }

    return std::ranges::any_of(grammar.st_.non_terminals_, [&](const auto& nt) {
        return !reachable.contains(nt);
    });
}

bool GrammarFactory::IsInfinite(Grammar& grammar) const {
    std::unordered_set<std::string> generating;
    bool                            changed = true;

    while (changed) {
        changed = false;
        for (const auto& [nt, productions] : grammar.g_) {
            if (generating.contains(nt)) {
                continue;
            }
            for (const auto& prod : productions) {
                bool all_generating = true;
                for (const auto& symbol : prod) {
                    if (!grammar.st_.IsTerminal(symbol) &&
                        !generating.contains(symbol)) {
                        all_generating = false;
                        break;
                    }
                }
                if (all_generating) {
                    generating.insert(nt);
                    changed = true;
                    break;
                }
            }
        }
    }
    // Counterexample:  S -> A; A -> B A c | e; B -> B a | B. Axiom can derive
    // into a terminal string (A -> e) return generating.find(grammar.axiom_) ==
    // generating.end();
    return generating != grammar.st_.non_terminals_;
}

bool GrammarFactory::HasDirectLeftRecursion(const Grammar& grammar) const {
    for (const auto& [nt, prods] : grammar.g_) {
        for (const auto& prod : prods) {
            if (nt == prod[0]) {
                return true;
            }
        }
    }
    return false;
}

bool GrammarFactory::HasIndirectLeftRecursion(const Grammar& grammar) const {
    std::unordered_set<std::string> nullable = NullableSymbols(grammar);
    std::unordered_map<std::string, std::unordered_set<std::string>> graph;

    for (const auto& [nt, productions] : grammar.g_) {
        graph[nt] = {};
        for (const production& prod : productions) {
            if (!grammar.st_.IsTerminal(prod[0])) {
                graph[nt].insert(prod[0]);
            }
            for (size_t i = 1; i < prod.size(); ++i) {
                if (grammar.st_.IsTerminal(prod[i])) {
                    break;
                }
                graph[nt].insert(prod[i]);
                if (!nullable.contains(prod[i])) {
                    break;
                }
            }
        }
    }
    return !graph.empty() && HasCycle(graph);
}

bool GrammarFactory::HasCycle(
    const std::unordered_map<std::string, std::unordered_set<std::string>>&
        graph) const {
    std::unordered_map<std::string, int> in_degree;
    std::queue<std::string>              q;

    for (const auto& [nt, _] : graph) {
        in_degree[nt] = 0;
    }

    for (const auto& [nt, adjacents] : graph) {
        for (const std::string& adj : adjacents) {
            in_degree[adj]++;
        }
    }

    for (const auto& [node, degree] : in_degree) {
        if (degree == 0) {
            q.push(node);
        }
    }

    int processed_nodes = 0;
    while (!q.empty()) {
        std::string node = q.front();
        q.pop();
        processed_nodes++;

        for (const std::string& adj : graph.at(node)) {
            if (--in_degree[adj] == 0) {
                q.push(adj);
            }
        }
    }
    return static_cast<size_t>(processed_nodes) != in_degree.size();
}

std::unordered_set<std::string>
GrammarFactory::NullableSymbols(const Grammar& grammar) const {
    std::unordered_set<std::string> nullable;
    bool                            changed;

    nullable.reserve(grammar.st_.non_terminals_.size());
    do {
        changed = false;
        for (const auto& [nt, productions] : grammar.g_) {
            if (nullable.contains(nt)) {
                continue;
            }
            for (const production& prod : productions) {
                if (prod.size() == 1 && prod[0] == grammar.st_.EPSILON_) {
                    nullable.insert(nt);
                    changed = true;
                } else {
                    bool all_nullable = true;
                    for (const std::string& sym : prod) {
                        if (!nullable.contains(sym) && sym != grammar.st_.EOL_) {
                            all_nullable = false;
                            break;
                        }
                    }
                    if (all_nullable) {
                        nullable.insert(nt);
                        changed = true;
                    }
                }
            }
        }
    } while (changed);
    return nullable;
}

void GrammarFactory::RemoveLeftRecursion(Grammar& grammar) {
    if (!HasDirectLeftRecursion(grammar)) {
        return;
    }
    std::unordered_map<std::string, std::vector<production>> new_rules;
    for (const auto& [nt, productions] : grammar.g_) {
        std::vector<production> alpha;
        std::vector<production> beta;
        std::string new_non_terminal = GenerateNewNonTerminal(grammar, nt);
        for (const auto& prod : productions) {
            if (!prod.empty() && prod[0] == nt) {
                alpha.emplace_back(prod.begin() + 1, prod.end());
            } else {
                if (prod[0] == grammar.st_.EPSILON_) {
                    continue;
                }
                beta.emplace_back(prod);
            }
        }
        if (!alpha.empty()) {
            if (beta.empty()) {
                beta.emplace_back();
            }
            for (auto& b : beta) {
                b.push_back(new_non_terminal);
            }
            for (auto& a : alpha) {
                a.push_back(new_non_terminal);
            }
            alpha.emplace_back(production{grammar.st_.EPSILON_});
            new_rules[nt]               = beta;
            new_rules[new_non_terminal] = alpha;
            grammar.st_.PutSymbol(new_non_terminal, false);
        } else {
            new_rules[nt] = productions;
        }
    }
    // EPSILON was introduced to the grammar, ensure it is in the symbol table
    grammar.st_.PutSymbol(grammar.st_.EPSILON_, true);
    grammar.g_ = std::move(new_rules);
}

void GrammarFactory::RemoveUnitRules(Grammar& grammar) const {
    for (const auto& [nt, prods] : grammar.g_) {
        for (const auto& prod : prods) {
            if (prod.size() == 1 && !grammar.st_.IsTerminal(prod[0])) {
                grammar.g_[nt] = grammar.g_.at(prod[0]);
                if (HasUnreachableSymbols(grammar)) {
                    grammar.g_.erase(prod[0]);
                    grammar.st_.non_terminals_.erase(prod[0]);
                }
            }
        }
    }
}

void GrammarFactory::LeftFactorize(Grammar& grammar) {
    bool changed;
    do {
        changed = false;
        std::unordered_map<std::string, std::vector<production>> new_rules;

        for (const auto& [nt, productions] : grammar.g_) {
            std::vector<production> factored_productions;
            std::vector<production> remaining_productions = productions;

            while (!remaining_productions.empty()) {
                std::vector<std::string> common_prefix =
                    LongestCommonPrefix(remaining_productions);

                if (common_prefix.empty()) {
                    factored_productions.emplace_back(remaining_productions[0]);
                    remaining_productions.erase(remaining_productions.begin());
                } else {
                    std::string new_non_terminal =
                        GenerateNewNonTerminal(grammar, nt);
                    grammar.st_.PutSymbol(new_non_terminal, false);

                    std::vector<std::string> new_production = common_prefix;
                    new_production.push_back(new_non_terminal);
                    factored_productions.emplace_back(std::move(new_production));

                    std::vector<production> new_remaining_productions;
                    for (const auto& prod : remaining_productions) {
                        if (StartsWith(prod, common_prefix)) {
                            std::vector<std::string> remaining_part(
                                prod.begin() + common_prefix.size(),
                                prod.end());
                            if (remaining_part.empty()) {
                                remaining_part.push_back(grammar.st_.EPSILON_);
                                grammar.st_.PutSymbol(grammar.st_.EPSILON_, true);
                            }
                            new_remaining_productions.emplace_back(std::move(remaining_part));
                        } else {
                            factored_productions.emplace_back(prod);
                        }
                    }

                    new_rules[new_non_terminal] = new_remaining_productions;
                    changed                     = true;
                    break;
                }
            }

            new_rules[nt] = factored_productions;
        }

        grammar.g_ = std::move(new_rules);
    } while (changed);
}

std::vector<std::string> GrammarFactory::LongestCommonPrefix(
    const std::vector<production>& productions) {
    if (productions.empty() || productions.size() < 2) {
        return {};
    }

    std::vector<production> sorted = productions;
    std::sort(sorted.begin(), sorted.end());
    production& first      = sorted.front();
    production& last       = sorted.back();
    size_t      min_length = std::min(first.size(), last.size());

    size_t i = 0;
    while (i < min_length && first[i] == last[i]) {
        ++i;
    }
    return std::vector<std::string>(first.begin(), first.begin() + i);
}

bool GrammarFactory::StartsWith(const production&               prod,
                                const std::vector<std::string>& prefix) {
    if (prod.size() < prefix.size()) {
        return false;
    }
    size_t i = 0;
    while (i < prefix.size() && prod[i] == prefix[i]) {
        ++i;
    }
    return i == prefix.size();
}

std::string GrammarFactory::GenerateNewNonTerminal(Grammar&           grammar,
                                                   const std::string& base) {
    std::string nt = base;
    while (grammar.st_.non_terminals_.find(nt) !=
           grammar.st_.non_terminals_.end()) {
        nt += "'";
    }
    return nt;
}

GrammarFactory::FactoryItem::FactoryItem(
    const std::unordered_map<std::string, std::vector<production>>& grammar) {
    for (const auto& [nt, prods] : grammar) {
        st_.PutSymbol(nt, false);
        for (const auto& prod : prods) {
            for (const std::string& symbol : prod) {
                if (symbol == "EPSILON") {
                    st_.PutSymbol(symbol, true);
                    continue;
                } else if (std::islower(symbol[0])) {
                    st_.PutSymbol(symbol, true);
                }
            }
        }
    }
    g_ = grammar;
}

bool GrammarFactory::FactoryItem::HasEmptyProduction(
    const std::string& antecedent) {
    auto& rules = g_.at(antecedent);
    return std::ranges::find_if(rules, [&](const auto& rule) {
               return rule[0] == st_.EPSILON_;
           }) != rules.end();
}

void GrammarFactory::FactoryItem::Debug() {
    std::cout << "Grammar:\n";
    for (const auto& entry : g_) {
        std::cout << entry.first << " -> ";
        for (const std::vector<std::string>& prod : entry.second) {
            for (const std::string& symbol : prod) {
                std::cout << symbol << " ";
            }
            std::cout << "| ";
        }
        std::cout << "\n";
    }
}

void GrammarFactory::NormalizeNonTerminals(FactoryItem& item,
                                            const std::string& nt) const {
    std::unordered_map<std::string, std::vector<production>> updated;
    for (auto& [old_nt, prods] : item.g_) {
        for (auto& prod : prods) {
            for (auto& symbol : prod) {
                if (!item.st_.IsTerminal(symbol)) {
                    symbol = nt;
                }
            }
        }
        updated[nt].insert(updated[nt].end(), prods.begin(), prods.end());
    }
    item.g_ = std::move(updated);
    item.st_.non_terminals_.clear();
    item.st_.non_terminals_.emplace(nt);
}

void GrammarFactory::AdjustTerminals(FactoryItem& base, const FactoryItem& cmb,
                                     const std::string& target_nt) const {
    std::random_device rd;
    std::mt19937       gen(rd());

    std::unordered_set<std::string> cmb_terminals = cmb.st_.terminals_wtho_eol_;
    std::unordered_set<std::string> alphabet(terminal_alphabet_.begin(),
                                             terminal_alphabet_.end());
    for (const auto& t : cmb_terminals) {
        alphabet.erase(t);
    }
    if (alphabet.empty()) {
        return;
    }
    std::vector<std::string> remaining(alphabet.begin(), alphabet.end());
    std::uniform_int_distribution<size_t> dist(0, remaining.size() - 1);
    std::string new_terminal = remaining[dist(gen)];

    std::vector<std::string> base_terms(base.st_.terminals_wtho_eol_.begin(),
                                        base.st_.terminals_wtho_eol_.end());
    std::uniform_int_distribution<size_t> base_dist(0, base_terms.size() - 1);
    std::string to_replace = base_terms.at(base_dist(gen));

    for (auto& [nt, prods] : base.g_) {
        for (auto& prod : prods) {
            for (auto& symbol : prod) {
                if (symbol == to_replace) {
                    symbol = new_terminal;
                }
            }
        }
    }
    base.st_.terminals_wtho_eol_.erase(to_replace);
    base.st_.terminals_wtho_eol_.emplace(new_terminal);

    base_terms.assign(base.st_.terminals_wtho_eol_.begin(),
                      base.st_.terminals_wtho_eol_.end());
    base_dist = std::uniform_int_distribution<size_t>(0, base_terms.size() - 1);
    to_replace = *std::next(base.st_.terminals_wtho_eol_.begin(),
                            base_dist(gen));
    for (auto& [nt, prods] : base.g_) {
        for (auto& prod : prods) {
            for (auto& symbol : prod) {
                if (symbol == to_replace) {
                    symbol = target_nt;
                }
            }
        }
    }
}

std::unordered_map<std::string, std::vector<production>>
GrammarFactory::Merge(const FactoryItem& base, const FactoryItem& cmb) const {
    auto result = base.g_;
    for (const auto& [nt, prods] : cmb.g_) {
        result[nt].insert(result[nt].end(), prods.begin(), prods.end());
    }
    return result;
}
