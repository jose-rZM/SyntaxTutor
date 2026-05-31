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

#ifndef SLRWIZARD_H
#define SLRWIZARD_H

#include "slr1_parser.hpp"
#include "slrwizardpage.h"

#include <QDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

/**
 * @class SLRWizard
 * @brief Interactive assistant that guides the student step-by-step through the
 * SLR(1) parsing table.
 *
 * This dialog presents one cell of the SLR(1) parsing table at a time, asking
 * the user to deduce the correct ACTION or GOTO entry based on the LR(0)
 * automaton and FOLLOW sets. It intentionally avoids the native wizard widget
 * to keep a consistent look across platforms.
 */
class SLRWizard : public QDialog {
    Q_OBJECT
  public:
    /**
     * @brief Constructs the SLR(1) guided dialog with all necessary parsing
     * context.
     *
     * @param parser The SLR(1) parser instance containing the LR(0) states and
     * transitions.
     * @param rawTable The target parsing table (student version or reference).
     * @param colHeaders Header symbols (terminals and non-terminals).
     * @param sortedGrammar Ordered list of grammar rules for reduce
     * explanations.
     * @param parent Parent widget.
     */
    SLRWizard(SLR1Parser& parser, const QVector<QVector<QString>>& rawTable,
              const QStringList&                               colHeaders,
              const QVector<QPair<QString, QVector<QString>>>& sortedGrammar,
              QWidget*                                         parent = nullptr)
        : QDialog(parent) {
        setProperty("guidedDialog", "slr");
        setWindowTitle(tr("Completar tabla SLR"));
        setWindowFlag(Qt::WindowCloseButtonHint, true);
        setModal(true);
        resize(640, 440);
        setMinimumSize(560, 400);

        auto* rootLayout = new QVBoxLayout(this);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->setSpacing(0);

        auto* panel = new QFrame(this);
        panel->setObjectName("slrWizardPanel");

        auto* panelLayout = new QVBoxLayout(panel);
        panelLayout->setContentsMargins(28, 24, 28, 24);
        panelLayout->setSpacing(20);

        auto* headerLayout = new QHBoxLayout();
        headerLayout->setSpacing(12);

        auto* titleColumn = new QVBoxLayout();
        titleColumn->setContentsMargins(0, 0, 0, 0);
        titleColumn->setSpacing(6);

        m_titleLabel = new QLabel(panel);
        m_titleLabel->setObjectName("slrWizardTitle");
        titleColumn->addWidget(m_titleLabel);

        m_stepCounterLabel = new QLabel(panel);
        m_stepCounterLabel->setObjectName("slrWizardStepCounter");
        titleColumn->addWidget(m_stepCounterLabel);

        m_progressBar = new QProgressBar(panel);
        m_progressBar->setObjectName("slrWizardProgress");
        m_progressBar->setTextVisible(false);
        m_progressBar->setRange(0, 100);
        m_progressBar->setFixedHeight(4);
        titleColumn->addWidget(m_progressBar);

        headerLayout->addLayout(titleColumn, 1);

        m_closeButton = new QPushButton(tr("Salir"), panel);
        m_closeButton->setObjectName("slrWizardCloseButton");
        m_closeButton->setAutoDefault(false);
        m_closeButton->setDefault(false);
        m_closeButton->setCursor(Qt::PointingHandCursor);
        headerLayout->addWidget(m_closeButton, 0, Qt::AlignTop);

        panelLayout->addLayout(headerLayout);

        m_stack = new QStackedWidget(panel);
        m_stack->setObjectName("slrWizardStack");
        panelLayout->addWidget(m_stack, 1);

        auto* footerLayout = new QHBoxLayout();
        footerLayout->setContentsMargins(0, 0, 0, 0);
        footerLayout->setSpacing(12);
        footerLayout->addStretch(1);

        m_nextButton = new QPushButton(panel);
        m_nextButton->setObjectName("slrWizardNextButton");
        m_nextButton->setProperty("role", "primary");
        m_nextButton->setAutoDefault(false);
        m_nextButton->setDefault(false);
        footerLayout->addWidget(m_nextButton);
        panelLayout->addLayout(footerLayout);

        rootLayout->addWidget(panel);

        const int nTerm = parser.gr_.st_.terminals_.size();
        for (int i = 0; i < rawTable.size(); ++i) {
            for (int j = 0; j < colHeaders.size(); ++j) {
                const QString sym = colHeaders[j];
                QString       expected;
                QString       explanation;

                if (j < nTerm) {
                    auto itAct = parser.actions_.at(i).find(sym.toStdString());
                    SLR1Parser::s_action act =
                        (itAct != parser.actions_.at(i).end()
                             ? itAct->second
                             : SLR1Parser::s_action{nullptr,
                                                    SLR1Parser::Action::Empty});

                    switch (act.action) {
                    case SLR1Parser::Action::Shift: {
                        const unsigned to =
                            parser.transitions_.at(i).at(sym.toStdString());
                        expected    = QString("s%1").arg(to);
                        explanation = tr("Estado %1: existe transición δ(%1, "
                                         "'%2'). ¿A qué estado harías shift?")
                                          .arg(i)
                                          .arg(sym);
                        break;
                    }
                    case SLR1Parser::Action::Reduce: {
                        int idx = -1;
                        for (int k = 0; k < sortedGrammar.size(); ++k) {
                            auto& rule = sortedGrammar[k];
                            if (rule.first.toStdString() ==
                                    act.item->antecedent_ &&
                                stdVectorToQVector(act.item->consequent_) ==
                                    rule.second) {
                                idx = k;
                                break;
                            }
                        }
                        expected = QString("r%1").arg(idx);
                        explanation = tr("Estado %1: contiene el ítem [%2 → "
                                         "...·] y '%3' ∈ SIG(%2). ¿Qué regla "
                                         "usas para reducir (0, 1, ...)?")
                                          .arg(i)
                                          .arg(QString::fromStdString(
                                              act.item->antecedent_))
                                          .arg(colHeaders[j]);
                        break;
                    }
                    case SLR1Parser::Action::Accept:
                        expected    = "acc";
                        explanation = tr("Estado %1: contiene [S → A · $]. "
                                         "¿Qué palabra clave usas para "
                                         "aceptar?")
                                          .arg(i);
                        break;
                    case SLR1Parser::Action::Empty:
                    default:
                        continue;
                    }
                } else {
                    const auto nonT = sym.toStdString();
                    if (!parser.transitions_.contains(i)) {
                        continue;
                    }

                    auto itGo = parser.transitions_.at(i).find(nonT);
                    if (itGo == parser.transitions_.at(i).end()) {
                        continue;
                    }

                    expected    = QString::number(itGo->second);
                    explanation = tr("Estado %1: δ(%1, '%2') existe. ¿A qué "
                                     "estado va la transición? (pon solo el "
                                     "número)")
                                      .arg(i)
                                      .arg(sym);
                }

                auto* page = new SLRWizardPage(i, sym, explanation, expected,
                                               m_stack);
                connect(page, &SLRWizardPage::completionChanged, this,
                        &SLRWizard::updateCurrentPage);
                connect(page, &SLRWizardPage::submitRequested, this,
                        &SLRWizard::advance);
                m_stack->addWidget(page);
            }
        }

        connect(m_closeButton, &QPushButton::clicked, this,
                &QDialog::reject);
        connect(m_nextButton, &QPushButton::clicked, this,
                &SLRWizard::advance);

        updateCurrentPage();
        QTimer::singleShot(0, this, [this]() {
            if (auto* page = currentPage()) {
                page->focusAnswerField();
            }
        });
    }

