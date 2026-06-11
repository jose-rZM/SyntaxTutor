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

#include "grammar_parser.hpp"
#include <algorithm>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

constexpr std::string_view kArrow{"->"};
constexpr std::string_view kArrowUtf8{"→"};
constexpr std::string_view kDotUtf8{"·"};
constexpr std::string_view kEpsilonUtf8{"ε"};

struct RawRule {
    std::string                           antecedent;
    std::vector<std::vector<std::string>> productions;
    int                                   line;
};

bool IsBlank(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

bool IsBlankText(const std::string& s) {
    return std::ranges::all_of(s, IsBlank);
}

std::string Trimmed(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return {};
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::vector<std::string> SplitOnBlanks(const std::string& s) {
    std::vector<std::string> tokens;
    std::string              current;
    for (char c : s) {
        if (IsBlank(c)) {
            if (!current.empty()) {
                tokens.push_back(std::move(current));
                current.clear();
            }
        } else {
            current.push_back(c);
        }
    }
    if (!current.empty()) {
        tokens.push_back(std::move(current));
    }
    return tokens;
}

std::string NormalizeArrows(const std::string& text) {
    std::string normalized;
    normalized.reserve(text.size());
    size_t i = 0;
    while (i < text.size()) {
        if (text.compare(i, kArrowUtf8.size(), kArrowUtf8) == 0) {
            normalized += kArrow;
            i += kArrowUtf8.size();
        } else {
            normalized.push_back(text[i]);
            ++i;
        }
    }
    return normalized;
}

// Splits a rule body into |-separated alternatives, where an empty
// alternative denotes an epsilon production.
std::vector<std::string> SplitAlternatives(const std::string& body) {
    std::vector<std::string> alternatives;
    std::string              current;
    for (char c : body) {
        if (c == '|') {
            alternatives.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    alternatives.push_back(current);
    return alternatives;
}

void ParseChunk(const std::string& chunk, int line, std::vector<RawRule>& rules,
                std::vector<GrammarParseError>& errors) {
    size_t arrowPos = chunk.find(kArrow);
    if (arrowPos == std::string::npos) {
        errors.push_back(
            {GrammarParseError::Kind::MissingArrow, line, Trimmed(chunk)});
        return;
    }

    const std::string lhsText = chunk.substr(0, arrowPos);
    const std::string body    = chunk.substr(arrowPos + kArrow.size());

    if (body.find(kArrow) != std::string::npos) {
        errors.push_back(
            {GrammarParseError::Kind::ExtraArrow, line, Trimmed(chunk)});
        return;
    }

    const std::vector<std::string> lhsTokens = SplitOnBlanks(lhsText);
    if (lhsTokens.empty()) {
        errors.push_back(
            {GrammarParseError::Kind::EmptyLeftHandSide, line, Trimmed(chunk)});
        return;
    }
    if (lhsTokens.size() > 1) {
        errors.push_back({GrammarParseError::Kind::MultipleLeftHandSide, line,
                          Trimmed(lhsText)});
        return;
    }

    RawRule rule{lhsTokens.front(), {}, line};
    bool    valid = true;
    if (GrammarParser::IsReservedToken(rule.antecedent)) {
        errors.push_back(
            {GrammarParseError::Kind::ReservedSymbol, line, rule.antecedent});
        valid = false;
    }

    for (const std::string& alternative : SplitAlternatives(body)) {
        std::vector<std::string> symbols = SplitOnBlanks(alternative);
        for (const std::string& symbol : symbols) {
            if (GrammarParser::IsReservedToken(symbol)) {
                errors.push_back(
                    {GrammarParseError::Kind::ReservedSymbol, line, symbol});
                valid = false;
            }
        }
        rule.productions.push_back(std::move(symbols));
    }

    if (valid) {
        rules.push_back(std::move(rule));
    }
}

std::string FreshAxiomName(const SymbolTable& st, const std::string& axiom) {
    if (!st.In("S")) {
        return "S";
    }
    std::string candidate = axiom + "'";
    while (st.In(candidate)) {
        candidate += "'";
    }
    return candidate;
}

Grammar BuildGrammar(const std::vector<RawRule>& rules) {
    Grammar gr;

    for (const RawRule& rule : rules) {
        gr.st_.PutSymbol(rule.antecedent, false);
    }

    std::set<std::pair<std::string, std::vector<std::string>>> seen;
    for (const RawRule& rule : rules) {
        for (const std::vector<std::string>& symbols : rule.productions) {
            production prod = symbols;
            if (prod.empty()) {
                prod.push_back(gr.st_.EPSILON_);
            }
            for (const std::string& symbol : prod) {
                if (symbol != gr.st_.EPSILON_) {
                    gr.st_.PutSymbol(symbol, !gr.st_.IsNonTerminal(symbol));
                }
            }
            if (seen.insert({rule.antecedent, prod}).second) {
                gr.AddProduction(rule.antecedent, prod);
            }
        }
    }

    const std::string userAxiom = rules.front().antecedent;
    const std::string axiom     = FreshAxiomName(gr.st_, userAxiom);
    gr.st_.PutSymbol(axiom, false);
    gr.AddProduction(axiom, {userAxiom, gr.st_.EOL_});
    gr.SetAxiom(axiom);
    return gr;
}

} // namespace

bool GrammarParser::IsReservedToken(const std::string& token) {
    if (token.empty() || token == "$" || token == "EPSILON" ||
        token == kEpsilonUtf8 || token == kDotUtf8) {
        return true;
    }
    if (token.find(kArrow) != std::string::npos ||
        token.find(kDotUtf8) != std::string::npos) {
        return true;
    }
    return std::ranges::any_of(token, [](char c) {
        return c == '.' || c == ',' || c == ':' || c == ';' || c == '|' ||
               c == '$';
    });
}

GrammarParseResult GrammarParser::Parse(const std::string& text) {
    const std::string normalized = NormalizeArrows(text);

    std::vector<RawRule>           rules;
    std::vector<GrammarParseError> errors;

    std::string chunk;
    int         line      = 1;
    int         chunkLine = 1;
    bool        chunkSeen = false;

    for (char c : normalized) {
        if (c == '.') {
            if (!IsBlankText(chunk)) {
                ParseChunk(chunk, chunkLine, rules, errors);
            }
            chunk.clear();
            chunkSeen = false;
        } else {
            if (!chunkSeen && !IsBlank(c)) {
                chunkLine = line;
                chunkSeen = true;
            }
            chunk.push_back(c);
        }
        if (c == '\n') {
            ++line;
        }
    }

    if (!IsBlankText(chunk)) {
        errors.push_back({GrammarParseError::Kind::MissingEndDot, chunkLine,
                          Trimmed(chunk)});
    }

    if (rules.empty() && errors.empty()) {
        errors.push_back({GrammarParseError::Kind::EmptyGrammar, 1, ""});
    }

    GrammarParseResult result;
    result.errors = std::move(errors);
    if (result.errors.empty()) {
        result.grammar = BuildGrammar(rules);
    }
    return result;
}
