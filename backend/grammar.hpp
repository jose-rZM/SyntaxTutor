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
#include "symbol_table.hpp"
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @typedef production
 * @brief Represents the right-hand side of a grammar rule.
 *
 * A production is a sequence of grammar symbols (terminals or non-terminals)
 * that can be derived from a non-terminal symbol in the grammar.
 *
 * For example, in the rule A → a B c, the production would be:
 * {"a", "B", "c"}
 */
using production = std::vector<std::string>;

/**
 * @struct Grammar
 * @brief Represents a context-free grammar, including its rules, symbol table, and starting symbol.
 *
 * This structure encapsulates all components required to define and manipulate a grammar,
 * including production rules, the associated symbol table, and metadata such as the start symbol.
 * It supports construction, transformation, and analysis of grammars.
 */
struct Grammar {

    Grammar();
    explicit Grammar(
        const std::unordered_map<std::string, std::vector<production>>&
            grammar);


    /**
     * @brief Sets the axiom (entry point) of the grammar.
     *
     * @param axiom The entry point or start symbol of the grammar.
     *
     * Defines the starting point for the grammar, which is used in parsing
     * algorithms and must be a non-terminal symbol present in the grammar.
     */
    void SetAxiom(const std::string& axiom);

    /**
     * @brief Checks if a given antecedent has an empty production.
     *
     * @param antecedent The left-hand side (LHS) symbol to check.
     * @return true if there exists an empty production for the antecedent,
     *         otherwise false.
     *
     * An empty production is represented as `<antecedent> -> ;`, indicating
     * that the antecedent can produce an empty string.
     */
    bool HasEmptyProduction(const std::string& antecedent) const;

    /**
     * @brief Filters grammar rules that contain a specific token in their
     * consequent.
     *
     * @param arg The token to search for within the consequents of the rules.
     * @return std::vector of pairs where each pair contains an antecedent and
     * its respective production that includes the specified token.
     *
     * Searches for rules in which the specified token is part of the consequent
     * and returns those rules.
     */
    std::vector<std::pair<const std::string, production>> FilterRulesByConsequent(
        const std::string& arg) const;

    /**
     * @brief Prints the current grammar structure to standard output.
     *
     * This function provides a debug view of the grammar by printing out all
     * rules, the axiom, and other relevant details.
     */
    void Debug() const; //NOSONAR

    /**
    * @brief Adds a production rule to the grammar and updates the symbol table.
    *
    * This function inserts a new production of the form A → α into the grammar,
    * where `antecedent` is the non-terminal A and `consequent` is the sequence α.
    * It also updates the internal symbol table to reflect any new symbols introduced.
    *
    * @param antecedent The left-hand side non-terminal of the production.
    * @param consequent The right-hand side sequence of grammar symbols.
    */
    void AddProduction(const std::string&              antecedent,
                       const std::vector<std::string>& consequent);

    /**
    * @brief Splits a string into grammar symbols using the current symbol table.
    *
    * This function tokenizes the input string `s` into a sequence of grammar symbols
    * based on the known entries in the symbol table. It uses a greedy approach,
    * matching the longest valid symbol at each step.
    *
    * @param s The input string to split.
    * @return A vector of grammar symbols extracted from the string.
    */
    std::vector<std::string> Split(const std::string& s);

    /**
     * @brief Stores the grammar rules with each antecedent mapped to a list of
     * productions.
     */
    std::unordered_map<std::string, std::vector<production>> g_;

    /**
     * @brief The axiom or entry point of the grammar.
     */
    std::string axiom_;

    /**
     * @brief Symbol table of the grammar.
     */
    SymbolTable st_;
};
