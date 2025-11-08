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

#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

/**
 * @enum symbol_type
 * @brief Represents the type of a grammar symbol.
 *
 * This enum distinguishes between terminal and non-terminal symbols
 * within the grammar and the symbol table.
 */
enum class symbol_type { NO_TERMINAL, TERMINAL };

/**
 * @struct SymbolTable
 * @brief Stores and manages grammar symbols, including their classification and
 * special markers.
 *
 * This structure holds information about all terminals and non-terminals used
 * in a grammar, as well as special symbols such as EPSILON and the end-of-line
 * marker ($). It supports symbol classification, membership checks, and
 * filtered views such as terminals excluding $.
 */
struct SymbolTable {
    /// @brief End-of-line symbol used in parsing, initialized as "$".
    std::string EOL_{"$"};

    /// @brief Epsilon symbol, representing empty transitions, initialized as
    /// "EPSILON".
    std::string EPSILON_{"EPSILON"};

    /// @brief Main symbol table, mapping identifiers to a pair of symbol type
    /// and its regex.
    std::unordered_map<std::string, symbol_type> st_{
        {EOL_, symbol_type::TERMINAL}, {EPSILON_, symbol_type::TERMINAL}};

    /**
     * @brief Set of all terminal symbols (including EOL).
     */
    std::unordered_set<std::string> terminals_{EOL_};

    /**
     * @brief Set of terminal symbols excluding the EOL symbol ($).
     */
    std::unordered_set<std::string> terminals_wtho_eol_{};

    /**
     * @brief Set of all non-terminal symbols.
     */
    std::unordered_set<std::string> non_terminals_;

    /**
     * @brief Adds a non-terminal symbol to the symbol table.
     *
     * @param identifier Name of the  symbol.
     * @param isTerminal True if the identifier is a terminal symbol
     */
    void PutSymbol(const std::string& identifier, bool isTerminal);

    /**
     * @brief Checks if a symbol exists in the symbol table.
     *
     * @param s Symbol identifier to search.
     * @return true if the symbol is present, otherwise false.
     */
    bool In(const std::string& s) const;

    /**
     * @brief Checks if a symbol is a terminal.
     *
     * @param s Symbol identifier to check.
     * @return true if the symbol is terminal, otherwise false.
     */
    bool IsTerminal(const std::string& s) const;

    /**
     * @brief Checks if a symbol is a terminal excluding EOL.
     *
     * @param s Symbol identifier to check.
     * @return true if the symbol is terminal, otherwise false.
     */
    bool IsTerminalWthoEol(const std::string& s) const;
};
