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

#include "grammareditordialog.h"
#include "grammar_factory.hpp"
#include "grammar_parser.hpp"
#include "ll1_parser.hpp"
#include "slr1_parser.hpp"

#include <QFontDatabase>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QStyle>
#include <QVBoxLayout>
#include <algorithm>

namespace {
#ifdef SYNTAXTUTOR_TESTING
constexpr auto kSettingsOrg = "UMA-Test";
constexpr auto kSettingsApp = "SyntaxTutor-Test";
#else
constexpr auto kSettingsOrg = "UMA";
constexpr auto kSettingsApp = "SyntaxTutor";
#endif

constexpr int kDebounceMs       = 200;
constexpr int kMaxListedErrors  = 4;
const char    kLastGrammarKey[] = "userGrammar/lastText";

QLabel* makeSummaryCaption(const QString& text, QWidget* parent) {
    auto* label = new QLabel(text, parent);
    label->setObjectName("grammarEditorSummaryCaption");
    return label;
}

QLabel* makeSummaryValue(QWidget* parent) {
    auto* label = new QLabel(parent);
    label->setObjectName("grammarEditorSummaryValue");
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}
} // namespace

// ===================== GrammarSyntaxHighlighter ==========================

GrammarSyntaxHighlighter::GrammarSyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
    arrowFormat_.setForeground(QColor("#11B3BC"));
    arrowFormat_.setFontWeight(QFont::Bold);
    punctuationFormat_.setForeground(QColor("#9AA5A8"));
    punctuationFormat_.setFontWeight(QFont::Bold);
    nonTerminalFormat_.setForeground(QColor("#36C5CC"));
    nonTerminalFormat_.setFontWeight(QFont::Bold);
}

void GrammarSyntaxHighlighter::setNonTerminals(
    const QSet<QString>& nonTerminals) {
    if (nonTerminals_ == nonTerminals) {
        return;
    }
    nonTerminals_ = nonTerminals;
    rehighlight();
}

void GrammarSyntaxHighlighter::highlightBlock(const QString& text) {
    qsizetype tokenStart = -1;
    auto      closeToken = [&](qsizetype end) {
        if (tokenStart < 0) {
            return;
        }
        const QString token = text.mid(tokenStart, end - tokenStart);
        if (nonTerminals_.contains(token)) {
            setFormat(tokenStart, end - tokenStart, nonTerminalFormat_);
        } else if (token == QStringLiteral("->") ||
                   token == QStringLiteral("→")) {
            setFormat(tokenStart, end - tokenStart, arrowFormat_);
        }
        tokenStart = -1;
    };

    for (qsizetype i = 0; i < text.size(); ++i) {
        const QChar c = text.at(i);
        if (c.isSpace()) {
            closeToken(i);
        } else if (c == '.' || c == '|') {
            closeToken(i);
            setFormat(i, 1, punctuationFormat_);
        } else {
            if (tokenStart < 0) {
                tokenStart = i;
            }
        }
    }
    closeToken(text.size());
}

// ======================== GrammarEditorDialog ============================

GrammarEditorDialog::GrammarEditorDialog(Mode mode, QWidget* parent)
    : QDialog(parent), mode_(mode) {
    buildUi();

    debounce_.setSingleShot(true);
    debounce_.setInterval(kDebounceMs);
    connect(&debounce_, &QTimer::timeout, this,
            &GrammarEditorDialog::validateNow);
    connect(input_, &QPlainTextEdit::textChanged, this,
            &GrammarEditorDialog::scheduleValidation);

    QSettings     settings(kSettingsOrg, kSettingsApp);
    const QString lastGrammar = settings.value(kLastGrammarKey).toString();
    if (!lastGrammar.isEmpty()) {
        input_->setPlainText(lastGrammar);
    }
    validateNow();
}

