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

#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <ranges>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "grammar.hpp"
#include "slr1_parser.hpp"
#include "symbol_table.hpp"

SLR1Parser::SLR1Parser(Grammar gr) : gr_(std::move(gr)) {
    ComputeFirstSets();
    ComputeFollowSets();
}

std::unordered_set<Lr0Item> SLR1Parser::AllItems() const {
    std::unordered_set<Lr0Item> items;
    for (const auto& rule : gr_.g_) {
        for (const auto& production : rule.second) {
            for (unsigned int i = 0; i <= production.size(); ++i)
                items.insert({rule.first, production, i, gr_.st_.EPSILON_,
                              gr_.st_.EOL_});
        }
    }
    return items;
}

void SLR1Parser::MakeInitialState() {
    state initial;
    initial.id_ = 0;
    auto axiom  = gr_.g_.at(gr_.axiom_);
    // the axiom must be unique
    initial.items_.insert(
        {gr_.axiom_, axiom[0], gr_.st_.EPSILON_, gr_.st_.EOL_});
    Closure(initial.items_);
    states_.insert(initial);
}

bool SLR1Parser::SolveLRConflicts(const state& st) {
    for (const Lr0Item& item : st.items_) {
        if (item.IsComplete()) {
            // Regla 3: Si el ítem es del axioma, ACCEPT en EOL
            if (item.antecedent_ == gr_.axiom_) {
                actions_[st.id_][gr_.st_.EOL_] = {nullptr, Action::Accept};
            } else {
                // Regla 2: Si el ítem es completo, REDUCE en FOLLOW(A)
                std::unordered_set<std::string> follows =
                    Follow(item.antecedent_);
                for (const std::string& sym : follows) {
                    auto it = actions_[st.id_].find(sym);
                    if (it != actions_[st.id_].end()) {
                        // Si ya hay un Reduce, comparar las reglas.
                        // REDUCE/REDUCE si reglas distintas
                        if (it->second.action == Action::Reduce) {
                            if (!(it->second.item->antecedent_ ==
                                      item.antecedent_ &&
                                  it->second.item->consequent_ ==
                                      item.consequent_)) {
                                return false;
                            }
                        } else {
                            return false; // SHIFT/REDUCE
                        }
                    }
                    actions_[st.id_][sym] = {&item, Action::Reduce};
                }
            }
        } else {
            // Regla 1: Si hay un terminal después del punto, hacemos SHIFT
            std::string nextToDot = item.NextToDot();
            if (gr_.st_.IsTerminal(nextToDot)) {
                auto it = actions_[st.id_].find(nextToDot);
                if (it != actions_[st.id_].end()) {
                    // Si hay una acción previa, hay conflicto si es REDUCE
                    if (it->second.action == Action::Reduce) {
                        return false;
                    }
                    // Si ya hay un SHIFT en esa celda, no hay conflicto (varios
                    // SHIFT están permitidos)
                }
                actions_[st.id_][nextToDot] = {nullptr, Action::Shift};
            }
        }
    }
    return true;
}

bool SLR1Parser::MakeParser() {
    ComputeFirstSets();
    ComputeFollowSets();
    MakeInitialState();
    std::queue<unsigned int> pending;
    pending.push(0);
    unsigned int current = 0;
    size_t       i       = 1;

    do {
        std::unordered_set<std::string> nextSymbols;
        current = pending.front();
        pending.pop();
        auto it = std::find_if(
            states_.begin(), states_.end(),
            [current](const state& st) -> bool { return st.id_ == current; });
        if (it == states_.end()) {
            break;
        }
        const state& qi = *it;
        std::ranges::for_each(qi.items_, [&](const Lr0Item& item) -> void {
            std::string next = item.NextToDot();
            if (next != gr_.st_.EPSILON_ && next != gr_.st_.EOL_) {
                nextSymbols.insert(next);
            }
        });
        for (const std::string& symbol : nextSymbols) {
            state newState;
            newState.id_ = i;
            for (const auto& item : qi.items_) {
                if (item.NextToDot() == symbol) {
                    Lr0Item newItem = item;
                    newItem.AdvanceDot();
                    newState.items_.insert(newItem);
                }
            }

            Closure(newState.items_);
            auto result = states_.insert(newState);
            std::map<std::string, unsigned int> column;

            if (result.second) {
                pending.push(i);
                if (transitions_.find(current) != transitions_.end()) {
                    transitions_[current].insert({symbol, i});
                } else {
                    std::map<std::string, unsigned int> column;
                    column.insert({symbol, i});
                    transitions_.insert({current, column});
                }
                ++i;
            } else {
                if (transitions_.find(current) != transitions_.end()) {
                    transitions_[current].insert({symbol, result.first->id_});
                } else {
                    std::map<std::string, unsigned int> column;
                    column.insert({symbol, result.first->id_});
                    transitions_.insert({current, column});
                }
            }
        }
        current++;
    } while (!pending.empty());
    for (const state& st : states_) {
        if (!SolveLRConflicts(st)) {
            return false;
        }
    }
    return true;
}

void SLR1Parser::Closure(std::unordered_set<Lr0Item>& items) {
    std::unordered_set<std::string> visited;
    ClosureUtil(items, items.size(), visited);
}

