#pragma once

#include "grammar.hpp"
#include "ll1_parser.hpp"

#include <QSet>
#include <QString>
#include <QStringList>

#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>

namespace Ll1TutorTestUtils {

inline std::vector<std::string> toStdVector(const QStringList& symbols) {
    std::vector<std::string> out;
    out.reserve(static_cast<size_t>(symbols.size()));
    for (const QString& symbol : symbols) {
        out.push_back(symbol.toStdString());
    }
    return out;
}

inline QString joinSet(const std::unordered_set<std::string>& symbols) {
    QStringList values;
    for (const std::string& symbol : symbols) {
        values.append(QString::fromStdString(symbol));
    }
    std::sort(values.begin(), values.end());
    return values.join(',');
}

inline QString tableSizeAnswer(const Grammar& grammar) {
    return QString("%1,%2")
        .arg(grammar.st_.non_terminals_.size())
        .arg(grammar.st_.terminals_.size());
}

inline QString nonTerminalCountAnswer(const Grammar& grammar) {
    return QString::number(grammar.st_.non_terminals_.size());
}

inline QString terminalCountAnswer(const Grammar& grammar) {
    return QString::number(grammar.st_.terminals_wtho_eol_.size());
}

inline QString predictionSymbolsAnswer(const Grammar&    grammar,
                                       const QString&    antecedent,
                                       const QStringList& consequent) {
    LL1Parser parser(grammar);
    return joinSet(parser.PredictionSymbols(antecedent.toStdString(),
                                            toStdVector(consequent)));
}

inline QString cabAnswer(const Grammar&    grammar,
                         const QString&    antecedent,
                         const QStringList& consequent) {
    LL1Parser                         parser(grammar);
    std::unordered_set<std::string>   result;
    const std::vector<std::string>    rule = toStdVector(consequent);

    parser.First(rule, result);
    if (antecedent.toStdString() == parser.gr_.axiom_ && !rule.empty() &&
        rule.back() == parser.gr_.st_.EOL_ &&
        result.contains(parser.gr_.st_.EPSILON_)) {
        result.erase(parser.gr_.st_.EPSILON_);
        result.insert(parser.gr_.st_.EOL_);
    }

    return joinSet(result);
}

inline QString followAnswer(const Grammar& grammar, const QString& antecedent) {
    LL1Parser parser(grammar);
    return joinSet(parser.Follow(antecedent.toStdString()));
}

} // namespace Ll1TutorTestUtils
