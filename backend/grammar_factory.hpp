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
#include "grammar.hpp"
#include "symbol_table.hpp"
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @struct GrammarFactory
 * @brief Responsible for creating and managing grammar items and performing
 * checks on grammars.
 */
struct GrammarFactory {

    /**
     * @struct FactoryItem
     * @brief Represents an individual grammar item with its associated symbol
     * table.
     */
    struct FactoryItem {
        /**
         * @brief Stores the grammar rules where each key is a non-terminal
         * symbol and each value is a vector of production rules.
         */
        std::unordered_map<std::string, std::vector<production>> g_;

        /**
         * @brief Symbol table associated with this grammar item.
         */
        SymbolTable st_;

        /**
         * @brief Constructor that initializes a FactoryItem with the provided
         * grammar.
         * @param grammar The grammar to initialize the FactoryItem with.
         */
        explicit FactoryItem(
            const std::unordered_map<std::string, std::vector<production>>&
                grammar);
    };

    /**
     * @brief Initializes the GrammarFactory and populates the items vector with
     * initial grammar items.
     */
    void Init();

    /**
     * @brief Picks a random grammar based on the specified difficulty level (1,
     * 2, or 3).
     * @param level The difficulty level (1, 2, or 3).
     * @return A randomly picked grammar.
     */
    Grammar PickOne(int level);

    /**
     * @brief Generates a LL(1) random grammar based on the specified difficulty
     * level.
     * @param level The difficulty level (1, 2, or 3)
     * @return A random LL(1) grammar.
     */
    Grammar GenLL1Grammar(int level);
    /**
     * @brief Generates a SLR(1) random grammar based on the specified
     * difficulty lefel.
     * @param level The difficulty level (1, 2, or 3)
     * @return A random SLR(1) grammar.
     */
    Grammar GenSLR1Grammar(int level);

    /**
     * @brief Generates a Level 1 grammar.
     * @return A Level 1 grammar.
     */
    Grammar Lv1();

    /**
     * @brief Generates a Level 2 grammar by combining Level 1 items.
     * @return A Level 2 grammar.
     */
    Grammar Lv2();

    /**
     * @brief Generates a Level 3 grammar by combining a Level 2 item and a
     * Level 1 item.
     * @return A Level 3 grammar.
     */
    Grammar Lv3();

    /**
     * @brief Generates a Level 4 grammar by combining Level 3 and Level 1
     * items.
     *
     * This function creates a more complex grammar by combining elements from
     * Level 3 and Level 1 grammars. It is used to generate grammars with
     * increased complexity for testing or parsing purposes.
     *
     * @return A Level 4 grammar.
     */
    Grammar Lv4();

    /**
     * @brief Generates a Level 5 grammar by combining Level 4 and Level 1
     * items.
     *
     * This function creates a more advanced grammar by combining elements from
     * Level 4 and Level 1 grammars. It is used to generate grammars with
     * higher complexity for testing or parsing purposes.
     *
     * @return A Level 5 grammar.
     */
    Grammar Lv5();

    /**
     * @brief Generates a Level 6 grammar by combining Level 5 and Level 1
     * items.
     *
     * This function creates a highly complex grammar by combining elements from
     * Level 5 and Level 1 grammars. It is used to generate grammars with
     * advanced structures for testing or parsing purposes.
     *
     * @return A Level 6 grammar.
     */
    Grammar Lv6();

    /**
     * @brief Generates a Level 7 grammar by combining Level 6 and Level 1
     * items.
     *
     * This function creates a very complex grammar by combining elements from
     * Level 6 and Level 1 grammars. It is used to generate grammars with
     * highly advanced structures for testing or parsing purposes.
     *
     * @return A Level 7 grammar.
     */
    Grammar Lv7();

    /**
     * @brief Creates a Level 2 grammar item for use in grammar generation.
     *
     * This function generates a Level 2 grammar item, which can be used as a
     * building block for creating more complex grammars.
     *
     * @return A FactoryItem representing a Level 2 grammar.
     */
    FactoryItem CreateLv2Item();

    /**
     * @brief Checks if a grammar contains unreachable symbols (non-terminals
     * that cannot be derived from the start symbol).
     * @param grammar The grammar to check.
     * @return true if there are unreachable symbols, false otherwise.
     */
    bool HasUnreachableSymbols(Grammar& grammar) const;

    /**
     * @brief Checks if a grammar is infinite, meaning there are non-terminal
     * symbols that can never derive a terminal string. This happens when a
     * production leads to an infinite recursion or an endless derivation
     * without reaching terminal symbols.
     * For example, a production like:
     * \code
     * S -> A
     * A -> a A | B
     * B -> c B
     * \endcode
     * could lead to an infinite derivation of non-terminals.
     * @param grammar The grammar to check.
     * @return true if the grammar has infinite derivations, false otherwise.
     */
    bool IsInfinite(Grammar& grammar) const;

    /**
     * @brief Checks if a grammar contains direct left recursion (a non-terminal
     * can produce itself on the left side of a production in one step).
     * @param grammar The grammar to check.
     * @return true if there is direct left recursion, false otherwise.
     */
    bool HasDirectLeftRecursion(const Grammar& grammar) const;

    /**
     * @brief Checks if a grammar contains indirect left recursion.
     * @param grammar The grammar to check.
     * @return true if there is direct left recursion, false otherwise.
     */
    bool HasIndirectLeftRecursion(const Grammar& grammar) const;

