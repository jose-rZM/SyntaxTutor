#include "symbol_table.hpp"
#include <unordered_map>
#include <vector>

void SymbolTable::PutSymbol(const std::string& identifier, bool isTerminal) {
    if (isTerminal) {
        st_.insert({identifier, TERMINAL});
        terminals_.insert(identifier);
        terminals_wtho_eol_.insert(identifier);

    } else {
        st_.insert({identifier, NO_TERMINAL});
        non_terminals_.insert(identifier);
    }
}

bool SymbolTable::In(const std::string& s) {
    return st_.find(s) != st_.cend();
}

bool SymbolTable::IsTerminal(const std::string& s) {
    return terminals_.find(s) != terminals_.end();
}

bool SymbolTable::IsTerminalWthoEol(const std::string& s) {
    return s != EPSILON_ && terminals_.find(s) != terminals_.end();
}
