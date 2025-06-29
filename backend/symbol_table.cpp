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
