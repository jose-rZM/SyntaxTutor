#include "grammar.hpp"
#include "symbol_table.hpp"
#include <algorithm>
#include <ranges>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <format>

Grammar::Grammar(
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
    axiom_ = "S";
    g_ = grammar;
    g_[axiom_] = {{"A", st_.EOL_}};
    st_.PutSymbol(axiom_, false);
}

void Grammar::TransformToAugmentedGrammar()
{
    std::string new_axiom = axiom_ + "'";
    g_[new_axiom] = {{axiom_}};
    st_.PutSymbol(new_axiom, false);

    axiom_ = new_axiom;
}

void Grammar::SetAxiom(const std::string& axiom)
{
    axiom_ = axiom;
}

bool Grammar::HasEmptyProduction(const std::string& antecedent)
{
    auto rules{g_.at(std::string{antecedent})};
    return std::ranges::find_if(rules, [&](const auto& rule) {
               return rule[0] == st_.EPSILON_;
           }) != rules.end();
}

std::vector<std::pair<const std::string, production>> Grammar::FilterRulesByConsequent(
    const std::string& arg)
{
    std::vector<std::pair<const std::string, production>> rules;
    for (const auto& [nt, prods] : g_) {
        for (const production& prod : prods) {
            if (std::ranges::find(prod, arg) != prod.end()) {
                rules.emplace_back(nt, prod);
            }
        }
    }
    return rules;
}

void Grammar::Debug() {
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
bool Grammar::HasLeftRecursion(const std::string&              antecedent,
                               const std::vector<std::string>& consequent) {
    return consequent.at(0) == antecedent;
}

std::string Grammar::GenerateNewNonTerminal(const std::string& base) {
    unsigned    i = 1;
    std::string new_nt;

    do {
        new_nt = std::format("{}'{}", base, i);
        ++i;
    } while (st_.non_terminals_.contains(new_nt));
    st_.non_terminals_.insert(new_nt);
    return new_nt;
}

void Grammar::AddProduction(const std::string&              antecedent,
                            const std::vector<std::string>& consequent) {
    g_[antecedent].push_back(consequent);
}

std::vector<std::string> Grammar::Split(const std::string& s) {
    if (s == st_.EPSILON_) {
        return {st_.EPSILON_};
    }
    std::vector<std::string> splitted;
    std::string str;
    unsigned start{0};
    unsigned end{1};

    while (end <= s.size()) {
        str = s.substr(start, end - start);

        if (st_.In(str)) {
            unsigned lookahead = end + 1;
            while (lookahead <= s.size()) {
                if (std::string extended =
                        s.substr(start, lookahead - start);
                    st_.In(extended)) {
                    end = lookahead;
                }
                ++lookahead;
            }
            splitted.push_back(s.substr(start, end - start));
            start = end;
            end = start + 1;
        } else {
            ++end;
        }
    }

    if (start < end - 1) {
        return {};
    }
    return splitted;
}
