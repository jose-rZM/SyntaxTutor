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

#include "symbol_table.hpp"
#include <unordered_map>
#include <vector>

void SymbolTable::PutSymbol(const std::string& identifier, bool isTerminal) {
    if (isTerminal) {
        st_.insert({identifier, symbol_type::TERMINAL});
        terminals_.insert(identifier);
        terminals_wtho_eol_.insert(identifier);

    } else {
        st_.insert({identifier, symbol_type::NO_TERMINAL});
        non_terminals_.insert(identifier);
    }
}

bool SymbolTable::In(const std::string& s) const {
    return st_.contains(s);
}

bool SymbolTable::IsTerminal(const std::string& s) const {
    return terminals_.contains(s);
}

bool SymbolTable::IsTerminalWthoEol(const std::string& s) const {
    return s != EPSILON_ && terminals_.contains(s);
}
