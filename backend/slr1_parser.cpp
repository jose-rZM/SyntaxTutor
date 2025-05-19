#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <unordered_set>
#include <vector>

#include "grammar.hpp"
#include "slr1_parser.hpp"
#include "symbol_table.hpp"
#include "tabulate.hpp"

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

void SLR1Parser::DebugStates() const {
    tabulate::Table        table;
    tabulate::Table::Row_t header = {"State ID", "Items"};
    table.add_row(header);

    for (size_t state = 0; state < states_.size(); ++state) {
        tabulate::Table::Row_t row;
        const auto             currentIt = std::find_if(
            states_.begin(), states_.end(),
            [state](const auto& st) -> bool { return st.id_ == state; });
        row.push_back(std::to_string(state));
        std::string str = "";
        for (const auto& item : currentIt->items_) {
            str += item.ToString();
            str += "\n";
        }
        row.push_back(str);
        table.add_row(row);
    }
    table.column(0)
        .format()
        .font_align(tabulate::FontAlign::center)
        .font_color(tabulate::Color::cyan);
    table.row(0).format().font_align(tabulate::FontAlign::center);
    table.column(1).format().font_color(tabulate::Color::yellow);
    std::cout << table << "\n";
}

void SLR1Parser::DebugActions() {
    std::vector<std::string> columns;
    columns.reserve(gr_.st_.terminals_.size() + gr_.st_.non_terminals_.size());
    tabulate::Table        table;
    tabulate::Table::Row_t header = {"State"};
    for (const auto& s : gr_.st_.terminals_) {
        if (s == gr_.st_.EPSILON_) {
            continue;
        }
        columns.push_back(s);
    }
    columns.insert(columns.end(), gr_.st_.non_terminals_.begin(),
                   gr_.st_.non_terminals_.end());
    header.insert(header.end(), columns.begin(), columns.end());
    table.add_row(header);

    for (unsigned state = 0; state < states_.size(); ++state) {
        tabulate::Table::Row_t row_data{std::to_string(state)};

        const auto  action_entry = actions_.find(state);
        const auto  trans_entry  = transitions_.find(state);
        const auto& transitions  = trans_entry->second;
        for (const auto& symbol : columns) {
            std::string cell        = "-";
            const bool  is_terminal = gr_.st_.IsTerminal(symbol);

            if (!is_terminal) {
                if (trans_entry != transitions_.end()) {
                    const auto it = transitions.find(symbol);
                    if (it != transitions.end()) {
                        cell = std::to_string(it->second);
                    }
                }
            } else {
                if (action_entry != actions_.end()) {
                    const auto action_it = action_entry->second.find(symbol);
                    if (action_it != action_entry->second.end()) {
                        switch (action_it->second.action) {
                        case Action::Accept:
                            cell = "A";
                            break;
                        case Action::Reduce:
                            cell = "R";
                            break;
                        case Action::Shift:
                            if (trans_entry != transitions_.end()) {
                                const auto shift_it = transitions.find(symbol);
                                if (shift_it != transitions.end()) {
                                    cell =
                                        "S" + std::to_string(shift_it->second);
                                }
                            }
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
            row_data.push_back(cell);
        }
        table.add_row(row_data);
    }
    table.format().font_align(tabulate::FontAlign::center);
    table.column(0).format().font_color(tabulate::Color::cyan);
    table.row(0).format().font_color(tabulate::Color::magenta);
    std::cout << table << std::endl;

    tabulate::Table reduce_table;
    reduce_table.add_row({"State", "Symbol", "Production Rule"});

    const auto state_color  = tabulate::Color::cyan;
    const auto symbol_color = tabulate::Color::yellow;
    const auto rule_color   = tabulate::Color::magenta;

    for (const auto& [state, actions] : actions_) {
        for (const auto& [symbol, action] : actions) {
            if (action.action == Action::Reduce) {
                tabulate::Table::Row_t row;
                std::string            rule;
                rule += action.item->antecedent_ + " -> ";
                for (const auto& sym : action.item->consequent_) {
                    rule += sym + " ";
                }
                row.push_back(std::to_string(state));
                row.push_back(symbol);
                row.push_back(rule);
                reduce_table.add_row(row);
            }
        }
    }
    reduce_table.format().font_align(tabulate::FontAlign::center);
    reduce_table.column(0).format().font_color(state_color);
    reduce_table.column(1).format().font_color(symbol_color);
    reduce_table.column(2).format().font_color(rule_color);

    std::cout << "\n\n";
    std::cout << "Reduce Actions:" << std::endl;
    std::cout << reduce_table << std::endl;
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
        std::for_each(qi.items_.begin(), qi.items_.end(),
                      [&](const Lr0Item& item) -> void {
                          std::string next = item.NextToDot();
                          if (next != gr_.st_.EPSILON_) {
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

void SLR1Parser::TeachAllItems() {
    std::cout << "What is an LR(0) item?\n";
    std::cout << "An LR(0) item represents a production rule with a 'dot' (•) "
                 "indicating the current position in the rule.\n";
    std::cout << "For example, for the rule A -> B C, the LR(0) items are:\n";
    std::cout << "  - A -> • B C (dot at the beginning)\n";
    std::cout << "  - A -> B • C (dot after B)\n";
    std::cout << "  - A -> B C • (dot at the end, complete item)\n";
    std::cout << "\n";

    std::cout << "How are LR(0) items generated?\n";
    std::cout << "For each production rule, we generate all possible positions "
                 "of the dot.\n";
    std::cout << "For example, for the rule A -> B C, the dot can be at "
                 "position 0, 1, or 2.\n";
    std::cout << "This results in the following LR(0) items:\n";
    std::cout << "  - A -> • B C\n";
    std::cout << "  - A -> B • C\n";
    std::cout << "  - A -> B C •\n";
    std::cout << "\n";

    std::cout << "Now, let's generate all LR(0) items for the given grammar:\n";

    std::unordered_set<Lr0Item> items = AllItems();

    std::unordered_map<std::string, std::vector<Lr0Item>> grouped_items;
    for (const Lr0Item& item : items) {
        grouped_items[item.antecedent_].push_back(item);
    }

    for (const auto& [antecedent, item_list] : grouped_items) {
        std::cout << "Non-terminal: " << antecedent << "\n";
        for (const Lr0Item& item : item_list) {
            std::cout << "  - " << item.antecedent_ << " -> ";
            for (size_t i = 0; i < item.consequent_.size(); ++i) {
                if (i == item.dot_) {
                    std::cout << "• ";
                }
                std::cout << item.consequent_[i] << " ";
            }
            if (item.dot_ == item.consequent_.size()) {
                std::cout << "•";
            }
            std::cout << "\n";
        }
    }

    std::cout << "Total LR(0) items generated: " << items.size() << "\n";
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
            std::for_each(rules.begin(), rules.end(),
                          [&](const auto& rule) -> void {
                              newItems.insert({item.NextToDot(), rule,
                                               gr_.st_.EPSILON_, gr_.st_.EOL_});
                          });
            visited.insert(next);
        }
    }
    items.insert(newItems.begin(), newItems.end());
    if (size != items.size())
        ClosureUtil(items, items.size(), visited);
}

std::string SLR1Parser::TeachClosure(std::unordered_set<Lr0Item>& items) {
    std::ostringstream output;
    output << "Para el estado:\n";
    output << PrintItems(items);

    std::unordered_set<std::string> visited;
    TeachClosureUtil(items, items.size(), visited, 0, output);
    output << "Cierre:\n";
    for (const Lr0Item& item : items) {
        output << "  - ";
        output << item.ToString();
        output << "\n";
    }
    return output.str();
}

void SLR1Parser::TeachClosureUtil(std::unordered_set<Lr0Item>&     items,
                                  unsigned int                     size,
                                  std::unordered_set<std::string>& visited,
                                  int                              depth,
                                std::ostringstream &output) {
    // Indent based on depth for better readability
    std::string indent(depth * 2, ' ');

    std::unordered_set<Lr0Item> newItems;

    output << indent << "- Coge los ítems con un no terminal después del ·:\n";
    for (const auto& item : items) {
        std::string next = item.NextToDot();
        if (next == gr_.st_.EPSILON_ || gr_.st_.IsTerminal(next)) {
            continue;
        }

        output << indent << "  - Item: ";
        output << item.ToString();
        output << "\n";

        if (!gr_.st_.IsTerminal(next) && !visited.contains(next)) {
            output << indent << "    - Encontrado un no terminal: " << next << "\n";
            output << indent << "    - Añade todas las producciones de " << next
                   << " con el · al inicio:\n";

            const std::vector<production>& rules = gr_.g_.at(next);
            for (const auto& rule : rules) {
                Lr0Item newItem(next, rule, 0, gr_.st_.EPSILON_, gr_.st_.EOL_);
                newItems.insert(newItem);

                output << indent << "      - Añadido: ";
                output << newItem.ToString();
                output << "\n";
            }

            visited.insert(next);
        }
    }

    items.insert(newItems.begin(), newItems.end());

    if (size != items.size()) {
        output << indent << "- Se han añadido nuevos items. Repite el proceso.\n";
        TeachClosureUtil(items, items.size(), visited, depth + 1, output);
    } else {
        output << indent << "- No se han añadido nuevos items. El cierre está completo.\n";
    }
}

std::string SLR1Parser::TeachDeltaFunction(const std::unordered_set<Lr0Item>& items,
                                    const std::string&                 symbol) {
    if (symbol == gr_.st_.EPSILON_) {
        return "Sin importar el estado, δ(I,EPSILON) = ∅.\n";
    }
    std::ostringstream output;
    output << "Sea I:\n\n";
    output << PrintItems(items);
    output << "\nPara encontrar δ(I, " << symbol << "):\n";
    output << "1. Busca las reglas con " << symbol
              << " después del ·. Es decir, items de la forma α·" << symbol
              << "β\n";
    std::unordered_set<Lr0Item> filtered;
    std::for_each(items.begin(), items.end(), [&](const Lr0Item& item) -> void {
        std::string next = item.NextToDot();
        if (next == symbol) {
            filtered.insert(item);
        }
    });
    if (items.empty()) {
        output << "2. No hay items. Por tanto δ(I, " << symbol
                  << ") = ∅\n";
    } else {
        output << "2. Sea J:\n\n";
        output << PrintItems(filtered);
        output << "\n3. Avanza el · una posición:\n\n";
        std::unordered_set<Lr0Item> advanced;
        for (const Lr0Item& item : filtered) {
            Lr0Item new_item = item;
            new_item.AdvanceDot();
            advanced.insert(new_item);
        }
        output << PrintItems(advanced);
        output << "\n4. δ(I, " << symbol << ") = CLOSURE(J)\n";
        output << "5. Cierre de J:\n\n";
        Closure(advanced);
        output << PrintItems(advanced);
    }
    return output.str();
}

std::unordered_set<Lr0Item>
SLR1Parser::Delta(const std::unordered_set<Lr0Item>& items,
                  const std::string&                 str) {
    if (str == gr_.st_.EPSILON_) {
        return {};
    }
    std::vector<Lr0Item> filtered;
    std::for_each(items.begin(), items.end(), [&](const Lr0Item& item) -> void {
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

void SLR1Parser::TeachCanonicalCollection() {
    std::cout << "=== Process of Constructing the Canonical Collection of "
                 "LR(0) Items ===\n\n";

    Lr0Item      init(gr_.axiom_, gr_.g_.at(gr_.axiom_)[0], gr_.st_.EPSILON_,
                      gr_.st_.EOL_);
    unsigned int id = 0;
    std::unordered_set<state>   canonical_collection;
    std::unordered_set<state>   to_add;
    std::unordered_set<Lr0Item> current{init};

    std::cout << "=== Step 1: Initialize the Initial State ===\n";
    std::cout << "- Initial item: ";
    init.PrintItem();
    std::cout << "\n";
    std::cout << "- Closure:\n";
    Closure(current);
    PrintItems(current);

    state qi;
    qi.id_    = id++;
    qi.items_ = current;
    canonical_collection.insert(qi);

    std::unordered_set<state> visited;

    std::map<std::pair<unsigned int, std::string>, unsigned int> transitions;

    bool changed;
    do {
        changed = false;
        std::cout << "\n=== Step 2: Compute Transitions ===\n";

        for (const state& st : canonical_collection) {
            if (visited.find(st) != visited.end()) {
                continue;
            }

            std::cout << "- Processing state " << st.id_ << ":\n";
            std::cout << "  - Current set of items (I):\n";
            PrintItems(st.items_);

            std::cout << "  - For each grammar symbol X, compute δ(I, X):\n";

            for (const auto& [nt, _] : gr_.st_.st_) {
                if (nt == gr_.st_.EOL_ || nt == gr_.st_.EPSILON_) {
                    continue;
                }
                std::cout << "    > Computing δ(I, " << nt << "):\n";

                std::unordered_set<Lr0Item> delta_ret = Delta(st.items_, nt);

                if (delta_ret.empty()) {
                    std::cout << "      - δ(I, " << nt << ") = ∅\n";
                } else {
                    std::cout << "      - δ(I, " << nt << ") = {\n";
                    PrintItems(delta_ret);
                    std::cout << "      }\n";

                    qi.id_    = id;
                    qi.items_ = delta_ret;

                    if (visited.find(qi) != visited.end() ||
                        to_add.find(qi) != to_add.end() ||
                        canonical_collection.find(qi) !=
                            canonical_collection.end()) {
                        std::cout << "      * This set is already in the "
                                     "collection. Skipping.\n";
                        const auto& it = canonical_collection.find(qi);
                        transitions[{st.id_, nt}] = it->id_;
                    } else {
                        to_add.insert(qi);
                        transitions[{st.id_, nt}] = qi.id_;
                        std::cout << "      * This set is added to the "
                                     "collection as state "
                                  << id << ".\n";
                        id++;
                        changed = true;
                    }
                }
            }

            visited.insert(st);
        }

        canonical_collection.insert(to_add.begin(), to_add.end());
        to_add.clear();
    } while (changed);

    std::cout << "\n=== Canonical Collection Summary ===\n";
    std::cout << "- Total states: " << canonical_collection.size() << "\n";
    std::cout << "- States:\n";

    for (unsigned int i = 0; i < canonical_collection.size(); ++i) {
        const auto& current_st = std::find_if(
            canonical_collection.begin(), canonical_collection.end(),
            [i](const state& q) -> bool { return i == q.id_; });
        const state st = *current_st;
        std::cout << "  State " << i << ":\n";
        PrintItems(st.items_);
    }

    std::cout << "- Transitions:\n";
    for (const auto& [key, to_state] : transitions) {
        unsigned int from_state = key.first;
        std::string  symbol     = key.second;
        std::cout << "  State " << from_state << " -- " << symbol
                  << " --> State " << to_state << "\n";
    }
}

std::string SLR1Parser::PrintItems(const std::unordered_set<Lr0Item>& items) {
    std::ostringstream output;
    for (const auto& item : items) {
        output << "  - ";
        output << item.ToString();
        output << "\n";
    }
    return output.str();
}

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
