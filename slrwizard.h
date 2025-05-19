#ifndef SLRWIZARD_H
#define SLRWIZARD_H

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWizard>
#include <QWizardPage>
#include "backend/slr1_parser.hpp"
#include "slrwizardpage.h"

class SLRWizard : public QWizard
{
    Q_OBJECT
public:
    SLRWizard(SLR1Parser &parser,
              const QVector<QVector<QString>> &rawTable,
              const QStringList &colHeaders,
              const QVector<QPair<QString, QVector<QString>>> &sortedGrammar,
              QWidget *parent = nullptr)
        : QWizard(parent)
    {
        setWindowTitle("Ayuda interactiva: Tabla SLR(1)");

        const int nTerm = parser.gr_.st_.terminals_.contains(parser.gr_.st_.EPSILON_)
                              ? parser.gr_.st_.terminals_.size() - 1
                              : parser.gr_.st_.terminals_.size();
        SLRWizardPage *last = nullptr;
        // Generar explicación y páginas
        int rows = rawTable.size();
        int cols = colHeaders.size();
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                QString sym = colHeaders[j];
                QString expected;
                QString explanation;
                if (j < nTerm) {
                    auto itAct = parser.actions_.at(i).find(sym.toStdString());
                    SLR1Parser::s_action act
                        = (itAct != parser.actions_.at(i).end()
                               ? itAct->second
                               : SLR1Parser::s_action{nullptr, SLR1Parser::Action::Empty});
                    switch (act.action) {
                    case SLR1Parser::Action::Shift: {
                        unsigned to = parser.transitions_.at(i).at(sym.toStdString());
                        expected = QString("s%1").arg(to);
                        explanation = QString("Estado %1: existe transición δ(%1, '%2'). ¿A qué "
                                              "estado harías shift?")
                                          .arg(i)
                                          .arg(sym);
                        break;
                    }
                    case SLR1Parser::Action::Reduce: {
                        int idx = -1;
                        for (int k = 0; k < sortedGrammar.size(); ++k) {
                            auto &rule = sortedGrammar[k];
                            if (rule.first.toStdString() == act.item->antecedent_
                                && stdVectorToQVector(act.item->consequent_) == rule.second) {
                                idx = k;
                                break;
                            }
                        }
                        expected = QString("r%1").arg(idx);
                        // explicación con FOLLOW
                        std::unordered_set<std::string> F;
                        F = parser.Follow(act.item->antecedent_);
                        QStringList followList;
                        for (auto &t : F)
                            followList << QString::fromStdString(t);
                        explanation = QString("Estado %1: contiene el ítem [%2 → ...·] y '%3' ∈ "
                                              "SIG(%2). ¿Qué regla usas para reducir (0, 1, ...)?")
                                          .arg(i)
                                          .arg(QString::fromStdString(act.item->antecedent_))
                                          .arg(colHeaders[j]);
                        break;
                    }
                    case SLR1Parser::Action::Accept:
                        expected = "acc";
                        explanation = QString("Estado %1: contiene [S' → S ·]. ¿Qué palabra clave "
                                              "usas para aceptar?")
                                          .arg(i);
                        break;
                    case SLR1Parser::Action::Empty:
                    default:
                        continue;
                    }
                } else {
                    // GOTO sobre no terminal
                    auto nonT = sym.toStdString();
                    if (!parser.transitions_.contains(i)) {
                        continue;
                    }
                    auto itGo = parser.transitions_.at(i).find(nonT);
                    if (itGo != parser.transitions_.at(i).end()) {
                        expected = QString::number(itGo->second);
                        explanation = QString("Estado %1: δ(%1, '%2') existe. ¿A qué estado va "
                                              "la transición? (pon solo el número)")
                                          .arg(i)
                                          .arg(sym);
                    } else {
                        continue;
                    }
                }

                SLRWizardPage *page = new SLRWizardPage(i, sym, explanation, expected, this);
                last = page;
                addPage(page);
            }
        }
        if (last) {
            last->setFinalPage(true);
        }
    }

    QVector<QString> stdVectorToQVector(const std::vector<std::string> &vec)
    {
        QVector<QString> result;
        result.reserve(vec.size());
        for (const auto &str : vec) {
            result.push_back(QString::fromStdString(str));
        }
        return result;
    }
};

#endif // SLRWIZARD_H
