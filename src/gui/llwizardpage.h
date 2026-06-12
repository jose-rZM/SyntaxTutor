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

#ifndef LLWIZARDPAGE_H
#define LLWIZARDPAGE_H

#include <QLabel>
#include <QLineEdit>
#include <QRegularExpression>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QWidget>

/**
 * @class LLWizardPage
 * @brief A single guided step for constructing the LL(1) table.
 *
 * This widget presents one (non-terminal, terminal) cell at a time with a
 * compact explanation card and an answer card. The expected answer is the
 * production that belongs in the cell; the input is validated ignoring
 * whitespace so "a B", "a  B" and "aB" are all accepted. It emits a signal
 * whenever the completion state changes.
 */
class LLWizardPage : public QWidget {
    Q_OBJECT
  public:
    /**
     * @brief Constructs a page for a specific cell in the LL(1) table.
     *
     * @param nonTerminal The non-terminal symbol (row header).
     * @param symbol The terminal symbol (column header).
     * @param explanation A pedagogical explanation shown to the user.
     * @param expected The expected production, space-separated (e.g. "a B"
     * or "EPSILON").
     * @param parent The parent widget.
     */
    LLWizardPage(const QString& nonTerminal, const QString& symbol,
                 const QString& explanation, const QString& expected,
                 QWidget* parent = nullptr)
        : QWidget(parent),
          m_title(tr("Fila %1, columna '%2'").arg(nonTerminal).arg(symbol)),
          m_expected(expected) {
        auto* rootLayout = new QVBoxLayout(this);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->setSpacing(22);

        auto* explanationTitle = new QLabel(tr("Pista"), this);
        explanationTitle->setObjectName("llWizardSectionTitle");
        rootLayout->addWidget(explanationTitle);

        auto* explanationLabel = new QLabel(explanation, this);
        explanationLabel->setObjectName("llWizardExplanationLabel");
        explanationLabel->setWordWrap(true);
        rootLayout->addWidget(explanationLabel);

        auto* answerTitle = new QLabel(tr("Tu respuesta"), this);
        answerTitle->setObjectName("llWizardSectionTitle");
        rootLayout->addWidget(answerTitle);

        m_edit = new QLineEdit(this);
        m_edit->setObjectName("llWizardAnswerEdit");
        m_edit->setMinimumHeight(48);
        m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        m_edit->setPlaceholderText(
            tr("Escribe la producción (EPSILON si es vacía)"));
        rootLayout->addWidget(m_edit);

        m_feedback = new QLabel(this);
        m_feedback->setObjectName("llWizardFeedbackLabel");
        m_feedback->setWordWrap(true);
        m_feedback->setMinimumHeight(40);
        m_feedback->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        rootLayout->addWidget(m_feedback);

        rootLayout->addStretch(1);

        connect(m_edit, &QLineEdit::textChanged, this,
                &LLWizardPage::onTextChanged);
        connect(m_edit, &QLineEdit::returnPressed, this,
                &LLWizardPage::submitRequested);
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
     *
     * The comparison ignores whitespace, matching the tolerant parsing the
     * table cells already accept.
     *
     * @param text The current user input.
     */
    void onTextChanged(const QString& text) {
        const QString trimmed = text.trimmed();
        const bool    correct = (compact(trimmed) == compact(m_expected));

        if (trimmed.isEmpty()) {
            m_feedback->clear();
        } else if (correct) {
            m_feedback->setText(
                tr("✔ Respuesta correcta, pasa a la siguiente pregunta"));
        } else {
            m_feedback->setText(tr("✘ Incorrecto, repasa los símbolos "
                                   "directores (SD) en el panel del tutor."));
        }

        if (m_isComplete == correct) {
            return;
        }

        m_isComplete = correct;
        emit completionChanged(m_isComplete);
    }

  private:
    static QString compact(const QString& text) {
        QString result = text;
        result.remove(QRegularExpression("\\s+"));
        return result;
    }

    QString    m_title;    ///< Header title for this guided step.
    QString    m_expected; ///< Expected production, space-separated.
    QLabel*    m_feedback; ///< Inline feedback label for answer validation.
    QLineEdit* m_edit;     ///< Input field for the user's answer.
    bool       m_isComplete =
        false; ///< Whether the user has entered the correct response.
};

#endif // LLWIZARDPAGE_H