void SLR1Parser::ClosureUtil(std::unordered_set<Lr0Item>&     items,
                             unsigned int                     size,
                             std::unordered_set<std::string>& visited) {
    std::unordered_set<Lr0Item> newItems;

    for (const auto& item : items) {
        std::string next = item.NextToDot();
        if (next == gr_.st_.EPSILON_) {
            continue;
        }
        if (!gr_.st_.IsTerminal(next) &&
            std::find(visited.cbegin(), visited.cend(), next) ==
                visited.cend()) {
            const std::vector<production>& rules = gr_.g_.at(next);
            std::ranges::for_each(rules, [&](const auto& rule) -> void {
                newItems.insert(
                    {item.NextToDot(), rule, gr_.st_.EPSILON_, gr_.st_.EOL_});
            });
            visited.insert(next);
        }
    }
    items.insert(newItems.begin(), newItems.end());
    if (size != items.size())
        ClosureUtil(items, items.size(), visited);
}

std::unordered_set<Lr0Item>
SLR1Parser::Delta(const std::unordered_set<Lr0Item>& items,
                  const std::string&                 str) {
    if (str == gr_.st_.EPSILON_) {
        return {};
    }
    std::vector<Lr0Item> filtered;
    std::ranges::for_each(items, [&](const Lr0Item& item) -> void {
        std::string next = item.NextToDot();
        if (next == str) {
            filtered.push_back(item);
        }
    });
    if (filtered.empty()) {
        return {};
    } else {
        std::unordered_set<Lr0Item> delta_items;
        delta_items.reserve(filtered.size());
        for (Lr0Item& lr : filtered) {
            lr.AdvanceDot();
            delta_items.insert(lr);
        }
        Closure(delta_items);
        return delta_items;
    }
}

// GCOVR_EXCL_START
// LCOV_EXCL_START
std::string
SLR1Parser::PrintItems(const std::unordered_set<Lr0Item>& items) const {
    std::ostringstream output;
    for (const auto& item : items) {
        output << "  - ";
        output << item.ToString();
        output << "\n";
    }
    return output.str();
}
// LCOV_EXCL_STOP
// GCOVR_EXCL_STOP

void SLR1Parser::First(std::span<const std::string>     rule,
                       std::unordered_set<std::string>& result) {
    if (rule.empty() || (rule.size() == 1 && rule[0] == gr_.st_.EPSILON_)) {
        result.insert(gr_.st_.EPSILON_);
        return;
    }

    if (gr_.st_.IsTerminal(rule[0])) {
        // EOL cannot be in first sets, if we reach EOL it means that the axiom
        // is nullable, so epsilon is included instead
        if (rule[0] == gr_.st_.EOL_) {
            result.insert(gr_.st_.EPSILON_);
            return;
        }
        result.insert(rule[0]);
        return;
    }

    const std::unordered_set<std::string>& fii = first_sets_[rule[0]];
    for (const auto& s : fii) {
        if (s != gr_.st_.EPSILON_) {
            result.insert(s);
        }
    }

    if (fii.find(gr_.st_.EPSILON_) == fii.cend()) {
        return;
    }
    First(std::span<const std::string>(rule.begin() + 1, rule.end()), result);
}

// Least fixed point
void SLR1Parser::ComputeFirstSets() {
    // Init all FIRST to empty
    for (const auto& [nonTerminal, _] : gr_.g_) {
        first_sets_[nonTerminal] = {};
    }

    bool changed;
    do {
        auto old_first_sets = first_sets_; // Copy current state

        for (const auto& [nonTerminal, productions] : gr_.g_) {
            for (const auto& prod : productions) {
                std::unordered_set<std::string> tempFirst;
                First(prod, tempFirst);

                if (tempFirst.find(gr_.st_.EOL_) != tempFirst.end()) {
                    tempFirst.erase(gr_.st_.EOL_);
                    tempFirst.insert(gr_.st_.EPSILON_);
                }

                auto& current_set = first_sets_[nonTerminal];
                current_set.insert(tempFirst.begin(), tempFirst.end());
            }
        }

        // Until all remain the same
        changed = (old_first_sets != first_sets_);

    } while (changed);
}

void SLR1Parser::ComputeFollowSets() {
    for (const auto& [nt, _] : gr_.g_) {
        follow_sets_[nt] = {};
    }
    follow_sets_[gr_.axiom_].insert(gr_.st_.EOL_);

    bool changed;
    do {
        changed = false;
        for (const auto& rule : gr_.g_) {
            const std::string& lhs = rule.first;
            for (const production& rhs : rule.second) {
                for (size_t i = 0; i < rhs.size(); ++i) {
                    const std::string& symbol = rhs[i];
                    if (!gr_.st_.IsTerminal(symbol)) {
                        std::unordered_set<std::string> first_remaining;

                        if (i + 1 < rhs.size()) {
                            First(std::span<const std::string>(
                                      rhs.begin() + i + 1, rhs.end()),
                                  first_remaining);
                        } else {
                            first_remaining.insert(gr_.st_.EPSILON_);
                        }

                        for (const std::string& terminal : first_remaining) {
                            if (terminal != gr_.st_.EPSILON_) {
                                if (follow_sets_[symbol]
                                        .insert(terminal)
                                        .second) {
                                    changed = true;
                                }
                            }
                        }

                        if (first_remaining.find(gr_.st_.EPSILON_) !=
                            first_remaining.end()) {
                            for (const std::string& terminal :
                                 follow_sets_[lhs]) {
                                if (follow_sets_[symbol]
                                        .insert(terminal)
                                        .second) {
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    } while (changed);
}

std::unordered_set<std::string> SLR1Parser::Follow(const std::string& arg) {
    if (follow_sets_.find(arg) == follow_sets_.end()) {
        return {};
    }
    return follow_sets_.at(arg);
}