    SLRWizardPage* currentPage() const {
        return qobject_cast<SLRWizardPage*>(m_stack->currentWidget());
    }

    QPushButton* nextButton() const { return m_nextButton; }
    QPushButton* closeButton() const { return m_closeButton; }

    bool isOnLastPage() const {
        return m_stack->count() > 0 &&
               m_stack->currentIndex() == m_stack->count() - 1;
    }

    /**
     * @brief Converts a std::vector<std::string> to QVector<QString> for UI
     * compatibility.
     * @param vec The input vector of strings.
     * @return A QVector of QStrings.
     */
    QVector<QString> stdVectorToQVector(const std::vector<std::string>& vec) {
        QVector<QString> result;
        result.reserve(vec.size());
        for (const auto& str : vec) {
            result.push_back(QString::fromStdString(str));
        }
        return result;
    }

  private slots:
    void advance() {
        auto* page = currentPage();
        if (page == nullptr || !page->isComplete()) {
            return;
        }

        if (isOnLastPage()) {
            accept();
            return;
        }

        m_stack->setCurrentIndex(m_stack->currentIndex() + 1);
        updateCurrentPage();
        if (auto* nextPage = currentPage()) {
            nextPage->focusAnswerField();
        }
    }

    void updateCurrentPage() {
        auto* page = currentPage();
        if (page == nullptr) {
            m_titleLabel->setText(tr("Modo guiado"));
            m_stepCounterLabel->clear();
            m_progressBar->setValue(0);
            m_nextButton->setEnabled(false);
            m_nextButton->setText(tr("Finalizar"));
            return;
        }

        const int currentStep = m_stack->currentIndex() + 1;
        const int totalSteps  = m_stack->count();
        m_titleLabel->setText(page->titleText());
        m_stepCounterLabel->setText(
            tr("Paso %1 de %2").arg(currentStep).arg(totalSteps));
        m_progressBar->setValue((100 * currentStep) / qMax(1, totalSteps));
        m_nextButton->setEnabled(page->isComplete());
        m_nextButton->setCursor(page->isComplete() ? Qt::PointingHandCursor
                                                   : Qt::ArrowCursor);
        m_nextButton->setText(isOnLastPage() ? tr("Finalizar")
                                             : tr("Continuar"));
    }

  private:
    QLabel*         m_titleLabel      = nullptr;
    QLabel*         m_stepCounterLabel = nullptr;
    QProgressBar*   m_progressBar     = nullptr;
    QPushButton*    m_closeButton     = nullptr;
    QStackedWidget* m_stack           = nullptr;
    QPushButton*    m_nextButton      = nullptr;
};

#endif // SLRWIZARD_H
