#pragma once

#include "lr0_item.hpp"
#include "qt_modal_test_utils.h"
#include "slr1_parser.hpp"
#include "slrtutorwindow.h"
#include "slrwizardpage.h"

#include <QApplication>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QTest>
#include <QWizard>

#include <algorithm>
#include <utility>
#include <vector>

namespace SlrTutorTestUtils {

inline QString productionToCompactString(const std::vector<std::string>& production,
                                         unsigned                        dot) {
    QString result;
    for (unsigned i = 0; i < production.size(); ++i) {
        if (i == dot) {
            result += '.';
        }
        result += QString::fromStdString(production[i]);
    }
    if (dot >= production.size()) {
        result += '.';
    }
    return result;
}

inline QString itemToAnswer(const Lr0Item& item) {
    return QString("%1 -> %2")
        .arg(QString::fromStdString(item.antecedent_))
        .arg(productionToCompactString(item.consequent_, item.dot_));
}

inline QStringList sortedItemAnswers(const std::unordered_set<Lr0Item>& items) {
    QStringList lines;
    for (const Lr0Item& item : items) {
        lines.append(itemToAnswer(item));
    }
    std::sort(lines.begin(), lines.end());
    return lines;
}

inline QString itemsAnswer(const std::unordered_set<Lr0Item>& items) {
    return sortedItemAnswers(items).join('\n');
}

inline QString rulesAnswer(
    const std::vector<std::pair<std::string, std::vector<std::string>>>& rules) {
    QStringList lines;
    for (const auto& [lhs, rhs] : rules) {
        QString consequent;
        for (const std::string& symbol : rhs) {
            consequent += QString::fromStdString(symbol);
        }
        lines.append(QString("%1 -> %2")
                         .arg(QString::fromStdString(lhs))
                         .arg(consequent));
    }
    return lines.join('\n');
}

inline QString sortedCommaAnswer(QStringList values) {
    values.removeAll({});
    std::sort(values.begin(), values.end());
    return values.join(',');
}

inline QString stringSetAnswer(const QSet<QString>& values) {
    return sortedCommaAnswer(QStringList(values.begin(), values.end()));
}

inline QString idSetAnswer(const QSet<unsigned>& values) {
    QStringList parts;
    for (unsigned value : values) {
        parts.append(QString::number(value));
    }
    return sortedCommaAnswer(parts);
}

inline QString idCountAnswer(const QMap<unsigned, unsigned>& values) {
    QStringList parts;
    for (auto it = values.cbegin(); it != values.cend(); ++it) {
        parts.append(QString("%1:%2").arg(it.key()).arg(it.value()));
    }
    return parts.join(',');
}

inline QString sortedSymbolsAnswer(const QStringList& values) {
    return sortedCommaAnswer(values);
}

inline QString correctAnswerForCurrentState(SLRTutorWindow& tutor) {
    const QString state = tutor.currentStateForTest();

    if (state == "A" || state == "A'") {
        return itemsAnswer(tutor.solutionForA());
    }
    if (state == "A1") {
        return tutor.solutionForA1();
    }
    if (state == "A2") {
        return tutor.solutionForA2();
    }
    if (state == "A3") {
        return rulesAnswer(tutor.solutionForA3());
    }
    if (state == "A4") {
        return itemsAnswer(tutor.solutionForA4());
    }
    if (state == "B") {
        return QString::number(tutor.solutionForB());
    }
    if (state == "C") {
        return QString::number(tutor.solutionForC());
    }
    if (state == "CA") {
        return sortedSymbolsAnswer(tutor.solutionForCA());
    }
    if (state == "CB") {
        return tutor.currentCbSymbolForTest() == "EPSILON"
                   ? QString()
                   : itemsAnswer(tutor.solutionForCB());
    }
    if (state == "D" || state == "D'") {
        return tutor.solutionForD().join(',');
    }
    if (state == "D1") {
        return tutor.solutionForD1();
    }
    if (state == "D2") {
        return tutor.solutionForD2();
    }
    if (state == "E") {
        return QString::number(tutor.solutionForE());
    }
    if (state == "E1") {
        return idSetAnswer(tutor.solutionForE1());
    }
    if (state == "E2") {
        return idCountAnswer(tutor.solutionForE2());
    }
    if (state == "F") {
        return idSetAnswer(tutor.solutionForF());
    }
    if (state == "FA") {
        return stringSetAnswer(tutor.solutionForFA());
    }
    if (state == "G") {
        return stringSetAnswer(tutor.solutionForG());
    }

    return {};
}

inline void submitCorrectAnswerForCurrentState(SLRTutorWindow& tutor) {
    tutor.setAnswerForTest(correctAnswerForCurrentState(tutor));
    tutor.submitForTest();
}

inline void driveTutorToState(SLRTutorWindow& tutor, const QString& targetState,
                              int maxSteps = 200) {
    int steps = 0;
    while (tutor.currentStateForTest() != targetState && steps < maxSteps) {
        submitCorrectAnswerForCurrentState(tutor);
        ++steps;
    }
    QCOMPARE(tutor.currentStateForTest(), targetState);
}

inline QVector<QPair<QString, QVector<QString>>> buildSortedGrammar(
    const Grammar& grammar) {
    QVector<QString> sortedNonTerminals;
    for (const std::string& nonTerminal : grammar.st_.non_terminals_) {
        sortedNonTerminals.append(QString::fromStdString(nonTerminal));
    }
    std::ranges::sort(sortedNonTerminals, [](const QString& a, const QString& b) {
        if (a == "S")
            return true;
        if (b == "S")
            return false;
        return a < b;
    });

    QVector<QPair<QString, QVector<QString>>> rules;
    for (const QString& nt : std::as_const(sortedNonTerminals)) {
        for (const production& prod : grammar.g_.at(nt.toStdString())) {
            QVector<QString> rhs;
            for (const std::string& symbol : prod) {
                rhs.append(QString::fromStdString(symbol));
            }
            rules.append({nt, rhs});
        }
    }
    return rules;
}

inline QVector<QVector<QString>> buildExpectedTable(const Grammar& grammar,
                                                    QTableWidget*  table) {
    SLR1Parser parser(grammar);
    parser.MakeParser();
    const auto sortedGrammar = buildSortedGrammar(grammar);
    const QStringList cols = QtModalTestUtils::horizontalHeaders(table);
    QVector<QVector<QString>> raw(table->rowCount(), QVector<QString>(table->columnCount()));

    for (const state& slrState : parser.states_) {
        const int row = static_cast<int>(slrState.id_);
        for (int col = 0; col < cols.size(); ++col) {
            const QString symbol = cols.at(col);
            bool filled = false;

            if (auto actMapIt = parser.actions_.find(slrState.id_);
                actMapIt != parser.actions_.end()) {
                auto actIt = actMapIt->second.find(symbol.toStdString());
                if (actIt != actMapIt->second.end()) {
                    switch (actIt->second.action) {
                    case SLR1Parser::Action::Shift:
                        raw[row][col] = QString("s%1")
                                            .arg(parser.transitions_.at(slrState.id_)
                                                     .at(symbol.toStdString()));
                        filled = true;
                        break;
                    case SLR1Parser::Action::Reduce: {
                        int prodIdx = -1;
                        QVector<QString> consequent;
                        for (const std::string& token : actIt->second.item->consequent_) {
                            consequent.append(QString::fromStdString(token));
                        }
                        for (int i = 0; i < sortedGrammar.size(); ++i) {
                            const auto& rule = sortedGrammar.at(i);
                            if (rule.first.toStdString() == actIt->second.item->antecedent_ &&
                                rule.second == consequent) {
                                prodIdx = i;
                                break;
                            }
                        }
                        raw[row][col] = QString("r%1").arg(prodIdx);
                        filled = true;
                        break;
                    }
                    case SLR1Parser::Action::Accept:
                        raw[row][col] = "acc";
                        filled = true;
                        break;
                    case SLR1Parser::Action::Empty:
                        break;
                    }
                }
            }

            if (!filled) {
                if (auto transIt = parser.transitions_.find(slrState.id_);
                    transIt != parser.transitions_.end()) {
                    auto gotoIt = transIt->second.find(symbol.toStdString());
                    if (gotoIt != transIt->second.end()) {
                        raw[row][col] = QString::number(gotoIt->second);
                    }
                }
            }
        }
    }

    return raw;
}

inline QVector<QVector<QString>> buildWrongTable(const Grammar& grammar,
                                                 QTableWidget*  table) {
    QVector<QVector<QString>> raw = buildExpectedTable(grammar, table);
    for (int row = 0; row < raw.size(); ++row) {
        for (int col = 0; col < raw[row].size(); ++col) {
            if (!raw[row][col].isEmpty()) {
                const QString value = raw[row][col];
                if (value == "acc") {
                    raw[row][col] = QStringLiteral("s0");
                } else if (value.startsWith('s')) {
                    raw[row][col] = QStringLiteral("s999");
                } else if (value.startsWith('r')) {
                    raw[row][col] = QStringLiteral("r999");
                } else {
                    raw[row][col] = QStringLiteral("999");
                }
                return raw;
            }
        }
    }
    if (!raw.isEmpty() && !raw.first().isEmpty()) {
        raw[0][0] = QStringLiteral("999");
    }
    return raw;
}

inline void finishWizard(QWizard* wizard) {
    QPointer<QWizard> wizardGuard(wizard);
    QVERIFY(wizardGuard != nullptr);
    while (wizardGuard != nullptr && wizardGuard->isVisible()) {
        auto* page = qobject_cast<SLRWizardPage*>(wizardGuard->currentPage());
        QVERIFY(page != nullptr);
        auto* edit = page->findChild<QLineEdit*>("slrWizardAnswerEdit");
        QVERIFY(edit != nullptr);
        edit->setText(page->expectedForTest());
        QApplication::processEvents();

        const bool isFinalPage = page->isFinalPage();
        QPushButton* nextButton = qobject_cast<QPushButton*>(
            wizardGuard->button(isFinalPage
                               ? QWizard::FinishButton
                               : QWizard::NextButton));
        QVERIFY(nextButton != nullptr);
        QTest::mouseClick(nextButton, Qt::LeftButton);
        QApplication::processEvents();

        if (isFinalPage) {
            break;
        }
        if (wizardGuard == nullptr || !wizardGuard->isVisible()) {
            break;
        }
    }
}

} // namespace SlrTutorTestUtils
