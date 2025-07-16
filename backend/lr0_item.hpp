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
#include <vector>

/**
 * @struct Lr0Item
 * @brief Represents an LR(0) item used in LR automata construction.
 *
 * An LR(0) item has a production of the form \f( A \rightarrow \alpha \bullet
 * \beta \f), where the dot indicates the current parsing position.
 *
 * This structure tracks the antecedent (left-hand side), consequent (right-hand
 * side), the dot position, and special symbols like EPSILON and end-of-line
 * ($).
 */

struct Lr0Item {
    /**
     * @brief The non-terminal on the left-hand side of the production.
     */
    std::string antecedent_;

    /**
     * @brief The sequence of symbols on the right-hand side of the production.
     */
    std::vector<std::string> consequent_;

    /**
     * @brief The symbol representing the empty string (\f(\epsilon\f)).
     */
    std::string epsilon_;

    /**
     * @brief The symbol representing end-of-line or end-of-input ($).
     */
    std::string eol_;

    /**
     * @brief The position of the dot (·) in the production.
     */
    unsigned int dot_ = 0;

    /**
     * @brief Constructs an LR(0) item with the dot at position 0.
     * @param antecedent The left-hand side non-terminal.
     * @param consequent The right-hand side of the production.
     * @param epsilon The EPSILON symbol.
     * @param eol The end-of-line symbol.
     */
    Lr0Item(std::string antecedent, std::vector<std::string> consequent,
            std::string epsilon, std::string eol);

    /**
     * @brief Constructs an LR(0) item with a custom dot position.
     * @param antecedent The left-hand side non-terminal.
     * @param consequent The right-hand side of the production.
     * @param dot The position of the dot.
     * @param epsilon The EPSILON symbol.
     * @param eol The end-of-line symbol.
     */
    Lr0Item(std::string antecedent, std::vector<std::string> consequent,
            unsigned int dot, std::string epsilon, std::string eol);

    /**
     * @brief Returns the symbol immediately after the dot, or empty if the dot is at the end.
     * @return The symbol after the dot, or an empty string.
     */
    std::string NextToDot() const;

    /**
     * @brief Prints the LR(0) item to the standard output in a human-readable format.
     */
    void PrintItem() const;

    /**
     * @brief Converts the item to a string representation, including the dot position.
     * @return A string representation of the item.
     */
    std::string ToString() const;

    /**
     * @brief Advances the dot one position to the right.
     */
    void AdvanceDot();

    /**
     * @brief Checks whether the dot has reached the end of the production.
     * @return true if the item is complete; false otherwise.
     */
    bool IsComplete() const;

    /**
     * @brief Equality operator for comparing two LR(0) items.
     * @param other The item to compare with.
     * @return true if both items are equal; false otherwise.
     */
    bool        operator==(const Lr0Item& other) const;
};

namespace std {
template <> struct hash<Lr0Item> {
    size_t operator()(const Lr0Item& item) const;
};
} // namespace std