void GrammarEditorDialog::buildUi() {
    setObjectName("grammarEditorDialog");
    setProperty("grammarEditor", true);
    setWindowTitle(mode_ == Mode::LL1 ? tr("Tu gramática — LL(1)")
                                      : tr("Tu gramática — SLR(1)"));
    setModal(true);
    resize(780, 580);
    setMinimumSize(660, 480);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(28, 24, 28, 24);
    rootLayout->setSpacing(16);

    auto* eyebrow = new QLabel(mode_ == Mode::LL1 ? tr("EJERCICIO LL(1)")
                                                  : tr("EJERCICIO SLR(1)"),
                               this);
    eyebrow->setObjectName("grammarEditorEyebrow");
    rootLayout->addWidget(eyebrow);

    auto* title = new QLabel(tr("Escribe tu gramática"), this);
    title->setObjectName("grammarEditorTitle");
    rootLayout->addWidget(title);

    auto* subtitle = new QLabel(
        tr("Cada regla termina con un punto y los símbolos se separan con "
           "espacios. El primer antecedente es el axioma, los antecedentes "
           "son los no terminales y el resto de símbolos son terminales. "
           "Usa | para alternativas y deja el consecuente vacío para "
           "épsilon (A -> .)."),
        this);
    subtitle->setObjectName("grammarEditorSubtitle");
    subtitle->setWordWrap(true);
    rootLayout->addWidget(subtitle);

    auto* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(16);

    input_ = new QPlainTextEdit(this);
    input_->setObjectName("grammarEditorInput");
    QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    mono.setPointSize(14);
    input_->setFont(mono);
    input_->setTabChangesFocus(true);
    input_->setPlaceholderText(tr("E -> E + T | T .\nT -> ( E ) | id ."));
    contentLayout->addWidget(input_, 1);

    auto* summaryPanel = new QFrame(this);
    summaryPanel->setObjectName("grammarEditorSummary");
    summaryPanel->setFixedWidth(220);
    auto* summaryLayout = new QVBoxLayout(summaryPanel);
    summaryLayout->setContentsMargins(16, 14, 16, 14);
    summaryLayout->setSpacing(4);

    summaryLayout->addWidget(makeSummaryCaption(tr("AXIOMA"), summaryPanel));
    axiomValue_ = makeSummaryValue(summaryPanel);
    summaryLayout->addWidget(axiomValue_);
    summaryLayout->addSpacing(8);

    summaryLayout->addWidget(
        makeSummaryCaption(tr("NO TERMINALES"), summaryPanel));
    nonTermValue_ = makeSummaryValue(summaryPanel);
    summaryLayout->addWidget(nonTermValue_);
    summaryLayout->addSpacing(8);

    summaryLayout->addWidget(
        makeSummaryCaption(tr("TERMINALES"), summaryPanel));
    termValue_ = makeSummaryValue(summaryPanel);
    summaryLayout->addWidget(termValue_);
    summaryLayout->addSpacing(8);

    summaryLayout->addWidget(makeSummaryCaption(tr("REGLAS"), summaryPanel));
    rulesValue_ = makeSummaryValue(summaryPanel);
    summaryLayout->addWidget(rulesValue_);
    summaryLayout->addStretch(1);

    contentLayout->addWidget(summaryPanel);
    rootLayout->addLayout(contentLayout, 1);

    statusLabel_ = new QLabel(this);
    statusLabel_->setObjectName("grammarEditorStatus");
    statusLabel_->setWordWrap(true);
    statusLabel_->setMinimumHeight(44);
    rootLayout->addWidget(statusLabel_);

    auto* footerLayout = new QHBoxLayout();
    footerLayout->setSpacing(12);
    footerLayout->addStretch(1);

    cancelButton_ = new QPushButton(tr("Cancelar"), this);
    cancelButton_->setObjectName("grammarEditorCancelButton");
    cancelButton_->setAutoDefault(false);
    cancelButton_->setCursor(Qt::PointingHandCursor);
    footerLayout->addWidget(cancelButton_);

    startButton_ = new QPushButton(tr("Comenzar ejercicio"), this);
    startButton_->setObjectName("grammarEditorStartButton");
    startButton_->setProperty("role", "primary");
    startButton_->setAutoDefault(false);
    startButton_->setEnabled(false);
    footerLayout->addWidget(startButton_);

    rootLayout->addLayout(footerLayout);

    highlighter_ = new GrammarSyntaxHighlighter(input_->document());

    connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);
    connect(startButton_, &QPushButton::clicked, this,
            &GrammarEditorDialog::accept);

    auto* submitShortcut =
        new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return), this);
    connect(submitShortcut, &QShortcut::activated, this,
            &GrammarEditorDialog::accept);

    clearSummary();
    input_->setFocus();
}