    /**
     * @brief Checks if directed graph has a cycle using topological sort.
     * @param graph The directed graph.
     * @return true if grammar has cycle.
     */
    bool HasCycle(
        const std::unordered_map<std::string, std::unordered_set<std::string>>&
            graph) const;

    /**
     * @brief Find nullable symbols in a grammar.
     * @param grammar The grammar to check.
     * @return set of nullable symbols.
     */
    std::unordered_set<std::string>
    NullableSymbols(const Grammar& grammar) const;

    /**
     * @brief Removes direct left recursion in a grammar.
     * A grammar has direct left recursion when one of its productions is
     * \code
     * A -> A a
     * \endcode
     * where A is a non-terminal symbol and "a" the rest of the production.
     * The procedure removes direct left recursion by adding a new
     * non-terminal. So, if the productions with left recursion are:
     * \code
     * A -> A a | b
     * \endcode
     * the result would be:
     * \code
     * A  -> b A'
     * A' -> a A' | epsilon
     * \endcode
     * @param grammar The grammar to remove left recursion.
     */

    void RemoveLeftRecursion(Grammar& grammar);

    /**
     * @brief Performs left factorization.
     * A grammar can be left factorized if it has productions with the
     * same prefix for one non-terminal. For example:
     * \code
     * A -> a x | a y
     * \endcode
     * could be left factorized because it has "a" as the common prefix.
     * The left factorization is done by adding a new non-terminal symbol
     * that contains the uncommon part, and by unifying the common prefix in one
     * production. So:
     * \code
     * A -> a x | a y
     * \endcode
     * would become:
     * \code
     * A  -> a A'
     * A' -> x | y
     * \endcode
     * @param grammar The grammar to be left factorized.
     */

    void LeftFactorize(Grammar& grammar);

    /**
     * @brief Finds the longest common prefix among a set of productions.
     *
     * This function computes the longest sequence of symbols that is common to
     * the beginning of all productions in the given vector. It is used during
     * left factorization to identify common prefixes that can be factored out.
     *
     * @param productions A vector of productions to analyze.
     * @return A vector of strings representing the longest common prefix. If no
     *         common prefix exists, an empty vector is returned.
     */
    std::vector<std::string>
    LongestCommonPrefix(const std::vector<production>& productions);

    /**
     * @brief Checks if a production starts with a given prefix.
     *
     * This function determines whether the symbols in a production match the
     * provided prefix sequence at the beginning. It is used during left
     * factorization to identify productions that share a common prefix.
     *
     * @param prod The production to check.
     * @param prefix The sequence of symbols to compare against the beginning of
     *               the production.
     * @return `true` if the production starts with the prefix, `false`
     * otherwise.
     */
    bool StartsWith(const production&               prod,
                    const std::vector<std::string>& prefix);

    /**
     * @brief Generates a new non-terminal symbol that is unique in the grammar.
     *
     * This function creates a new non-terminal symbol by appending a prime
     * symbol (') to the base name until the resulting symbol is not already
     * present in the grammar's symbol table. It is used during left
     * factorization to introduce new non-terminals for factored productions.
     *
     * @param grammar The grammar in which the new non-terminal will be added.
     * @param base The base name for the new non-terminal.
     * @return A unique non-terminal symbol derived from the base name.
     */
    std::string GenerateNewNonTerminal(Grammar&           grammar,
                                       const std::string& base);

    /**
     * @brief Replaces all non-terminal symbols in a grammar item with a single
     * target non-terminal.
     *
     * This function is used during grammar combination to normalize the
     * non-terminal symbols in a given FactoryItem, so that they are consistent
     * and compatible with another item.
     *
     * @param item The grammar item whose non-terminals will be renamed.
     * @param nt The new non-terminal symbol that will replace all existing
     * ones.
     */
    void NormalizeNonTerminals(FactoryItem& item, const std::string& nt) const;

    /**
     * @brief Adjusts the terminal symbols between two grammar items.
     *
     * This function modifies the terminal symbols of a base grammar item so
     * that they do not conflict with those of the item being combined. It also
     * renames terminals to ensure consistency and inserts the target
     * non-terminal where appropriate.
     *
     * @param base The base grammar item to adjust.
     * @param cmb The grammar item being combined with the base.
     * @param target_nt The target non-terminal symbol used for replacement.
     */
    void AdjustTerminals(FactoryItem& base, const FactoryItem& cmb,
                         const std::string& target_nt) const;

    /**
     * @brief Merges the grammar rules of two grammar items into a single
     * grammar.
     *
     * This function performs a raw combination of the production rules from
     * both grammar items, resulting in a single grammar map that contains all
     * productions.
     *
     * @param base The first grammar item.
     * @param cmb The second grammar item.
     * @return A merged grammar map containing all production rules from both
     * inputs.
     */
    std::unordered_map<std::string, std::vector<production>>
    Merge(const FactoryItem& base, const FactoryItem& cmb) const;

    /**
     * @brief A vector of FactoryItem objects representing different level 1
     * grammar items created by the Init method.
     */
    std::vector<FactoryItem> items;

    /**
     * @brief A vector of terminal symbols (alphabet) used in the grammar.
     */
    std::vector<std::string> terminal_alphabet_{"a", "b", "c", "d", "e", "f",
                                                "g", "h", "i", "j", "k", "l"};

    /**
     * @brief A vector of non-terminal symbols (alphabet) used in the grammar.
     */
    std::vector<std::string> non_terminal_alphabet_{"A", "B", "C", "D",
                                                    "E", "F", "G"};
};
