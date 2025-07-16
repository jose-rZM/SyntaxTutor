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
#include "lr0_item.hpp"
#include <cstddef>
#include <functional>
#include <numeric>
#include <unordered_set>

/**
 * @struct state
 * @brief Represents a state in the LR(0) automaton.
 *
 * Each state consists of a unique identifier and a set of LR(0) items
 * that define its core. States are used to build the SLR(1) parsing table.
 */
struct state {
    /**
     * @brief The set of LR(0) items that make up this state.
     */
    std::unordered_set<Lr0Item> items_;

    /**
     * @brief Unique identifier of the state.
     */
    unsigned int id_;

    /**
     * @brief Equality operator for comparing states based on their items.
     * @param other The state to compare with.
     * @return true if both states have the same item set; false otherwise.
     */
    bool operator==(const state& other) const { return other.items_ == items_; }
};

namespace std {
template <> struct hash<state> {
    size_t operator()(const state& st) const {
        size_t seed =
            std::accumulate(st.items_.begin(), st.items_.end(), 0,
                            [](size_t acc, const Lr0Item& item) {
                                return acc ^ (std::hash<Lr0Item>()(item));
                            });
        return seed;
    }
};
} // namespace std
