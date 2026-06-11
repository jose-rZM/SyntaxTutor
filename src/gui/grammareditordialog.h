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

#ifndef GRAMMAREDITORDIALOG_H
#define GRAMMAREDITORDIALOG_H

#include "grammar.hpp"
#include <QDialog>
#include <QSet>
#include <QSyntaxHighlighter>
#include <QTimer>

class QLabel;
class QPlainTextEdit;
class QPushButton;
struct GrammarParseResult;

/**
 * @class GrammarSyntaxHighlighter
 * @brief Lightweight highlighter for user-written grammar text.
 *
 * Colors the structural pieces of a grammar (arrows, rule dots and
 * alternative pipes) and renders the currently-detected non-terminals in the
 * accent color, so the user gets immediate feedback on how their text is
 * being interpreted.
 */
class GrammarSyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
  public:
    explicit GrammarSyntaxHighlighter(QTextDocument* parent);

    /**
     * @brief Updates the set of symbols rendered as non-terminals.
     *
     * Triggers a rehighlight only when the set actually changes.
     *
     * @param nonTerminals Symbols currently classified as non-terminals.
     */
    void setNonTerminals(const QSet<QString>& nonTerminals);

  protected:
    void highlightBlock(const QString& text) override;

  private:
    QSet<QString>   nonTerminals_;
    QTextCharFormat arrowFormat_;
    QTextCharFormat punctuationFormat_;
    QTextCharFormat nonTerminalFormat_;
};

/**
 * @class GrammarEditorDialog
 * @brief Dialog where the user writes their own grammar for an exercise.
 *
 * The user types rules in the `LHS -> SYMBOL ... .` format. The text is
 * parsed and validated live: format errors are reported with line numbers,
 * the detected axiom / non-terminals / terminals are summarized, and the
 * grammar is checked to be LL(1) or SLR(1) depending on the exercise the
 * dialog was opened for. The last accepted grammar is persisted through
 * QSettings so it can be practiced again.
 */
class GrammarEditorDialog : public QDialog {
    Q_OBJECT
  public:
    /**
     * @enum Mode
     * @brief Exercise the grammar is being written for.
     */
    enum class Mode { LL1, SLR1 };

    /**
     * @brief Constructs the editor for the given exercise kind.
     *
     * @param mode Exercise the grammar must be valid for.
     * @param parent Parent widget.
     */
    explicit GrammarEditorDialog(Mode mode, QWidget* parent = nullptr);

    /**
     * @brief Returns the validated, augmented grammar.
     *
     * Only meaningful after the dialog was accepted.
     */
    const Grammar& grammar() const { return grammar_; }

#ifdef SYNTAXTUTOR_TESTING
    /// @brief Testing hook: replaces the editor text and validates at once.
    void setGrammarTextForTest(const QString& text);
    /// @brief Testing hook: whether the current text passed all validations.
    bool isGrammarValidForTest() const { return valid_; }
#endif

  protected:
    void accept() override;

  private slots:
    void scheduleValidation();
    void validateNow();

  private:
    void    buildUi();
    void    updateSummary(const Grammar& grammar);
    void    clearSummary();
    void    setStatus(const QString& message, const QString& state);
    QString describeParseErrors(const GrammarParseResult& result);

    Mode                      mode_;
    Grammar                   grammar_;
    bool                      valid_ = false;
    QTimer                    debounce_;
    QPlainTextEdit*           input_        = nullptr;
    QLabel*                   statusLabel_  = nullptr;
    QLabel*                   axiomValue_   = nullptr;
    QLabel*                   nonTermValue_ = nullptr;
    QLabel*                   termValue_    = nullptr;
    QLabel*                   rulesValue_   = nullptr;
    QPushButton*              startButton_  = nullptr;
    QPushButton*              cancelButton_ = nullptr;
    GrammarSyntaxHighlighter* highlighter_  = nullptr;
};

#endif // GRAMMAREDITORDIALOG_H