void GrammarEditorDialog::scheduleValidation() {
    debounce_.start();
}

void GrammarEditorDialog::validateNow() {
    debounce_.stop();
    const QString text = input_->toPlainText();

    GrammarParseResult result = GrammarParser::Parse(text.toStdString());
    if (!result.Ok()) {
        valid_ = false;
        startButton_->setEnabled(false);
        clearSummary();
        const bool emptyText =
            result.errors.size() == 1 &&
            result.errors.front().kind == GrammarParseError::Kind::EmptyGrammar;
        setStatus(describeParseErrors(result), emptyText ? "hint" : "error");
        return;
    }

    updateSummary(result.grammar);

    GrammarFactory checks;
    if (checks.IsInfinite(result.grammar)) {
        valid_ = false;
        startButton_->setEnabled(false);
        setStatus(tr("Hay símbolos que no generan ninguna cadena de "
                     "terminales. Revisa que cada no terminal tenga un caso "
                     "base."),
                  "error");
        return;
    }
    if (checks.HasUnreachableSymbols(result.grammar)) {
        valid_ = false;
        startButton_->setEnabled(false);
        setStatus(tr("Hay símbolos que no se pueden alcanzar desde el "
                     "axioma."),
                  "error");
        return;
    }

    if (mode_ == Mode::LL1) {
        LL1Parser ll1(result.grammar);
        if (!ll1.CreateLL1Table()) {
            valid_ = false;
            startButton_->setEnabled(false);
            setStatus(tr("La gramática no es LL(1): su tabla tiene "
                         "conflictos. Prueba a factorizar por la izquierda "
                         "o a eliminar la recursividad izquierda."),
                      "error");
            return;
        }
    } else {
        SLR1Parser slr1(result.grammar);
        if (!slr1.MakeParser()) {
            valid_ = false;
            startButton_->setEnabled(false);
            setStatus(tr("La gramática no es SLR(1): hay conflictos "
                         "shift/reduce o reduce/reduce."),
                      "error");
            return;
        }
    }

    grammar_ = result.grammar;
    valid_   = true;
    startButton_->setEnabled(true);

    const QString axiom = QString::fromStdString(grammar_.axiom_);
    const QString start =
        QString::fromStdString(grammar_.g_.at(grammar_.axiom_).at(0).at(0));
    setStatus(tr("Gramática %1 válida. Se añadirá la regla inicial "
                 "%2 → %3 $.")
                  .arg(mode_ == Mode::LL1 ? QStringLiteral("LL(1)")
                                          : QStringLiteral("SLR(1)"),
                       axiom, start),
              "ok");
}

void GrammarEditorDialog::updateSummary(const Grammar& grammar) {
    const QString axiom = QString::fromStdString(grammar.axiom_);
    const QString start =
        QString::fromStdString(grammar.g_.at(grammar.axiom_).at(0).at(0));

    QStringList   nonTerminals;
    QSet<QString> highlightSet;
    for (const std::string& nt : grammar.st_.non_terminals_) {
        const QString symbol = QString::fromStdString(nt);
        highlightSet.insert(symbol);
        if (symbol != axiom) {
            nonTerminals.append(symbol);
        }
    }
    std::sort(nonTerminals.begin(), nonTerminals.end());
    nonTerminals.removeAll(start);
    nonTerminals.prepend(start);

    QStringList terminals;
    for (const std::string& t : grammar.st_.terminals_wtho_eol_) {
        terminals.append(QString::fromStdString(t));
    }
    std::sort(terminals.begin(), terminals.end());

    int ruleCount = 0;
    for (const auto& [lhs, productions] : grammar.g_) {
        if (lhs != grammar.axiom_) {
            ruleCount += static_cast<int>(productions.size());
        }
    }

    axiomValue_->setText(start);
    nonTermValue_->setText(nonTerminals.join(QStringLiteral("  ")));
    termValue_->setText(terminals.join(QStringLiteral("  ")));
    rulesValue_->setText(QString::number(ruleCount));
    highlighter_->setNonTerminals(highlightSet);
}

