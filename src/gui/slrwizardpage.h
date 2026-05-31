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

#ifndef SLRWIZARDPAGE_H
#define SLRWIZARDPAGE_H

#include <QLabel>
#include <QLineEdit>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QWidget>

/**
 * @class SLRWizardPage
 * @brief A single guided step for constructing the SLR(1) table.
 *
 * This widget presents one (state, symbol) question at a time with a compact
 * explanation card and an answer card. It validates the current input and
 * emits a signal whenever the completion state changes.
 */
class SLRWizardPage : public QWidget {
    Q_OBJECT
  public:
    /**
     * @brief Constructs a page for a specific cell in the SLR(1) table.
     *
     * @param state The state ID (row index in the table).
     * @param symbol The grammar symbol (column header).
     * @param explanation A pedagogical explanation shown to the user.
     * @param expected The expected answer (e.g., "s2", "r1", "acc", or a
     * state number).
     * @param parent The parent widget.
     */
    SLRWizardPage(int state, const QString& symbol, const QString& explanation,
                  const QString& expected, QWidget* parent = nullptr)
        : QWidget(parent), m_title(tr("Estado %1, símbolo '%2'").arg(state).arg(symbol)),
          m_expected(expected) {
        auto* rootLayout = new QVBoxLayout(this);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->setSpacing(22);

        auto* explanationTitle = new QLabel(tr("Pista"), this);
        explanationTitle->setObjectName("slrWizardSectionTitle");
        rootLayout->addWidget(explanationTitle);

        auto* explanationLabel = new QLabel(explanation, this);
        explanationLabel->setObjectName("slrWizardExplanationLabel");
        explanationLabel->setWordWrap(true);
        rootLayout->addWidget(explanationLabel);

        auto* answerTitle = new QLabel(tr("Tu respuesta"), this);
        answerTitle->setObjectName("slrWizardSectionTitle");
        rootLayout->addWidget(answerTitle);

        m_edit = new QLineEdit(this);
        m_edit->setObjectName("slrWizardAnswerEdit");
        m_edit->setMinimumHeight(48);
        m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        m_edit->setPlaceholderText(
            tr("Escribe tu respuesta (p.ej. s3, r2, acc, 5)"));
        rootLayout->addWidget(m_edit);

        m_feedback = new QLabel(this);
        m_feedback->setObjectName("slrWizardFeedbackLabel");
        m_feedback->setWordWrap(true);
        m_feedback->setMinimumHeight(40);
        m_feedback->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        rootLayout->addWidget(m_feedback);

        rootLayout->addStretch(1);

        connect(m_edit, &QLineEdit::textChanged, this,
                &SLRWizardPage::onTextChanged);
        connect(m_edit, &QLineEdit::returnPressed, this,
                &SLRWizardPage::submitRequested);
    }

    QString titleText() const { return m_title; }
    bool    isComplete() const { return m_isComplete; }
    void    focusAnswerField() { m_edit->setFocus(); }

#ifdef SYNTAXTUTOR_TESTING
    QString expectedForTest() const { return m_expected; }
#endif

  signals:
    void completionChanged(bool complete);
    void submitRequested();

  private slots:
    /**
     * @brief Checks the user's input and updates inline feedback.
     * @param text The current user input.
     */
    void onTextChanged(const QString& text) {
        const QString trimmed = text.trimmed();
        const bool    correct = (trimmed == m_expected);

        if (trimmed.isEmpty()) {
            m_feedback->clear();
        } else if (correct) {
            m_feedback->setText(
                tr("✔ Respuesta correcta, pasa a la siguiente pregunta"));
        } else {
            m_feedback->setText(tr("✘ Incorrecto, revisa el enunciado. "
                                   "Consulta los estados que has "
                                   "construido."));
        }

        if (m_isComplete == correct) {
            return;
        }

        m_isComplete = correct;
        emit completionChanged(m_isComplete);
    }

  private:
    QString    m_title;    ///< Header title for this guided step.
    QString    m_expected; ///< Expected user response.
    QLabel*    m_feedback; ///< Inline feedback label for answer validation.
    QLineEdit* m_edit;     ///< Input field for the user's answer.
    bool       m_isComplete =
        false; ///< Whether the user has entered the correct response.
};

#endif // SLRWIZARDPAGE_H
