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

#ifndef LLWIZARD_H
#define LLWIZARD_H

#include "ll1_parser.hpp"
#include "llwizardpage.h"

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
 * @class LLWizard
 * @brief Interactive assistant that guides the student step-by-step through
 * the LL(1) parsing table.
 *
 * This dialog presents one non-empty cell of the LL(1) table at a time,
 * asking the user to deduce the production that belongs in it from the
 * prediction symbols (símbolos directores): the cell (A, b) holds the
 * production A → α with b ∈ SD(A → α). It mirrors the SLR(1) guided dialog
 * and intentionally avoids the native wizard widget to keep a consistent
 * look across platforms.
 */
class LLWizard : public QDialog {
    Q_OBJECT
  public:
    /**
     * @brief Constructs the LL(1) guided dialog from the computed table.
     *
     * @param parser The LL(1) parser instance holding the reference table.
     * @param rowHeaders Non-terminal symbols in display order.
     * @param colHeaders Terminal symbols in display order.
     * @param parent Parent widget.
     */
    LLWizard(LL1Parser& parser, const QStringList& rowHeaders,
             const QStringList& colHeaders, QWidget* parent = nullptr)
        : QDialog(parent) {
        setProperty("guidedDialog", "ll");
        setWindowTitle(tr("Completar tabla LL(1)"));
        setWindowFlag(Qt::WindowCloseButtonHint, true);
        setModal(true);
        resize(640, 440);
        setMinimumSize(560, 400);

        auto* rootLayout = new QVBoxLayout(this);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->setSpacing(0);

        auto* panel = new QFrame(this);
        panel->setObjectName("llWizardPanel");

        auto* panelLayout = new QVBoxLayout(panel);
        panelLayout->setContentsMargins(28, 24, 28, 24);
        panelLayout->setSpacing(20);

        auto* headerLayout = new QHBoxLayout();
        headerLayout->setSpacing(12);

        auto* titleColumn = new QVBoxLayout();
        titleColumn->setContentsMargins(0, 0, 0, 0);
        titleColumn->setSpacing(6);

        m_titleLabel = new QLabel(panel);
        m_titleLabel->setObjectName("llWizardTitle");
        titleColumn->addWidget(m_titleLabel);

        m_stepCounterLabel = new QLabel(panel);
        m_stepCounterLabel->setObjectName("llWizardStepCounter");
        titleColumn->addWidget(m_stepCounterLabel);

        m_progressBar = new QProgressBar(panel);
        m_progressBar->setObjectName("llWizardProgress");
        m_progressBar->setTextVisible(false);
        m_progressBar->setRange(0, 100);
        m_progressBar->setFixedHeight(4);
        titleColumn->addWidget(m_progressBar);

        headerLayout->addLayout(titleColumn, 1);

        m_closeButton = new QPushButton(tr("Salir"), panel);
        m_closeButton->setObjectName("llWizardCloseButton");
        m_closeButton->setAutoDefault(false);
        m_closeButton->setDefault(false);
        m_closeButton->setCursor(Qt::PointingHandCursor);
        headerLayout->addWidget(m_closeButton, 0, Qt::AlignTop);

        panelLayout->addLayout(headerLayout);

        m_stack = new QStackedWidget(panel);
        m_stack->setObjectName("llWizardStack");
        panelLayout->addWidget(m_stack, 1);

        auto* footerLayout = new QHBoxLayout();
        footerLayout->setContentsMargins(0, 0, 0, 0);
        footerLayout->setSpacing(12);
        footerLayout->addStretch(1);

        m_nextButton = new QPushButton(panel);
        m_nextButton->setObjectName("llWizardNextButton");
        m_nextButton->setProperty("role", "primary");
        m_nextButton->setAutoDefault(false);
        m_nextButton->setDefault(false);
        footerLayout->addWidget(m_nextButton);
        panelLayout->addLayout(footerLayout);

        rootLayout->addWidget(panel);

        for (const QString& nonTerminal : rowHeaders) {
            const auto rowIt = parser.ll1_t_.find(nonTerminal.toStdString());
            if (rowIt == parser.ll1_t_.end()) {
                continue;
            }
            for (const QString& symbol : colHeaders) {
                const auto cellIt = rowIt->second.find(symbol.toStdString());
                if (cellIt == rowIt->second.end() || cellIt->second.empty() ||
                    cellIt->second[0].empty()) {
                    continue;
                }

                QStringList production;
                for (const std::string& s : cellIt->second[0]) {
                    production.append(QString::fromStdString(s));
                }
                const QString expected = production.join(' ');

                const QString explanation =
                    tr("La celda (%1, '%2') no está vacía: existe una "
                       "producción %1 → α tal que '%2' ∈ SD(%1 → α).\n"
                       "¿Qué producción α escribes en esta celda?")
                        .arg(nonTerminal, symbol);

                auto* page = new LLWizardPage(nonTerminal, symbol, explanation,
                                              expected, m_stack);
                connect(page, &LLWizardPage::completionChanged, this,
                        &LLWizard::updateCurrentPage);
                connect(page, &LLWizardPage::submitRequested, this,
                        &LLWizard::advance);
                m_stack->addWidget(page);
            }
        }

        connect(m_closeButton, &QPushButton::clicked, this, &QDialog::reject);
        connect(m_nextButton, &QPushButton::clicked, this, &LLWizard::advance);

        updateCurrentPage();
        QTimer::singleShot(0, this, [this]() {
            if (auto* page = currentPage()) {
                page->focusAnswerField();
            }
        });
    }

    LLWizardPage* currentPage() const {
        return qobject_cast<LLWizardPage*>(m_stack->currentWidget());
    }

    QPushButton* nextButton() const { return m_nextButton; }
    QPushButton* closeButton() const { return m_closeButton; }

    bool isOnLastPage() const {
        return m_stack->count() > 0 &&
               m_stack->currentIndex() == m_stack->count() - 1;
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
    QLabel*         m_titleLabel       = nullptr;
    QLabel*         m_stepCounterLabel = nullptr;
    QProgressBar*   m_progressBar      = nullptr;
    QPushButton*    m_closeButton      = nullptr;
    QStackedWidget* m_stack            = nullptr;
    QPushButton*    m_nextButton       = nullptr;
};

#endif // LLWIZARD_H
