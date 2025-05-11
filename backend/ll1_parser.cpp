#include <algorithm>
#include <cstddef>
#include <iostream>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "grammar.hpp"
#include "ll1_parser.hpp"
#include "symbol_table.hpp"
#include "tabulate.hpp"

LL1Parser::LL1Parser(Grammar gr) : gr_(std::move(gr)) {
    ComputeFirstSets();
    ComputeFollowSets();
}

bool LL1Parser::CreateLL1Table() {
    if (first_sets_.empty() || follow_sets_.empty()) {
        ComputeFirstSets();
        ComputeFollowSets();
    }
    size_t nrows{gr_.g_.size()};
    ll1_t_.reserve(nrows);
    bool has_conflict{false};
    for (const auto& rule : gr_.g_) {
        std::unordered_map<std::string, std::vector<production>> column;
        for (const production& p : rule.second) {
            std::unordered_set<std::string> ds =
                PredictionSymbols(rule.first, p);
            column.reserve(ds.size());
            for (const std::string& symbol : ds) {
                auto& cell = column[symbol];
                if (!cell.empty()) {
                    has_conflict = true;
                }
                cell.push_back(p);
            }
        }
        ll1_t_.insert({rule.first, column});
    }
    return !has_conflict;
}

void LL1Parser::First(std::span<const std::string>     rule,
                      std::unordered_set<std::string>& result) {
    if (rule.empty() || (rule.size() == 1 && rule[0] == gr_.st_.EPSILON_)) {
        result.insert(gr_.st_.EPSILON_);
        return;
    }

    if (rule.size() > 1 && rule[0] == gr_.st_.EPSILON_) {
        First(std::span<const std::string>(rule.begin() + 1, rule.end()),
              result);
    } else {

        if (gr_.st_.IsTerminal(rule[0])) {
            // EOL cannot be in first sets, if we reach EOL it means that the
            // axiom is nullable, so epsilon is included instead
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
        First(std::span<const std::string>(rule.begin() + 1, rule.end()),
              result);
    }
}

// Least fixed point
void LL1Parser::ComputeFirstSets() {
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

void LL1Parser::ComputeFollowSets() {
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

std::unordered_set<std::string> LL1Parser::Follow(const std::string& arg) {
    if (follow_sets_.find(arg) == follow_sets_.end()) {
        return {};
    }
    return follow_sets_.at(arg);
}

std::unordered_set<std::string>
LL1Parser::PredictionSymbols(const std::string&              antecedent,
                             const std::vector<std::string>& consequent) {
    std::unordered_set<std::string> hd{};
    First({consequent}, hd);
    if (hd.find(gr_.st_.EPSILON_) == hd.end()) {
        return hd;
    }
    hd.erase(gr_.st_.EPSILON_);
    hd.merge(Follow(antecedent));
    return hd;
}

std::string LL1Parser::TeachFirst(const std::vector<std::string>& symbols) {
    std::ostringstream output;

    output << "El conjunto CABECERA de una cadena de símbolos es el conjunto\n";
    output << "de terminales que pueden aparecer como primer símbolo en\n";
    output << "alguna derivación desde esa cadena.\n\n";
    output << "Reglas generales:\n";
    output << "  1. Si el primer símbolo es terminal, CABECERA es ese símbolo.\n";
    output << "  2. Si es no terminal, explora sus producciones.\n";
    output << "  3. Si alguna producción contiene ε (cadena vacía),\n";
    output << "     se sigue con el siguiente símbolo.\n";

    output << "Calcular CAB(";
    for (const std::string& symbol : symbols) {
        output << symbol;
    }
    output << "):\n";

    std::unordered_set<std::string> first_set;
    std::unordered_set<std::string>
        processing; // Track non-terminals being processed
    TeachFirstUtil(symbols, first_set, 0, processing, output);

    output << "\n======== RESULTADO FINAL ========\n";
    output << "FIRST(";
    for (const auto &symbol : symbols)
        output << symbol;
    output << ") = { ";
    for (const auto &symbol : first_set)
        output << symbol << " ";
    output << "}\n";
    return output.str();
    return output.str();
}

void LL1Parser::TeachFirstUtil(const std::vector<std::string> &symbols,
                               std::unordered_set<std::string> &first_set,
                               int depth,
                               std::unordered_set<std::string> &processing,
                               std::ostringstream &output)
{
    if (symbols.empty())
        return;

    std::string indent(depth * 2, ' ');
    std::string current_symbol = symbols[0];
    std::vector<std::string> remaining_symbols(symbols.begin() + 1, symbols.end());

    output << indent << "Paso " << depth + 1 << ": Analizando símbolo '" << current_symbol << "'\n";

    // Caso 1: Terminal
    if (gr_.st_.IsTerminal(current_symbol)) {
        output << indent << "  - Es un terminal.\n";
        output << indent << "  - Se agrega directamente al conjunto CABECERA.\n";

        std::vector<std::string> full_chain{current_symbol};
        full_chain.insert(full_chain.end(), remaining_symbols.begin(), remaining_symbols.end());
        std::unordered_set<std::string> partial_first;
        First(std::span<const std::string>(full_chain.begin(), full_chain.end()), partial_first);
        first_set.insert(partial_first.begin(), partial_first.end());

        return;
    }

    // Caso 2: No terminal
    output << indent << "  - Es un no terminal. Explorando sus producciones.\n";

    // Evitar ciclos
    if (processing.find(current_symbol) != processing.end()) {
        output << indent << "  - Ya estamos procesando '" << current_symbol
               << "'. Evitamos ciclo.\n";
        return;
    }
    processing.insert(current_symbol);

    const auto &productions = gr_.g_.at(current_symbol);
    for (const auto &prod : productions) {
        output << indent << "  - Aplicando producción: " << current_symbol << " → ";
        for (const auto &s : prod)
            output << s << " ";
        output << "\n";

        // Derivación recursiva
        std::vector<std::string> new_symbols = prod;
        new_symbols.insert(new_symbols.end(), remaining_symbols.begin(), remaining_symbols.end());
        TeachFirstUtil(new_symbols, first_set, depth + 1, processing, output);

        // Verificar si hay ε en la producción
        bool has_epsilon = std::find(prod.begin(), prod.end(), gr_.st_.EPSILON_) != prod.end();
        if (has_epsilon) {
            output << indent << "  - Esta producción contiene ε (cadena vacía).\n";
            output << indent << "    → Continua con los símbolos restantes: ";
            for (const auto &s : remaining_symbols)
                output << s << " ";
            output << "\n";
            TeachFirstUtil(remaining_symbols, first_set, depth + 1, processing, output);
        }
    }

    // Desmarcar el símbolo como procesado
    processing.erase(current_symbol);

    // Mostrar FIRST parcial
    output << indent << "  - Conjunto CABECERA parcial tras procesar '" << current_symbol
           << "': { ";
    for (const auto &s : first_set)
        output << s << " ";
    output << "}\n";
}

std::string LL1Parser::TeachFollow(const std::string& non_terminal) {
    std::ostringstream output;
    output << "Encontrar los símbolos siguientes a " << non_terminal
              << ":\n";

    if (non_terminal == gr_.axiom_) {
        output << "Como " << non_terminal << " es el axioma, SIG("
                  << non_terminal << ") = { " << gr_.st_.EOL_ << " }\n";
        return output.str();
    }
    // Step 1: Find all rules where the non-terminal appears in the consequent
    std::vector<std::pair<std::string, production>> rules_with_nt;
    for (const auto& [antecedent, productions] : gr_.g_) {
        for (const auto& prod : productions) {
            auto it = std::find(prod.begin(), prod.end(), non_terminal);
            if (it != prod.end()) {
                rules_with_nt.emplace_back(antecedent, prod);
            }
        }
    }

    if (rules_with_nt.empty()) {
        output << "1. " << non_terminal
                  << " no aparece en ningún consecuente.\n";
        return output.str();
    }

    output << "1. Busca las reglas donde " << non_terminal
              << " está en el consecuente:\n";
    for (const auto& [antecedent, prod] : rules_with_nt) {
        output << "   - " << antecedent << " -> ";
        for (const std::string& symbol : prod) {
            output << symbol << " ";
        }
        output << "\n";
    }

    // Step 2: Compute Follow for each occurrence of the non-terminal in the
    // rules
    std::unordered_set<std::string> follow_set;
    for (const auto& [antecedent, prod] : rules_with_nt) {
        for (auto it = prod.begin(); it != prod.end(); ++it) {
            if (*it == non_terminal) {
                // Case 1: Non-terminal is not at the end of the production
                if (std::next(it) != prod.end()) {
                    std::vector<std::string> remaining_symbols(std::next(it),
                                                               prod.end());
                    std::unordered_set<std::string> first_of_remaining;
                    First(remaining_symbols, first_of_remaining);

                    output << "2. Calcula la cabecera de la subcadena después de "
                              << non_terminal << ": { ";
                    for (const std::string& symbol : remaining_symbols) {
                        output << symbol << " ";
                    }
                    output << "} = { ";
                    for (const std::string& symbol : first_of_remaining) {
                        output << symbol << " ";
                    }
                    output << "}\n";

                    // Add First(remaining_symbols) to Follow(non_terminal)
                    for (const std::string& symbol : first_of_remaining) {
                        if (symbol != gr_.st_.EPSILON_) {
                            follow_set.insert(symbol);
                        }
                    }

                    // If ε ∈ First(remaining_symbols), add Follow(antecedent)
                    if (first_of_remaining.find(gr_.st_.EPSILON_) !=
                        first_of_remaining.end()) {
                        output << "   - Como ε ∈ CAB, agrega SIG("
                                  << antecedent << ") = { ";
                        std::unordered_set<std::string> ant_follow(
                            Follow(antecedent));
                        for (const std::string& str : ant_follow) {
                            output << str << " ";
                        }
                        output << "} a SIG(" << non_terminal << ")\n";
                        follow_set.insert(ant_follow.begin(), ant_follow.end());
                    }
                }
                // Case 2: Non-terminal is at the end of the production
                else {
                    output << "2. " << non_terminal
                              << " está al final de la producción. Agrega SIG("
                              << antecedent << ") = { ";
                    std::unordered_set<std::string> ant_follow(
                        Follow(antecedent));
                    for (const std::string& str : ant_follow) {
                        output << str << " ";
                    }

                    output << "} a SIG(" << non_terminal << ")\n";
                    follow_set.insert(ant_follow.begin(), ant_follow.end());
                }
            }
        }
    }

    // Step 3: Display the final Follow set
    output << "3. Conjunto SIG(" << non_terminal << ") = { ";
    for (const std::string& symbol : follow_set) {
        output << symbol << " ";
    }
    output << "}\n";
    return output.str();
}

std::string LL1Parser::TeachPredictionSymbols(const std::string& antecedent,
                                       const production&  consequent) {
    std::ostringstream output;
    // Convert the consequent to a string for display purposes
    std::string consequent_str;
    for (const std::string& symbol : consequent) {
        consequent_str += symbol + " ";
    }
    if (!consequent_str.empty()) {
        consequent_str.pop_back(); // Remove the trailing space
    }

    output << "Encontrar los símbolos directores de: "
              << antecedent << " -> " << consequent_str << ":\n";

    // Step 1: Compute First(consequent)
    std::unordered_set<std::string> first_of_consequent;
    First(consequent, first_of_consequent);

    output << "1. Calcula CAB(" << consequent_str << ") = { ";
    for (const std::string& symbol : first_of_consequent) {
        output << symbol << " ";
    }
    output << "}\n";

    // Step 2: Initialize prediction symbols with First(consequent) excluding ε
    std::unordered_set<std::string> prediction_symbols;
    for (const std::string& symbol : first_of_consequent) {
        if (symbol != gr_.st_.EPSILON_) {
            prediction_symbols.insert(symbol);
        }
    }

    output << "2. Inicializa los símbolos directores con CAB("
              << consequent_str << ") excepto ε: { ";
    for (const std::string& symbol : prediction_symbols) {
        output << symbol << " ";
    }
    output << "}\n";

    // Step 3: If ε ∈ First(consequent), add Follow(antecedent) to prediction
    // symbols
    if (first_of_consequent.find(gr_.st_.EPSILON_) !=
        first_of_consequent.end()) {
        output << "  - Como ε ∈ CAB(" << consequent_str
                  << "), agrega SIG(" << antecedent
                  << ") a los símbolos directores.\n";
        const auto& follow_antecedent = Follow(antecedent);
        prediction_symbols.insert(follow_antecedent.begin(),
                                  follow_antecedent.end());

        output << "    SIG(" << antecedent << ") = { ";
        for (const std::string& symbol : follow_antecedent) {
            output << symbol << " ";
        }
        output << "}\n";
    }

    // Step 4: Display the final prediction symbols
    output << "3. Entonces, los símbolos directores de " << antecedent << " -> "
              << consequent_str << " son: { ";
    for (const std::string& symbol : prediction_symbols) {
        output << symbol << " ";
    }
    output << "}\n";
    return output.str();
}

std::string LL1Parser::TeachLL1Table() {
    std::ostringstream output;
    output << "1. Proceso para construir la tabla LL(1):\n";
    output << "La tabla LL(1) se construye definiendo todos los símbolos directores para cada regla.\n";
    size_t i = 1;
    for (const auto& [nt, prods] : gr_.g_) {
        for (const production& prod : prods) {
            std::unordered_set<std::string> pred;
            pred = PredictionSymbols(nt, prod);
            output << "  " << i + ". SD( " << nt << " -> ";
            for (const std::string& symbol : prod) {
                output << symbol << " ";
            }
            output << ") = { ";
            for (const std::string& symbol : pred) {
                output << symbol << " ";
            }
            output << "}\n";
        }
    }
    output
        << "2. Una gramática cumple la condición LL(1) si para cada no terminal, ninguna de "
           "sus producciones tienen símbolos directores en común.\nEs decir, para "
           "cada regla A -> X y A -> Y, SD(A -> X) ∩ SD(A -> Y) = ∅\n";
    bool has_conflicts = false;
    for (const auto& [nt, cols] : ll1_t_) {
        for (const auto& col : cols) {
            if (col.second.size() > 1) {
                has_conflicts                       = true;
                const std::vector<production> prods = col.second;
                output << "- Conflicto en " << col.first << ":\n";
                for (const production& prod : prods) {
                    output << "  SD( " << nt << " -> ";
                    for (const std::string& symbol : prod) {
                        output << symbol << " ";
                    }
                    output << ")\n";
                }
            }
        }
    }
    if (!has_conflicts) {
        output << "3. Los conjuntos de símbolos directores no se solapan. La gramática es "
                     "LL(1). La tabla LL(1) se construye de la siguiente forma.\n";
        output << "4. Ten una fila por cada símbolo no terminal ("
                  << gr_.st_.non_terminals_.size()
                  << " filas), y una columna por cada terminal más "
                  << gr_.st_.EOL_ << " (" << gr_.st_.terminals_.size()
                  << " columnas).\n";
        output
            << "5. Coloca α en la celda (A,β) si β ∈ SD(A ->α), déjala vacía en otro caso.\n";
        for (const auto& [nt, cols] : ll1_t_) {
            for (const auto& col : cols) {
                output << "  - ll1(" << nt << ", " << col.first << ") = ";
                for (const std::string& symbol : col.second.at(0)) {
                    output << symbol << " ";
                }
                output << "\n";
            }
        }
    } else {
        output << "3. Como al menos dos conjuntos se solapan "
                     "con el mismo terminal, la gramática no es LL(1).\n";
    }
    return output.str();
}

void LL1Parser::PrintTable() {
    using namespace tabulate;
    Table table;

    Table::Row_t                          headers = {"Non-terminal"};
    std::unordered_map<std::string, bool> columns;

    for (const auto& outerPair : ll1_t_) {
        for (const auto& innerPair : outerPair.second) {
            columns[innerPair.first] = true;
        }
    }

    for (const auto& col : columns) {
        headers.push_back(col.first);
    }

    auto& header_row = table.add_row(headers);
    header_row.format()
        .font_align(FontAlign::center)
        .font_color(Color::yellow)
        .font_style({FontStyle::bold});

    std::vector<std::string> non_terminals;
    for (const auto& outerPair : ll1_t_) {
        non_terminals.push_back(outerPair.first);
    }

    std::sort(non_terminals.begin(), non_terminals.end(),
              [this](const std::string& a, const std::string& b) {
                  if (a == gr_.axiom_)
                      return true; // Axiom comes first
                  if (b == gr_.axiom_)
                      return false; // Axiom comes first
                  return a < b;     // Sort the rest alphabetically
              });

    for (const std::string& nonTerminal : non_terminals) {
        Table::Row_t row_data = {nonTerminal};

        for (const auto& col : columns) {
            auto innerIt = ll1_t_.at(nonTerminal).find(col.first);
            if (innerIt != ll1_t_.at(nonTerminal).end()) {
                std::string cell_content;
                for (const auto& prod : innerIt->second) {
                    cell_content += "[ ";
                    for (const std::string& elem : prod) {
                        cell_content += elem + " ";
                    }
                    cell_content += "] ";
                }
                row_data.push_back(cell_content);
            } else {
                row_data.push_back("-");
            }
        }

        table.add_row(row_data);
    }

    table[0].format().font_color(Color::cyan).font_style({FontStyle::bold});
    for (size_t i = 1; i < table.size(); ++i) {
        for (size_t j = 1; j < table[i].size(); ++j) {
            if (table[i][j].get_text().find("] [") != std::string::npos) {
                table[i][j].format().font_color(Color::red);
            }
        }
    }
    table.format().font_align(FontAlign::center);
    table.column(0).format().font_color(Color::cyan);

    // Print the table
    std::cout << table << std::endl;
}