void GrammarEditorDialog::clearSummary() {
    const QString dash = QStringLiteral("—");
    axiomValue_->setText(dash);
    nonTermValue_->setText(dash);
    termValue_->setText(dash);
    rulesValue_->setText(dash);
}

void GrammarEditorDialog::setStatus(const QString& message,
                                    const QString& state) {
    statusLabel_->setText(message);
    if (statusLabel_->property("state").toString() != state) {
        statusLabel_->setProperty("state", state);
        statusLabel_->style()->unpolish(statusLabel_);
        statusLabel_->style()->polish(statusLabel_);
    }
}

QString
GrammarEditorDialog::describeParseErrors(const GrammarParseResult& result) {
    QStringList messages;
    for (const GrammarParseError& error : result.errors) {
        if (messages.size() == kMaxListedErrors) {
            messages.append(tr("… y %1 errores más.")
                                .arg(result.errors.size() - kMaxListedErrors));
            break;
        }
        QString detail = QString::fromStdString(error.detail).simplified();
        if (detail.size() > 40) {
            detail = detail.left(39) + QStringLiteral("…");
        }
        switch (error.kind) {
        case GrammarParseError::Kind::EmptyGrammar:
            messages.append(
                tr("Escribe al menos una regla, por ejemplo: A -> a A | b ."));
            break;
        case GrammarParseError::Kind::MissingArrow:
            messages.append(tr("Línea %1: falta la flecha «->» en «%2».")
                                .arg(error.line)
                                .arg(detail));
            break;
        case GrammarParseError::Kind::MissingEndDot:
            messages.append(tr("Línea %1: falta el punto final en «%2».")
                                .arg(error.line)
                                .arg(detail));
            break;
        case GrammarParseError::Kind::EmptyLeftHandSide:
            messages.append(
                tr("Línea %1: falta el antecedente antes de la flecha.")
                    .arg(error.line));
            break;
        case GrammarParseError::Kind::MultipleLeftHandSide:
            messages.append(tr("Línea %1: el antecedente debe ser un único "
                               "símbolo, no «%2».")
                                .arg(error.line)
                                .arg(detail));
            break;
        case GrammarParseError::Kind::ExtraArrow:
            messages.append(tr("Línea %1: hay una flecha de más en «%2». "
                               "¿Olvidaste terminar la regla anterior con "
                               "un punto?")
                                .arg(error.line)
                                .arg(detail));
            break;
        case GrammarParseError::Kind::ReservedSymbol:
            messages.append(tr("Línea %1: el símbolo «%2» está reservado o "
                               "contiene caracteres no permitidos "
                               "(. , : ; | $).")
                                .arg(error.line)
                                .arg(detail));
            break;
        }
    }
    return messages.join(QStringLiteral("\n"));
}

void GrammarEditorDialog::accept() {
    if (!valid_) {
        validateNow();
        if (!valid_) {
            return;
        }
    }
    QSettings settings(kSettingsOrg, kSettingsApp);
    settings.setValue(kLastGrammarKey, input_->toPlainText());
    QDialog::accept();
}

#ifdef SYNTAXTUTOR_TESTING
void GrammarEditorDialog::setGrammarTextForTest(const QString& text) {
    input_->setPlainText(text);
    validateNow();
}
#endif
