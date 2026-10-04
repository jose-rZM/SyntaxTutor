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
#include <string>
#include <vector>

/**
 * @struct GrammarParseError
 * @brief Describes one problem found while parsing user-written grammar text.
 *
 * Errors carry a machine-readable kind, the 1-based line where the offending
 * rule starts, and the offending fragment so the GUI can build a localized,
 * user-friendly message.
 */
struct GrammarParseError {
    /**
     * @enum Kind
     * @brief Machine-readable category of a grammar text error.
     */
    enum class Kind {
        /// @brief The text contains no rules at all.
        EmptyGrammar,
        /// @brief A rule has no "->" separator.
        MissingArrow,
        /// @brief Text after the last "." looks like an unterminated rule.
        MissingEndDot,
        /// @brief A rule has no symbol before "->".
        EmptyLeftHandSide,
        /// @brief A rule has more than one symbol before "->".
        MultipleLeftHandSide,
        /// @brief A rule body contains another "->", usually a missing dot.
        ExtraArrow,
        /// @brief A symbol uses a reserved character or reserved word.
        ReservedSymbol,
    };

    /// @brief Category of the error.
    Kind kind;

    /// @brief 1-based line in the source text where the rule starts.
    int line;

    /// @brief Offending token or rule fragment, for diagnostics.
    std::string detail;
};

/**
 * @struct GrammarParseResult
 * @brief Outcome of parsing user-written grammar text.
 *
 * When `errors` is empty, `grammar` holds a fully-built grammar: symbols are
 * classified (non-terminals are exactly the left-hand sides), the axiom is
 * the left-hand side of the first rule, and the grammar has been augmented
 * with a fresh axiom rule `X -> <axiom> $` following the app-wide convention.
 */
struct GrammarParseResult {
    /// @brief Resulting grammar. Only meaningful when `errors` is empty.
    Grammar grammar;

    /// @brief All problems found in the text, in source order.
    std::vector<GrammarParseError> errors;

    /// @brief True when parsing succeeded and `grammar` is usable.
    bool Ok() const { return errors.empty(); }
};

/**
 * @struct GrammarParser
 * @brief Parses user-written grammar text into a Grammar object.
 *
 * Input format, kept deliberately flexible:
 *  - Each rule is `LHS -> SYMBOL SYMBOL ... .` and ends with a dot. Rules
 *    may span several lines and several rules may share one line.
 *  - Symbols are separated by whitespace; any whitespace-free string is a
 *    valid symbol, so multi-character tokens such as `id` or `function`
 *    are supported. `A b` is two symbols while `Ab` is one.
 *  - "→" is accepted as an alias of "->", with or without surrounding
 *    spaces.
 *  - Alternatives can be grouped with `|`: `A -> a | b .`
 *  - An empty right-hand side (`A -> .` or an empty `|` branch) denotes an
 *    epsilon production.
 *  - Non-terminals are exactly the symbols that appear on a left-hand
 *    side; every other symbol is a terminal. The first rule defines the
 *    axiom.
 */
struct GrammarParser {
    /**
     * @brief Parses grammar text into an augmented Grammar.
     *
     * @param text User-written grammar description.
     * @return Parse result with either a ready-to-use grammar or the list
     *         of errors found.
     */
    static GrammarParseResult Parse(const std::string& text);

    /**
     * @brief Checks whether a token cannot be used as a grammar symbol.
     *
     * Reserved tokens are the meta symbols ($, EPSILON, ε, ·) and any token
     * containing characters used by the tutors as separators or markers
     * (dot, comma, colon, semicolon, pipe or an embedded arrow).
     *
     * @param token Token to check.
     * @return true if the token is reserved and must be rejected.
     */
    static bool IsReservedToken(const std::string& token);
};
