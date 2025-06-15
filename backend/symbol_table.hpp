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
 * @brief Stores and manages grammar symbols, including their classification and special markers.
 *
 * This structure holds information about all terminals and non-terminals used
 * in a grammar, as well as special symbols such as EPSILON and the end-of-line marker ($).
 * It supports symbol classification, membership checks, and filtered views
 * such as terminals excluding $.
 */
struct SymbolTable {
    /// @brief End-of-line symbol used in parsing, initialized as "$".
    std::string EOL_{"$"};

    /// @brief Epsilon symbol, representing empty transitions, initialized as
    /// "EPSILON".
    std::string EPSILON_{"EPSILON"};

    /// @brief Main symbol table, mapping identifiers to a pair of symbol type
    /// and its regex.
    std::unordered_map<std::string, symbol_type> st_{{EOL_, symbol_type::TERMINAL},
                                                     {EPSILON_, symbol_type::TERMINAL}};

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
