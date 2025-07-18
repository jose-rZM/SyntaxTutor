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

#include "slrtutorwindow.h"
#include "tutorialmanager.h"
#include "ui_slrtutorwindow.h"
#include <QEasingCurve>
#include <QFontDatabase>
#include <sstream>

SLRTutorWindow::SLRTutorWindow(const Grammar& g, TutorialManager* tm,
                               QWidget* parent)
    : QMainWindow(parent), ui(new Ui::SLRTutorWindow), grammar(g), slr1(g),
      tm(tm) {
    // ====== Parser Initialization ============================
    slr1.MakeParser();

#ifdef QT_DEBUG
    grammar.Debug();
#endif

    // ====== Conflict & Reduction State Identification =========
    std::ranges::for_each(slr1.states_, [this](const state& st) {
        bool hasComplete   = false;
        bool hasIncomplete = false;
        for (const Lr0Item& it : st.items_) {
            if (it.IsComplete())
                hasComplete = true;
            else
                hasIncomplete = true;
            if (hasComplete && hasIncomplete &&
                it.antecedent_ != slr1.gr_.axiom_) {
                statesWithLr0Conflict.append(&st);
                conflictStatesIdQueue.push(st.id_);
                break;
            }
        }
    });

    std::ranges::for_each(slr1.states_, [this](const state& st) {
        if (statesWithLr0Conflict.contains(&st))
            return;
        for (const Lr0Item& it : st.items_) {
            if (it.IsComplete() && it.antecedent_ != slr1.gr_.axiom_) {
                reduceStatesIdQueue.push(st.id_);
                break;
            }
        }
    });

    // ====== UI Setup ==========================================
    ui->setupUi(this);

    // -- Confirm Button: Icon + Shadow
    ui->confirmButton->setIcon(QIcon(":/resources/send.svg"));
    auto* shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(10);
    shadow->setOffset(0);
    shadow->setColor(QColor::fromRgb(0, 200, 214));
    ui->confirmButton->setGraphicsEffect(shadow);

    ui->textEdit->setFont(QFontDatabase::font("Noto Sans", "Regular", 12));

    ui->userResponse->setFont(QFontDatabase::font("Noto Sans", "Regular", 12));
    ui->userResponse->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->userResponse->setPlaceholderText(
        tr("Introduce aquí tu respuesta. Ctrl + Enter para nueva línea."));

    // -- Chat Appearance
    QFont chatFont = QFontDatabase::font("Noto Sans", "Regular", 12);
    ui->listWidget->setFont(chatFont);
    ui->listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->listWidget->verticalScrollBar()->setSingleStep(10);

    // ====== Grammar Formatting =================================
    sortedNonTerminals =
        stdUnorderedSetToQSet(slr1.gr_.st_.non_terminals_).values();
    std::ranges::sort(sortedNonTerminals,
                      [](const QString& a, const QString& b) {
                          if (a == "S")
                              return true;
                          if (b == "S")
                              return false;
                          return a < b;
                      });
    fillSortedGrammar();
    formattedGrammar = FormatGrammar(grammar);

    ui->gr->setFont(QFontDatabase::font("Noto Sans", "Regular", 14));
    ui->gr->setText(formattedGrammar);

    // ====== Status, Progress & First Message ===================
    ui->cntRight->setText(QString::number(cntRightAnswers));
    ui->cntWrong->setText(QString::number(cntWrongAnswers));

    updateProgressPanel();
    addMessage(tr("La gramática es:\n") + formattedGrammar, false);

    currentState = StateSlr::A;
    addMessage(generateQuestion(), false);

    // ====== Signal Connections ==================================
    connect(ui->userResponse, &CustomTextEdit::sendRequested, this,
            &SLRTutorWindow::on_confirmButton_clicked);

    if (tm) {
        setupTutorial();
    }
}

SLRTutorWindow::~SLRTutorWindow() {
    delete ui;
}

void SLRTutorWindow::exportConversationToPdf(const QString& filePath) {
    QTextDocument doc;
    QString       html;

    html += R"(
        <html>
        <head>
        </head>
        <body>
    )";
    html += R"(
    <style>
    body {
        font-family: 'Noto Sans', sans-serif;
        font-size: 11pt;
        line-height: 1.6;
        margin: 20px;
    }

    h2 {
        font-size: 16pt;
        color: #393E46;
        border-bottom: 2px solid #ccc;
        padding-bottom: 5px;
        margin-top: 40px;
        margin-bottom: 20px;
    }

    h3 {
        font-size: 13pt;
        color: #007B8A;
        margin-top: 30px;
        margin-bottom: 10px;
    }

    .entry {
        border-left: 4px solid #007B8A;
        padding: 10px 15px;
        margin: 15px 0;
        border-radius: 4px;
    }

    .entry .role {
        font-weight: bold;
        margin-bottom: 6px;
        color: #2c3e50;
    }

    ul {
        padding-left: 20px;
        margin-bottom: 20px;
        font-family: 'Noto Sans', sans-serif;
        font-size: 11pt;
    }
    li {
        margin-bottom: 4px;
    }

    table {
        border-collapse: collapse;
        margin: 0 auto 20px auto;
        width: auto;
        font-size: 10.5pt;
    }

    th, td {
        border: 1px solid #999;
        padding: 6px 10px;
        text-align: center;
    }

    th {
        background-color: #f0f0f0;
        font-weight: bold;
    }

    td {
        background-color: #fafafa;
    }

    tr:nth-child(even) td {
        background-color: #f0f0f0;
    }

    .container {
        display: flex;
        justify-content: center;
        margin-bottom: 30px;
    }

    .page-break {
        page-break-before: always;
    }
    </style>
    )";
    html += "<div style='text-align: center; font-size: 8pt; color: #888; "
            "margin-top: 60px;'>";
    html += tr("Generado automáticamente por SyntaxTutor el ") +
            QDate::currentDate().toString("dd/MM/yyyy");
    html += "</div>";

    html += "<h2>" + tr("Conversación") + "</h2>";

    for (auto it = conversationLog.constBegin();
         it != conversationLog.constEnd(); ++it) {
        const MessageLog& message = *it;
        QString           safeText =
            message.message.toHtmlEscaped().replace("\n", "<br>");
        html += "<div class='entry'>";
        html += "<div class='role'>";
        html += (message.isUser ? tr("Usuario: ") : tr("Tutor: "));
        html += "</div>";
        if (!message.isCorrect) {
            html +=
                "<span style='background-color: red;'>" + safeText + "</span>";
        } else {
            html += safeText;
        }
        html += "</div>";
    }

    html += "</body></html>";
    html += R"(<div class='page-break'></div>)";

    html += "<h2>" + tr("Estados del Autómata") + "</h2>";
    for (size_t i = 0; i < userMadeStates.size(); ++i) {
        const state& st = *std::ranges::find_if(
            userMadeStates, [i](const state& st) { return st.id_ == i; });
        html += QString("<h3>%1 %2</h3><ul>").arg(tr("Estado")).arg(st.id_);
        for (const Lr0Item& item : st.items_) {
            html += "<li>" +
                    QString::fromStdString(item.ToString()).toHtmlEscaped() +
                    "</li>";
        }
        html += "</ul><br>";
    }
    html += R"(<div class='page-break'></div>)";

    html += "<h2>" + tr("Tabla de análisis SLR") + "</h2><br>";
    html +=
        R"(<div class="container"><table border='1' cellspacing='0' cellpadding='5'>)";
    html += "<tr><th>" + tr("Estado") + "</th>";
    std::vector<std::string> columns;
    columns.reserve(slr1.gr_.st_.terminals_.size() +
                    slr1.gr_.st_.non_terminals_.size());
    for (const auto& s : slr1.gr_.st_.terminals_) {
        if (s == slr1.gr_.st_.EPSILON_) {
            continue;
        }
        columns.push_back(s);
    }
    columns.insert(columns.end(), slr1.gr_.st_.non_terminals_.begin(),
                   slr1.gr_.st_.non_terminals_.end());

    for (const auto& symbol : columns) {
        html += "<th>" + QString::fromStdString(symbol) + "</th>";
    }
    html += "</tr>";

    for (unsigned state = 0; state < slr1.states_.size(); ++state) {
        html += "<tr><td align='center'>" + QString::number(state) + "</td>";
        const auto  action_entry = slr1.actions_.find(state);
        const auto  trans_entry  = slr1.transitions_.find(state);
        const auto& transitions  = trans_entry->second;
        for (const auto& symbol : columns) {
            QString    cell       = "-";
            const bool isTerminal = slr1.gr_.st_.IsTerminal(symbol);
            if (!isTerminal) {
                if (trans_entry != slr1.transitions_.end()) {
                    const auto it = transitions.find(symbol);
                    if (it != transitions.end()) {
                        cell =
                            QString::fromStdString(std::to_string(it->second));
                    }
                }
            } else {
                if (action_entry != slr1.actions_.end()) {
                    const auto action_it = action_entry->second.find(symbol);
                    if (action_it != action_entry->second.end()) {
                        switch (action_it->second.action) {
                        case SLR1Parser::Action::Accept:
                            cell = "A";
                            break;
                        case SLR1Parser::Action::Reduce:
                            cell = "R";
                            break;
                        case SLR1Parser::Action::Shift:
                            if (trans_entry != slr1.transitions_.end()) {
                                const auto shift_it = transitions.find(symbol);
                                if (shift_it != transitions.end()) {
                                    cell = QString::fromStdString(
                                        "S" + std::to_string(shift_it->second));
                                }
                            }
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
            html += "<td align='center'>" + cell + "</td>";
        }
        html += "</tr>";
    }
    html += "</table></div>";

    html += R"(<div class='page-break'></div>)";
    html += "<h2>" + tr("Acciones Reduce") + "</h2><br>";
    html += R"(<div class="container">)";
    html += "<table border='1' cellspacing='0' cellpadding='5'>";
    html += "<tr><th>" + tr("Estado") + "</th><th>" + tr("Símbolo") +
            "</th><th>" + tr("Regla") + "</th>";
    for (const auto& [state, actions] : slr1.actions_) {
        for (const auto& [symbol, action] : actions) {
            if (action.action == SLR1Parser::Action::Reduce) {
                std::string rule = action.item->antecedent_ + " → ";
                for (const auto& sym : action.item->consequent_) {
                    rule += sym + " ";
                }

                html += "<tr>";
                html +=
                    "<td align='center'>" + QString::number(state) + "</td>";
                html += "<td align='center'>" + QString::fromStdString(symbol) +
                        "</td>";
                html += "<td>" + QString::fromStdString(rule) + "</td>";
                html += "</tr>";
            }
        }
    }

    html += "</table></div>";
    doc.setHtml(html);

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(10, 10, 10, 10));

    doc.print(&printer);
}

void SLRTutorWindow::showTable() {
    QStringList colHeaders;
    for (const auto& symbol : slr1.gr_.st_.terminals_) {
        if (symbol == slr1.gr_.st_.EPSILON_)
            continue;
        colHeaders << QString::fromStdString(symbol);
    }
    for (const auto& symbol : slr1.gr_.st_.non_terminals_) {
        colHeaders << QString::fromStdString(symbol);
    }
    std::sort(colHeaders.begin(), colHeaders.end(),
              [](const QString& a, const QString& b) {
                  auto rank = [](const QString& s) -> int {
                      if (s == "$")
                          return 1;
                      if (!s.isEmpty() && s[0].isLower())
                          return 0;
                      return 2;
                  };

                  int ra = rank(a);
                  int rb = rank(b);
                  return (ra != rb) ? (ra < rb) : (a < b);
              });
    auto* dialog = new SLRTableDialog(slr1.states_.size(), colHeaders.size(),
                                      colHeaders, this, &rawTable);
    static const char* darkQss = R"(
    QDialog, QWidget {
        background-color: #2b2b2b;
        color: #e0e0e0;
    }
    QTableWidget {
        background-color: #1F1F1F;
        color: #E0E0E0;
        gridline-color: #555555;
    }
    QHeaderView::section {
        background-color: #313436;
        color: #E0E0E0;
        padding: 4px;
        border: 1px solid #555555;
    }
    QTableWidget::item:selected {
        background-color: #50575F;
        color: #ffffff;
    }
    QPushButton {
        background-color: #393E46;
        color: white;
        border: none;
        padding: 8px 20px;
        border-radius: 8px;
    }
    QPushButton:hover {
        background-color: #50575F;
    }
    QPushButton:pressed {
        background-color: #222831;
    }
    )";
    dialog->setStyleSheet(darkQss);

    connect(dialog, &QDialog::accepted, this, [this, dialog, colHeaders]() {
        rawTable = dialog->getTableData();

        slrtable.clear();

        const int nTerm =
            slr1.gr_.st_.terminals_.contains(slr1.gr_.st_.EPSILON_)
                ? slr1.gr_.st_.terminals_.size() - 1
                : slr1.gr_.st_.terminals_.size();
        for (int state = 0; state < rawTable.size(); ++state) {
            for (int j = 0; j < rawTable[state].size(); ++j) {
                QString cell = rawTable[state][j].trimmed();
                if (cell.isEmpty())
                    continue;

                const QString sym = colHeaders[j];

                if (j < nTerm) {
                    // --- Action with terminal ---
                    if (cell.startsWith('s', Qt::CaseInsensitive)) {
                        int toState          = cell.mid(1).toInt();
                        slrtable[state][sym] = ActionEntry::makeShift(toState);
                    } else if (cell.startsWith('r', Qt::CaseInsensitive)) {
                        int prodIdx          = cell.mid(1).toInt();
                        slrtable[state][sym] = ActionEntry::makeReduce(prodIdx);
                    } else if (cell.compare("acc", Qt::CaseInsensitive) == 0) {
                        slrtable[state][sym] = ActionEntry::makeAccept();
                    } else {
                        qWarning() << "Entrada no reconocida en Action["
                                   << state << "][" << sym << "]:" << cell;
                    }
                } else {
                    // --- Goto with non-terminal ---
                    bool ok      = false;
                    int  toState = cell.toInt(&ok);
                    if (ok) {
                        slrtable[state][sym] = ActionEntry::makeGoto(toState);
                    } else {
                        qWarning() << "Goto inválido en [" << state << "]["
                                   << sym << "]:" << cell;
                    }
                }
            }
        }

        on_confirmButton_clicked();
        dialog->deleteLater();
    });

    connect(dialog, &QDialog::rejected, this, [this, dialog]() {
        rawTable.clear();
        QMessageBox msg(this);
        msg.setWindowTitle(tr("Cancelar ejercicio SLR(1)"));
        msg.setTextFormat(Qt::RichText);
        msg.setText(tr(
            "¿Quieres salir del tutor? Esto cancelará el ejercicio."
            " Si lo que quieres es enviar tu respuesta, pulsa \"Finalizar\"."));

        // 2) Configura los botones
        msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msg.setDefaultButton(QMessageBox::No);

        msg.setStyleSheet(R"(
  QMessageBox {
    background-color: #1F1F1F;
    color: #EEEEEE;
    font-family: 'Noto Sans';
  }
  QMessageBox QLabel {
    color: #EEEEEE;
  }
)");
        QAbstractButton* yesBtn = msg.button(QMessageBox::Yes);
        QAbstractButton* noBtn  = msg.button(QMessageBox::No);

        if (yesBtn) {
            yesBtn->setText(tr("Sí"));
            yesBtn->setCursor(Qt::PointingHandCursor);
            yesBtn->setIcon(QIcon());
            yesBtn->setStyleSheet(R"(
      QPushButton {
        background-color: #00ADB5;
        color: white;
        border: none;
        padding: 6px 14px;
        border-radius: 4px;
        font-weight: bold;
        font-family: 'Noto Sans';
      }
      QPushButton:hover {
        background-color: #00CED1;
      }
      QPushButton:pressed {
        background-color: #007F86;
      }
    )");
        }

        if (noBtn) {
            noBtn->setText(tr("No"));
            noBtn->setCursor(Qt::PointingHandCursor);
            noBtn->setIcon(QIcon());
            noBtn->setStyleSheet(R"(
      QPushButton {
        background-color: #D9534F;
        color: white;
        border: none;
        padding: 6px 14px;
        border-radius: 4px;
        font: 'Noto Sans';
        font-weight: bold;
      }
      QPushButton:hover {
        background-color: #E14E50;
      }
      QPushButton:pressed {
        background-color: #C12E2A;
      }
    )");
        }

        int ret = msg.exec();
        if (ret == QMessageBox::Yes) {
            this->close();
        } else {
            showTable();
        }
        dialog->deleteLater();
    });
    dialog->show();
}

void SLRTutorWindow::updateProgressPanel() {
    int scrollPos = ui->textEdit->verticalScrollBar()->value();

    QString text;

    text += R"(
        <html>
        <body style="font-family: 'Noto Sans'; color: #f0f0f0; background-color: #1e1e1e;">
    )";

    if (userMadeStates.empty()) {
        text += "<div style='color:#aaaaaa;'>" +
                tr("No se han construido estados aún.") + "</div>";
    } else {
        for (size_t i = 0; i < slr1.states_.size(); ++i) {
            auto st = std::ranges::find_if(
                userMadeStates, [i](const state& st) { return st.id_ == i; });

            if (st != userMadeStates.end()) {
                text += QString("<div style='color:#00ADB5; font-weight:bold; "
                                "margin-top:12px;'>%1 I%2:</div>")
                            .arg(tr("Estado"))
                            .arg((*st).id_);

                text += "<p style='margin-left:12px; margin-top:4px;'>";
                for (const Lr0Item& item : (*st).items_) {
                    text += QString::fromStdString(item.ToString()) + "<br/>";
                }
                text += "</p>";

                auto it = userMadeTransitions.find((*st).id_);
                if (it != userMadeTransitions.end() && !it->second.empty()) {
                    text += "<div style='margin-left:12px; color:#BBBBBB; "
                            "margin-top:4px;'>" +
                            tr("Transiciones:") +
                            "</div><ul "
                            "style='list-style-type:circle; color:#777777; "
                            "margin-left:20px;'>";
                    for (const auto& entry : it->second) {
                        const QString symbol =
                            QString::fromStdString(entry.first);
                        const int target = entry.second;
                        text += QString("<li> δ(I%1, %2) = I%3</li>")
                                    .arg((*st).id_)
                                    .arg(symbol)
                                    .arg(target);
                    }
                    text += "</ul>";
                }
            }
        }
    }

    text += "</body></html>";

    ui->textEdit->setHtml(text);
    ui->textEdit->verticalScrollBar()->setValue(scrollPos);
}

void SLRTutorWindow::addUserState(unsigned id) {
    auto st = std::ranges::find_if(
        slr1.states_, [id](const state& st) { return st.id_ == id; });
    if (st != slr1.states_.end()) {
        userMadeStates.insert(*st);
        updateProgressPanel();
    }
}

void SLRTutorWindow::addUserTransition(unsigned           fromId,
                                       const std::string& symbol,
                                       unsigned           toId) {
    userMadeTransitions[fromId][symbol] = toId;
}

void SLRTutorWindow::addMessage(const QString& text, bool isUser) {
    if (!isUser && text.isEmpty()) {
        return;
    }
    QString messageText = text;
    // LOG
    if (messageText.isEmpty()) {
        messageText = tr("No se proporcionó respuesta.");
    }
    conversationLog.emplaceBack(messageText, isUser);
    if (isUser) {
        lastUserMessageLogIdx = conversationLog.size() - 1;
    }
    QWidget*     messageWidget = new QWidget;
    QVBoxLayout* mainLayout    = new QVBoxLayout;
    mainLayout->setSpacing(2);
    mainLayout->setContentsMargins(10, 5, 10, 5);

    QLabel* header = new QLabel(isUser ? tr("Usuario") : "Tutor");
    header->setAlignment(isUser ? Qt::AlignRight : Qt::AlignLeft);
    header->setFont(QFontDatabase::font("Noto Sans", "Regular", 10));
    header->setStyleSheet(isUser ? "font-weight: bold; color: #00ADB5;"
                                 : "font-weight: bold; color: #BBBBBB;");

    QHBoxLayout* messageLayout = new QHBoxLayout;
    messageLayout->setSpacing(0);

    QVBoxLayout* innerLayout = new QVBoxLayout;
    innerLayout->setSpacing(0);

    QLabel* label = new QLabel(messageText);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    QFontMetrics fm(label->font());
    int          textWidth = fm.boundingRect(0, 0, ui->listWidget->width(), 0,
                                             Qt::TextWordWrap, text)
                        .width();

    int maxWidth      = ui->listWidget->width() * 0.8;
    int adjustedWidth = qMin(textWidth + 32, maxWidth);
    label->setMaximumWidth(adjustedWidth);
    label->setMinimumWidth(300);

    if (isUser) {
        if (text.isEmpty()) {
            label->setFont(QFontDatabase::font("Noto Sans", "Italic", 12));
            label->setStyleSheet(R"(
            background-color: #00ADB5;
            color: white;
            padding: 12px 16px;
            border-top-left-radius: 18px;
            border-top-right-radius: 0px;
            border-bottom-left-radius: 18px;
            border-bottom-right-radius: 18px;
            border: 1px solid rgba(0, 0, 0, 0.15);
        )");
        } else {
            label->setFont(QFontDatabase::font("Noto Sans", "Regular", 12));
            label->setStyleSheet(R"(
            background-color: #00ADB5;
            color: white;
            padding: 12px 16px;
            border-top-left-radius: 18px;
            border-top-right-radius: 0px;
            border-bottom-left-radius: 18px;
            border-bottom-right-radius: 18px;
            border: 1px solid rgba(0, 0, 0, 0.15);
        )");
        }
    } else {
        label->setFont(QFontDatabase::font("Noto Sans", "Regular", 12));
        label->setStyleSheet(R"(
            background-color: #2F3542;
            color: #F1F1F1;
            padding: 12px 16px;
            border-top-left-radius: 0px;
            border-top-right-radius: 18px;
            border-bottom-left-radius: 18px;
            border-bottom-right-radius: 18px;
            border: 1px solid rgba(255, 255, 255, 0.05);
        )");
    }
    label->setAlignment(Qt::AlignJustify);
    label->adjustSize();

    QLabel* timestamp = new QLabel(QTime::currentTime().toString("HH:mm"));
    timestamp->setFont(QFontDatabase::font("Noto Sans", "Regular", 10));
    timestamp->setStyleSheet("color: gray; margin-left: 5px;");
    timestamp->setAlignment(Qt::AlignRight);

    innerLayout->addWidget(label);
    innerLayout->addWidget(timestamp);

    if (isUser) {
        messageLayout->addStretch();
        messageLayout->addLayout(innerLayout);
    } else {
        messageLayout->addLayout(innerLayout);
        messageLayout->addStretch();
    }

    mainLayout->addWidget(header);
    mainLayout->addLayout(messageLayout);

    messageWidget->setLayout(mainLayout);
    messageWidget->setSizePolicy(QSizePolicy::Preferred,
                                 QSizePolicy::MinimumExpanding);
    messageWidget->adjustSize();
    messageWidget->updateGeometry();

    QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(messageWidget->sizeHint());

    if (isUser) {
        lastUserMessage = messageWidget;
    }

    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, messageWidget);
    ui->listWidget->update();
    ui->listWidget->scrollToBottom();
}

void SLRTutorWindow::wrongAnimation() {
    if (lastUserMessage == nullptr) {
        return;
    }
    QList<QLabel*> labels = lastUserMessage->findChildren<QLabel*>();
    if (labels.size() > 1) {
        QLabel* label = labels[1];

        auto* effect = new QGraphicsColorizeEffect(label);
        effect->setColor(Qt::red);
        label->setGraphicsEffect(effect);

        auto* animation = new QPropertyAnimation(effect, "strength");
        animation->setDuration(1000);
        animation->setKeyValueAt(0, 0.0);
        animation->setKeyValueAt(0.5, 1.0);
        animation->setKeyValueAt(1, 0.0);

        animation->start(QAbstractAnimation::DeleteWhenStopped);

        QObject::connect(animation, &QPropertyAnimation::finished, [label]() {
            if (label && label->graphicsEffect()) {
                label->graphicsEffect()->deleteLater();
                label->setGraphicsEffect(nullptr);
            }
        });
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void SLRTutorWindow::wrongUserResponseAnimation() {
    if (m_shakeAnimation &&
        m_shakeAnimation->state() == QAbstractAnimation::Running)
        return;

    QPoint originalPos = ui->userResponse->pos();

    QPropertyAnimation* animation =
        new QPropertyAnimation(ui->userResponse, "pos");
    m_shakeAnimation = animation;
    animation->setDuration(200);
    animation->setLoopCount(1);

    animation->setKeyValueAt(0, originalPos);
    animation->setKeyValueAt(0.2, originalPos + QPoint(4, 0));
    animation->setKeyValueAt(0.4, originalPos - QPoint(4, 0));
    animation->setKeyValueAt(0.6, originalPos + QPoint(3, 0));
    animation->setKeyValueAt(0.8, originalPos - QPoint(3, 0));
    animation->setKeyValueAt(1, originalPos);

    animation->setEasingCurve(QEasingCurve::OutBounce);
    connect(animation, &QAbstractAnimation::finished, this,
            [this, originalPos]() {
                ui->userResponse->move(originalPos);
                m_shakeAnimation = nullptr;
            });

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void SLRTutorWindow::animateLabelPop(QLabel* label) {
    auto* runningAnim =
        label->property("popAnimation").value<QPropertyAnimation*>();
    if (runningAnim && runningAnim->state() == QAbstractAnimation::Running)
        return;

    QRect originalRect;
    if (label->property("popOriginalRect").isValid()) {
        originalRect = label->property("popOriginalRect").toRect();
    } else {
        originalRect = label->geometry();
        label->setProperty("popOriginalRect", originalRect);
    }

    QRect expandedRect(originalRect.x() - 3, originalRect.y() - 2,
                       originalRect.width() + 5, originalRect.height() + 4);

    auto* anim = new QPropertyAnimation(label, "geometry", this);
    label->setProperty("popAnimation", QVariant::fromValue(anim));

    anim->setDuration(200);
    anim->setKeyValueAt(0.0, originalRect);
    anim->setKeyValueAt(0.5, expandedRect);
    anim->setKeyValueAt(1.0, originalRect);
    anim->setEasingCurve(QEasingCurve::OutCubic);

    connect(anim, &QAbstractAnimation::finished, this, [label]() {
        const QRect orig = label->property("popOriginalRect").toRect();
        label->setGeometry(orig);
        label->setProperty("popAnimation", QVariant());
    });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void SLRTutorWindow::animateLabelColor(QLabel*       label,
                                       const QColor& flashColor) {
    int durationMs = 400;

    if (!label->property("originalStyle").isValid()) {
        label->setProperty("originalStyle", label->styleSheet());
    }

    QString flashStyle = QString("color: %1;").arg(flashColor.name());
    label->setStyleSheet(flashStyle);

    // Cancel previous timers
    const auto children = label->children();
    for (QObject* child : children) {
        if (auto oldTimer = qobject_cast<QTimer*>(child)) {
            oldTimer->stop();
            oldTimer->deleteLater();
        }
    }

    QTimer* resetTimer = new QTimer(label);
    resetTimer->setSingleShot(true);
    QObject::connect(
        resetTimer, &QTimer::timeout, label, [label, resetTimer]() {
            label->setStyleSheet(label->property("originalStyle").toString());
            resetTimer->deleteLater();
        });

    resetTimer->start(durationMs);
}

void SLRTutorWindow::markLastUserIncorrect() {
    if (!lastUserMessage)
        return;

    QList<QLabel*> labels = lastUserMessage->findChildren<QLabel*>();

    if (labels.isEmpty() || labels.size() < 2) {
        return;
    }
    labels[1]->setStyleSheet(R"(
        background-color: #CC3333;
        color: white;
        padding: 12px 16px;
        border-top-left-radius: 18px;
        border-top-right-radius: 0px;
        border-bottom-left-radius: 18px;
        border-bottom-right-radius: 18px;
        border: 1px solid rgba(0, 0, 0, 0.15);
    )");
}

void SLRTutorWindow::on_confirmButton_clicked() {
    QString userResponse;
    bool    isCorrect;

    if (currentState != StateSlr::H && currentState != StateSlr::H_prime) {
        userResponse = ui->userResponse->toPlainText().trimmed();
        addMessage(userResponse, true);
        isCorrect = verifyResponse(userResponse);
    } else {
        isCorrect = verifyResponse("");
    }

    if (!isCorrect) {
        ui->cntWrong->setText(QString::number(++cntWrongAnswers));
        animateLabelPop(ui->cross);
        animateLabelColor(ui->cross, QColor::fromRgb(204, 51, 51));
        addMessage(feedback(), false);
        wrongAnimation();
        wrongUserResponseAnimation();
        markLastUserIncorrect();
        if (lastUserMessageLogIdx != -1) {
            conversationLog[lastUserMessageLogIdx].toggleIsCorrect();
            lastUserMessageLogIdx = -1;
        }
        lastUserMessage = nullptr;
    } else {
        ui->cntRight->setText(QString::number(++cntRightAnswers));
        animateLabelPop(ui->tick);
        animateLabelColor(ui->tick, QColor::fromRgb(0, 204, 102));
    }
    updateState(isCorrect);
    if (currentState == StateSlr::fin) {
        QMessageBox end(this);
        end.setWindowTitle(tr("Fin del ejercicio"));
        end.setText(tr("¿Exportar a PDF?"));
        end.setInformativeText(
            tr("Se generará un PDF con toda la conversación, estados "
               "calculados y la tabla SLR(1)."));

        end.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        end.setDefaultButton(QMessageBox::No);

        QAbstractButton* yesBtn = end.button(QMessageBox::Yes);
        QAbstractButton* noBtn  = end.button(QMessageBox::No);

        if (yesBtn) {
            yesBtn->setText(tr("Sí"));
            yesBtn->setCursor(Qt::PointingHandCursor);
            yesBtn->setIcon(QIcon());
            yesBtn->setStyleSheet(R"(
      QPushButton {
        background-color: #00ADB5;
        color: white;
        border: none;
        padding: 6px 14px;
        border-radius: 4px;
        font-weight: bold;
        font-family: 'Noto Sans';
      }
      QPushButton:hover {
        background-color: #00CED1;
      }
      QPushButton:pressed {
        background-color: #007F86;
      }
        )");
        }

        if (noBtn) {
            noBtn->setText(tr("No"));
            noBtn->setCursor(Qt::PointingHandCursor);
            noBtn->setIcon(QIcon());
            noBtn->setStyleSheet(R"(
      QPushButton {
        background-color: #D9534F;
        color: white;
        border: none;
        padding: 6px 14px;
        border-radius: 4px;
        font: 'Noto Sans';
        font-weight: bold;
      }
      QPushButton:hover {
        background-color: #E14E50;
      }
      QPushButton:pressed {
        background-color: #C12E2A;
      }
    )");
        }

        int ret = end.exec();
        if (ret == QMessageBox::Yes) {
            QString filePath = QFileDialog::getSaveFileName(
                this, tr("Guardar conversación"), "conver.pdf",
                tr("Archivo PDF (*.pdf)"));

            if (!filePath.isEmpty()) {
                exportConversationToPdf(filePath);
            }
        }
        close();
    }
    addMessage(generateQuestion(), false);
    ui->userResponse->clear();
}

/************************************************************
 *                 QUESTION GENERATION (SLR)                *
 * Returns a context-sensitive question string based on
 * the currentState of the tutor.
 ************************************************************/
#include "slrwizard.h"
QString SLRTutorWindow::generateQuestion() {
    switch (currentState) {
    // ======= A: Initial Item and Closure ========================
    case StateSlr::A:
        return tr("¿Cuál es el estado inicial del analizador?\n"
                  "Formato (Ctrl + Enter para nueva línea):\n  X → a·b\n  X → "
                  "·b\n  X → EPSILON·");

    case StateSlr::A1:
        return tr("¿Cuál es el axioma de la gramática?");

    case StateSlr::A2:
        return tr("Dado el ítem:  S -> · A $\n"
                  "¿Qué símbolo aparece justo después del punto (·)?");

    case StateSlr::A3:
        return tr("Si ese símbolo es un no terminal,\n"
                  "¿cuáles son las reglas cuyo antecedente es ese símbolo?");

    case StateSlr::A4:
        return tr("¿Cuál es el cierre del ítem inicial?");

    case StateSlr::A_prime:
        return tr("Entonces, ¿cuál es el estado inicial generado?");

        // ======= B: State Counting ================================
    case StateSlr::B:
        return tr("¿Cuántos estados se han generado en la colección LR(0) "
                  "hasta ahora?");

        // ======= C: State Analysis ================================
    case StateSlr::C: {
        // Load current state to be inspected
        currentStateId = statesIdQueue.front();
        statesIdQueue.pop();

        auto currState =
            std::ranges::find_if(slr1.states_, [&](const state& st) {
                return st.id_ == currentStateId;
            });
        currentSlrState = *currState;

        return tr("Estado I%1:\n%2\n"
                  "¿Cuántos ítems contiene este estado?")
            .arg(currentStateId)
            .arg(QString::fromStdString(
                slr1.PrintItems(currentSlrState.items_)));
    }

        // ======= CA: Symbols after dot ============================
    case StateSlr::CA:
        return tr("¿Qué símbolos aparecen después del punto (·) en los ítems "
                  "de este estado?\n"
                  "Formato: a,b,c");

        // ======= CB: Transition generation (δ) ====================
    case StateSlr::CB: {
        QString currentSymbol = followSymbols.at(currentFollowSymbolsIdx);
        if (currentSymbol.toStdString() == slr1.gr_.st_.EPSILON_) {
            return tr("Calcula δ(I%1, %2):\n"
                      "Deja la entrada vacía si el resultado es vacío.")
                .arg(currentStateId)
                .arg(currentSymbol);
        } else {
            nextStateId =
                slr1.transitions_.at(static_cast<unsigned int>(currentStateId))
                    .at(currentSymbol.toStdString());
            statesIdQueue.push(nextStateId);
            return tr("Calcula δ(I%1, %2):\n"
                      "¿Qué estado se genera al hacer transición con '%2'?\n"
                      "Este será el estado número %3.")
                .arg(currentStateId)
                .arg(currentSymbol)
                .arg(nextStateId);
        }
    }

        // ======= D: Table Structure ===============================
    case StateSlr::D:
        return tr("¿Cuántas filas y columnas tiene la tabla SLR(1)?\n"
                  "Formato: filas,columnas");

    case StateSlr::D1:
        return tr("¿Cuántos estados contiene la colección LR(0)?");

    case StateSlr::D2:
        return tr("¿Cuántos símbolos terminales y no terminales hay en la "
                  "gramática?\n"
                  "(Excluye ε. Incluye $)");

    case StateSlr::D_prime:
        return tr(
            "Con los datos anteriores,\n"
            "¿cuál es el tamaño total (filas,columnas) de la tabla SLR(1)?");

        // ======= E: Completed Items ===============================
    case StateSlr::E:
        return tr("¿Cuántos estados contienen al menos un ítem completo?");

    case StateSlr::E1:
        return tr("Indica los ID de los estados con ítems completos, separados "
                  "por comas.\n"
                  "Ejemplo: 2,5,7");

    case StateSlr::E2:
        return tr("Indica cuántos ítems completos tiene cada estado.\n"
                  "Formato: id1:n1, id2:n2, ...");

        // ======= F: LR(0) Conflicts ===============================
    case StateSlr::F:
        return tr("¿Qué estados presentan un CONFLICTO LR(0)?\n"
                  "Deja la respuesta vacía si no hay conflictos.\n"
                  "Formato: 1,3,7");

    case StateSlr::FA: {
        currentConflictStateId = conflictStatesIdQueue.front();

        auto it = std::ranges::find_if(slr1.states_, [&](const state& st) {
            return st.id_ == currentConflictStateId;
        });
        currentConflictState = *it;

        return tr("Estado I%1 con conflicto LR(0):\n%2\n"
                  "Indica los símbolos terminales sobre los que debe aplicarse "
                  "REDUCCIÓN.\n"
                  "Formato: a,b,c (vacío si ninguno).")
            .arg(currentConflictStateId)
            .arg(QString::fromStdString(
                slr1.PrintItems(currentConflictState.items_)));
    }

        // ======= G: Reduction States ==============================
    case StateSlr::G: {
        currentReduceStateId = reduceStatesIdQueue.front();

        auto it = std::ranges::find_if(slr1.states_, [&](const state& st) {
            return st.id_ == currentReduceStateId;
        });
        currentReduceState = *it;

        return tr("Estado I%1:\n%2\n"
                  "Indica los terminales sobre los que se aplicará REDUCCIÓN.\n"
                  "Formato: a,b,c — vacío si no se aplica en ninguno.")
            .arg(currentReduceStateId)
            .arg(QString::fromStdString(
                slr1.PrintItems(currentReduceState.items_)));
    }

    case StateSlr::H: {
        lastUserMessage = nullptr;
        ui->userResponse->setDisabled(true);
        ui->confirmButton->setDisabled(true); // handled externally
        showTable();
        return "";
    }

    case StateSlr::H_prime: {
        QStringList colHeaders;

        for (const auto& symbol : slr1.gr_.st_.terminals_) {
            if (symbol == slr1.gr_.st_.EPSILON_)
                continue;
            colHeaders << QString::fromStdString(symbol);
        }

        for (const auto& symbol : slr1.gr_.st_.non_terminals_) {
            colHeaders << QString::fromStdString(symbol);
        }
        auto* wizard =
            new SLRWizard(slr1, rawTable, colHeaders, sortedGrammar, this);
        wizard->setStyleSheet(R"(
    * {
        background-color: #2b2b2b;
        color: #e0e0e0;
    }
    QWizard, QWizardPage {

            background-color: #1F1F1F;
            color: #E0E0E0;

    }

    QWizard::title {
        color: #ffffff;
        font-size: 18px;
    }

    QWizard::subTitle {
        color: #cccccc;
        font-size: 14px;
        margin-bottom: 12px;
    }

    QLabel {
        color: #e0e0e0;
    }

    QLineEdit {
        background-color: #3c3f41;
        color: #e0e0e0;
        border: 1px solid #555555;
        border-radius: 4px;
        padding: 4px;
    }

    QWizard::WizardButton {
        background-color: #444444;
        color: #ffffff;
        border: none;
        padding: 6px 12px;
        border-radius: 4px;
    }

    QWizard::WizardButton:hover {
        background-color: #555555;
    }

    QWizard::WizardButton:pressed {
            background-color: #333333;
    }
    )");
        connect(wizard, &QWizard::accepted, this,
                [this]() { on_confirmButton_clicked(); });
        connect(wizard, &QWizard::rejected, this, [this, wizard]() {
            QMessageBox msg(this);
            msg.setWindowTitle(tr("Cancelar ejercicio SLR(1)"));
            msg.setTextFormat(Qt::RichText);
            msg.setText(
                tr("¿Quieres salir del tutor? Esto cancelará el ejercicio."
                   " Si lo que quieres es enviar tu respuesta, pulsa "
                   "\"Finalizar\"."));

            // 2) Configura los botones
            msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msg.setDefaultButton(QMessageBox::No);

            msg.setStyleSheet(R"(
  QMessageBox {
    background-color: #1F1F1F;
    color: #EEEEEE;
    font-family: 'Noto Sans';
  }
  QMessageBox QLabel {
    color: #EEEEEE;
  }
)");
            QAbstractButton* yesBtn = msg.button(QMessageBox::Yes);
            QAbstractButton* noBtn  = msg.button(QMessageBox::No);

            if (yesBtn) {
                yesBtn->setText(tr("Sí"));
                yesBtn->setCursor(Qt::PointingHandCursor);
                yesBtn->setIcon(QIcon());
                yesBtn->setStyleSheet(R"(
      QPushButton {
        background-color: #00ADB5;
        color: white;
        border: none;
        padding: 6px 14px;
        border-radius: 4px;
        font-weight: bold;
        font-family: 'Noto Sans';
      }
      QPushButton:hover {
        background-color: #00CED1;
      }
      QPushButton:pressed {
        background-color: #007F86;
      }
    )");
            }

            if (noBtn) {
                noBtn->setText(tr("No"));
                noBtn->setCursor(Qt::PointingHandCursor);
                noBtn->setIcon(QIcon());
                noBtn->setStyleSheet(R"(
      QPushButton {
        background-color: #D9534F;
        color: white;
        border: none;
        padding: 6px 14px;
        border-radius: 4px;
        font: 'Noto Sans';
        font-weight: bold;
      }
      QPushButton:hover {
        background-color: #E14E50;
      }
      QPushButton:pressed {
        background-color: #C12E2A;
      }
    )");
            }
            int ret = msg.exec();
            if (ret == QMessageBox::Yes) {
                this->close();
                return "";
            } else {
                on_confirmButton_clicked();
                wizard->deleteLater();
                return "";
            }
        });
        wizard->show();
    }
        return "";

    default:
        return "";
    }
}

/************************************************************
 *                     STATE TRANSITION                    *
 * Advances the tutor's finite state machine based on the
 * currentState and whether the user's response was correct.
 ************************************************************/
void SLRTutorWindow::updateState(bool isCorrect) {
    switch (currentState) {
    // ====== A: Initial item and closure construction ========
    case StateSlr::A: {
        currentState = isCorrect ? StateSlr::B : StateSlr::A1;
        if (isCorrect) {
            addUserState(0);
            statesIdQueue.push(0);
        }
        break;
    }

    case StateSlr::A1:
        currentState = isCorrect ? StateSlr::A2 : StateSlr::A1;
        break;

    case StateSlr::A2:
        currentState = isCorrect ? StateSlr::A3 : StateSlr::A2;
        break;

    case StateSlr::A3:
        currentState = isCorrect ? StateSlr::A4 : StateSlr::A3;
        break;

    case StateSlr::A4:
        currentState = isCorrect ? StateSlr::A_prime : StateSlr::A4;
        break;

    case StateSlr::A_prime: {
        currentState = StateSlr::B;
        addUserState(0);
        statesIdQueue.push(0);
        break;
    }

        // ====== B: State loop (C → CA → CB per state) ===========
    case StateSlr::B:
        if (statesIdQueue.empty()) {
            currentState = StateSlr::D;
        } else {
            currentState = StateSlr::C;
        }
        break;

    case StateSlr::C:
        currentState = StateSlr::CA;
        break;

    case StateSlr::CA:
        if (!isCorrect) {
            currentState = StateSlr::CA;
        } else if (!followSymbols.empty() &&
                   currentFollowSymbolsIdx < followSymbols.size()) {
            currentState = StateSlr::CB;
        } else {
            currentState = StateSlr::B;
        }
        break;

    case StateSlr::CB: {
        if (followSymbols.at(currentFollowSymbolsIdx).toStdString() !=
            slr1.gr_.st_.EPSILON_) {
            addUserTransition(
                currentStateId,
                followSymbols[currentFollowSymbolsIdx].toStdString(),
                nextStateId);
        }
        ++currentFollowSymbolsIdx;
        if (currentFollowSymbolsIdx < followSymbols.size()) {
            currentState = StateSlr::CB;
        } else {
            followSymbols.clear();
            currentFollowSymbolsIdx = 0;
            currentState            = StateSlr::B;
        }
        addUserState(nextStateId);
        break;
    }

        // ====== D: Table structure questions =====================
    case StateSlr::D:
        currentState = isCorrect ? StateSlr::E : StateSlr::D1;
        break;

    case StateSlr::D1:
        currentState = isCorrect ? StateSlr::D2 : StateSlr::D1;
        break;

    case StateSlr::D2:
        currentState = isCorrect ? StateSlr::D_prime : StateSlr::D2;
        break;

    case StateSlr::D_prime:
        currentState = StateSlr::E;
        break;

        // ====== E: Completed item states =========================
    case StateSlr::E:
        currentState = isCorrect ? StateSlr::F : StateSlr::E1;
        break;

    case StateSlr::E1:
        currentState = isCorrect ? StateSlr::E2 : StateSlr::E1;
        break;

    case StateSlr::E2:
        currentState = StateSlr::F;
        break;

        // ====== F: LR(0) conflict detection ======================
    case StateSlr::F: {
        if (!isCorrect) {
            currentState = StateSlr::F; // Retry
            break;
        }

        // Initialize conflict resolution queue
        conflictStatesIdQueue = {};
        for (const state* st : std::as_const(statesWithLr0Conflict))
            conflictStatesIdQueue.push(st->id_);

        currentState =
            conflictStatesIdQueue.empty() ? StateSlr::G : StateSlr::FA;
        break;
    }

        // ====== FA: Resolve conflicts via FOLLOW ================
    case StateSlr::FA:
        if (!isCorrect) {
            currentState = StateSlr::FA;
        } else {
            if (!conflictStatesIdQueue.empty())
                conflictStatesIdQueue.pop();

            currentState =
                conflictStatesIdQueue.empty() ? StateSlr::G : StateSlr::FA;
        }
        break;

        // ====== G: Apply REDUCE for completed states ============
    case StateSlr::G:
        if (!isCorrect) {
            currentState = StateSlr::G;
        } else {
            if (!reduceStatesIdQueue.empty())
                reduceStatesIdQueue.pop();

            currentState =
                reduceStatesIdQueue.empty() ? StateSlr::H : StateSlr::G;
        }
        break;

    case StateSlr::H:
        currentState = isCorrect ? StateSlr::fin : StateSlr::H_prime;
        break;

    case StateSlr::H_prime:
        currentState = StateSlr::H;
        break;

        // ====== Final state ======================================
    case StateSlr::fin:
        break;
    }
}

/************************************************************
 *                  VERIFY USER RESPONSES                   *
 * Dispatches the current user input to the appropriate
 * response verification handler based on the tutor's state.
 ************************************************************/
bool SLRTutorWindow::verifyResponse(const QString& userResponse) {
    switch (currentState) {
    // ====== A: Initial item and closure steps ===============
    case StateSlr::A:
        return verifyResponseForA(userResponse);
    case StateSlr::A1:
        return verifyResponseForA1(userResponse);
    case StateSlr::A2:
        return verifyResponseForA2(userResponse);
    case StateSlr::A3:
        return verifyResponseForA3(userResponse);
    case StateSlr::A4:
        return verifyResponseForA4(userResponse);
    case StateSlr::A_prime:
        return verifyResponseForA(userResponse);

        // ====== B: State count ==================================
    case StateSlr::B:
        return verifyResponseForB(userResponse);

        // ====== C: Item analysis ================================
    case StateSlr::C:
        return verifyResponseForC(userResponse);
    case StateSlr::CA:
        return verifyResponseForCA(userResponse);
    case StateSlr::CB:
        return verifyResponseForCB(userResponse);

        // ====== D: Table size / symbols =========================
    case StateSlr::D:
        return verifyResponseForD(userResponse);
    case StateSlr::D1:
        return verifyResponseForD1(userResponse);
    case StateSlr::D2:
        return verifyResponseForD2(userResponse);
    case StateSlr::D_prime:
        return verifyResponseForD(userResponse);

        // ====== E: Completed item tracking ======================
    case StateSlr::E:
        return verifyResponseForE(userResponse);
    case StateSlr::E1:
        return verifyResponseForE1(userResponse);
    case StateSlr::E2:
        return verifyResponseForE2(userResponse);

        // ====== F: Conflict resolution ==========================
    case StateSlr::F:
        return verifyResponseForF(userResponse);
    case StateSlr::FA:
        return verifyResponseForFA(userResponse);

        // ====== G: Reduction action =============================
    case StateSlr::G:
        return verifyResponseForG(userResponse);

    case StateSlr::H:
        return verifyResponseForH();

    case StateSlr::H_prime:
        return true;

        // ====== Final / undefined state =========================
    default:
        return false;
    }
}

bool SLRTutorWindow::verifyResponseForA(const QString& userResponse) {
    std::unordered_set<Lr0Item> response = ingestUserItems(userResponse);
    return response == solutionForA();
}

bool SLRTutorWindow::verifyResponseForA1(const QString& userResponse) {
    return userResponse == solutionForA1();
}

bool SLRTutorWindow::verifyResponseForA2(const QString& userResponse) {
    return userResponse == solutionForA2();
}

bool SLRTutorWindow::verifyResponseForA3(const QString& userResponse) {
    const auto response = ingestUserRules(userResponse);
    return response == solutionForA3();
}

bool SLRTutorWindow::verifyResponseForA4(const QString& userResponse) {
    std::unordered_set<Lr0Item> response = ingestUserItems(userResponse);
    return response == solutionForA4();
}

bool SLRTutorWindow::verifyResponseForB(const QString& userResponse) {
    unsigned response = userResponse.toUInt();
    return response == solutionForB();
}

bool SLRTutorWindow::verifyResponseForC(const QString& userResponse) {
    unsigned response = userResponse.toUInt();
    return response == solutionForC();
}

bool SLRTutorWindow::verifyResponseForCA(const QString& userResponse) {
    QStringList   response = userResponse.split(",", Qt::SkipEmptyParts);
    QSet<QString> responseSet;

    for (const auto& s : std::as_const(response)) {
        QString trimmed = s.trimmed();
        if (responseSet.contains(trimmed)) {
            return false;
        }
        responseSet.insert(trimmed);
    }
    QStringList   expected = solutionForCA();
    QSet<QString> expectedSet(expected.begin(), expected.end());
    return responseSet == expectedSet;
}

bool SLRTutorWindow::verifyResponseForCB(const QString& userResponse) {
    if (followSymbols.at(currentFollowSymbolsIdx).toStdString() ==
        slr1.gr_.st_.EPSILON_) {
        return userResponse.isEmpty();
    } else {
        const auto response = ingestUserItems(userResponse);
        return response == solutionForCB();
    }
}

bool SLRTutorWindow::verifyResponseForD(const QString& userResponse) {
    return userResponse == solutionForD();
}

bool SLRTutorWindow::verifyResponseForD1(const QString& userResponse) {
    return userResponse == solutionForD1();
}

bool SLRTutorWindow::verifyResponseForD2(const QString& userResponse) {
    return userResponse == solutionForD2();
}

bool SLRTutorWindow::verifyResponseForE(const QString& userResponse) {
    bool ok    = false;
    int  value = userResponse.toInt(&ok);
    return ok && value == static_cast<int>(solutionForE());
}

bool SLRTutorWindow::verifyResponseForE1(const QString& userResponse) {
    QSet<unsigned> expected = solutionForE1();
    QStringList    parts    = userResponse.split(',', Qt::SkipEmptyParts);
    QSet<unsigned> given;

    for (const QString& s : std::as_const(parts)) {
        bool     ok;
        unsigned id = s.trimmed().toUInt(&ok);
        if (!ok)
            return false;
        given.insert(id);
    }
    return given == expected;
}

bool SLRTutorWindow::verifyResponseForE2(const QString& userResponse) {
    QStringList userResponseSplitted =
        userResponse.split(',', Qt::SkipEmptyParts);
    QMap<unsigned, unsigned> expected = solutionForE2();
    QMap<unsigned, unsigned> given;

    for (const QString& pair : std::as_const(userResponseSplitted)) {
        auto kv = pair.split(':', Qt::SkipEmptyParts);
        if (kv.size() != 2)
            return false;

        bool     ok1, ok2;
        unsigned id  = kv[0].trimmed().toUInt(&ok1);
        unsigned num = kv[1].trimmed().toUInt(&ok2);
        if (!ok1 || !ok2)
            return false;

        given[id] = num;
    }
    return given == expected;
}

bool SLRTutorWindow::verifyResponseForF(const QString& userResponse) {
    QSet<unsigned> expected = solutionForF(); // ids con conflicto
    QStringList    parts    = userResponse.split(',', Qt::SkipEmptyParts);
    QSet<unsigned> given;
    for (const QString& s : std::as_const(parts)) {
        bool     ok;
        unsigned id = s.trimmed().toUInt(&ok);
        if (!ok)
            return false;
        given.insert(id);
    }
    return given == expected;
}

bool SLRTutorWindow::verifyResponseForFA(const QString& userResponse) {
    QStringList   resp = userResponse.split(',', Qt::SkipEmptyParts);
    QSet<QString> userSet;
    for (const auto& s : std::as_const(resp)) {
        userSet.insert(s.trimmed());
    }
    QSet<QString> expected = solutionForFA();
    return userSet == expected;
}

bool SLRTutorWindow::verifyResponseForG(const QString& userResponse) {
    QSet<QString> expected = solutionForG();
    QStringList   resp     = userResponse.split(',', Qt::SkipEmptyParts);
    QSet<QString> userSet;

    for (const auto& s : std::as_const(resp)) {
        userSet.insert(s.trimmed());
    }
    return userSet == expected;
}

bool SLRTutorWindow::verifyResponseForH() {
    if (slrtable.empty()) {
        return false;
    }

    for (const state& slrState : slr1.states_) {
        unsigned int state = slrState.id_;
        for (const auto& terminal : slr1.gr_.st_.terminals_) {
            if (terminal == slr1.gr_.st_.EPSILON_)
                continue;
            QString sym = QString::fromStdString(terminal);

            const auto&          actMap = slr1.actions_.at(state);
            auto                 itAct  = actMap.find(terminal);
            SLR1Parser::s_action expectedAct =
                (itAct != actMap.end()
                     ? itAct->second
                     : SLR1Parser::s_action{nullptr,
                                            SLR1Parser::Action::Empty});

            auto userIt    = slrtable[state].find(sym);
            bool userEmpty = (userIt == slrtable[state].end());

            if (expectedAct.action == SLR1Parser::Action::Empty && userEmpty) {
                continue;
            }
            if (expectedAct.action == SLR1Parser::Action::Empty || userEmpty) {
                return false;
            }

            const ActionEntry& entry = userIt.value();
            switch (expectedAct.action) {
            case SLR1Parser::Action::Shift: {
                const auto& transMap = slr1.transitions_.at(state);
                auto        itTrans  = transMap.find(terminal);
                if (itTrans == transMap.end() ||
                    entry.type != ActionEntry::Shift ||
                    entry.target != static_cast<int>(itTrans->second)) {
                    return false;
                }
                break;
            }
            case SLR1Parser::Action::Reduce: {
                const Lr0Item* itItem = expectedAct.item;
                // Search in sortedGrammar the production given by item
                int prodIdx = -1;
                for (int k = 0; k < sortedGrammar.size(); ++k) {
                    const auto& rule = sortedGrammar[k];
                    if (rule.first.toStdString() == itItem->antecedent_ &&
                        stdVectorToQVector(itItem->consequent_) ==
                            rule.second) {
                        prodIdx = k;
                        break;
                    }
                }
                if (prodIdx < 0 || entry.type != ActionEntry::Reduce ||
                    entry.target != prodIdx) {
                    return false;
                }
                break;
            }
            case SLR1Parser::Action::Accept: {
                if (entry.type != ActionEntry::Accept) {
                    return false;
                }
                break;
            }
            default:
                return false;
            }
        }

        for (const auto& nonTerm : slr1.gr_.st_.non_terminals_) {
            QString sym = QString::fromStdString(nonTerm);

            auto userIt    = slrtable[state].find(sym);
            bool userEmpty = (userIt == slrtable[state].end());

            if (!slr1.transitions_.contains(state) && userEmpty) {
                continue;
            }

            const auto   transMap      = slr1.transitions_.at(state);
            auto         itTrans       = transMap.find(nonTerm);
            bool         hasGoto       = (itTrans != transMap.end());
            unsigned int expectedState = hasGoto ? itTrans->second : 0;

            if (!hasGoto && userEmpty) {
                continue;
            }
            if (!hasGoto || userEmpty) {
                return false;
            }

            const ActionEntry& entry = userIt.value();
            if (entry.type != ActionEntry::Goto ||
                static_cast<unsigned int>(entry.target) != expectedState) {
                return false;
            }
        }
    }
    return true;
}

/************************************************************
 *                         SOLUTIONS                        *
 ************************************************************/

std::unordered_set<Lr0Item> SLRTutorWindow::solutionForA() {
    const auto expected = std::ranges::find_if(
        slr1.states_, [](const state& st) { return st.id_ == 0; });
    return expected->items_;
}

QString SLRTutorWindow::solutionForA1() {
    return QString::fromStdString(grammar.axiom_);
}

QString SLRTutorWindow::solutionForA2() {
    return QString::fromStdString(grammar.g_.at(grammar.axiom_).at(0).at(0));
}

std::vector<std::pair<std::string, std::vector<std::string>>>
SLRTutorWindow::solutionForA3() {
    std::string next = grammar.g_.at(grammar.axiom_).at(0).at(0);
    std::vector<std::pair<std::string, std::vector<std::string>>> result;

    const auto& rules = grammar.g_.at(next);
    for (const auto& rhs : rules) {
        result.emplace_back(next, rhs);
    }

    return result;
}

std::unordered_set<Lr0Item> SLRTutorWindow::solutionForA4() {
    std::unordered_set<Lr0Item> items;
    items.emplace(grammar.axiom_, grammar.g_.at(grammar.axiom_).at(0),
                  grammar.st_.EPSILON_, grammar.st_.EOL_);
    slr1.Closure(items);
    return items;
}

unsigned SLRTutorWindow::solutionForB() {
    return userMadeStates.size();
}

unsigned SLRTutorWindow::solutionForC() {
    return currentSlrState.items_.size();
}

QStringList SLRTutorWindow::solutionForCA() {
    QSet<QString> following_symbols;
    bool          acceptingItem = false;

    std::ranges::for_each(currentSlrState.items_, [&](const Lr0Item& item) {
        if (item.NextToDot() == slr1.gr_.st_.EPSILON_ &&
            item.antecedent_ == slr1.gr_.axiom_) {
            acceptingItem = true;
        } else {
            following_symbols.insert(QString::fromStdString(item.NextToDot()));
        }
    });

    // FILL FOLLOW SYMBOLS FOR CB QUESTION
    followSymbols           = following_symbols.values();
    currentFollowSymbolsIdx = 0;

    QStringList result = followSymbols;
    if (acceptingItem) {
        result.append(QString::fromStdString(slr1.gr_.st_.EOL_));
    }
    followSymbols.sort();
    return result;
}

std::unordered_set<Lr0Item> SLRTutorWindow::solutionForCB() {
    auto st = std::ranges::find_if(slr1.states_, [this](const state& st) {
                  return st.id_ == nextStateId;
              })->items_;
    return st;
}

QString SLRTutorWindow::solutionForD() {
    return QString("%1,%2").arg(solutionForD1()).arg(solutionForD2());
}

QString SLRTutorWindow::solutionForD1() {
    return QString::number(slr1.states_.size());
}

QString SLRTutorWindow::solutionForD2() {
    unsigned terminals = slr1.gr_.st_.terminals_.contains(slr1.gr_.st_.EPSILON_)
                             ? slr1.gr_.st_.terminals_.size() - 1
                             : slr1.gr_.st_.terminals_.size();
    unsigned non_terminals = slr1.gr_.st_.non_terminals_.size();
    return QString::number(terminals + non_terminals);
}

std::ptrdiff_t SLRTutorWindow::solutionForE() {
    return std::ranges::count_if(slr1.states_, [](const state& st) {
        return std::ranges::any_of(
            st.items_, [](const Lr0Item& item) { return item.IsComplete(); });
    });
}

QSet<unsigned> SLRTutorWindow::solutionForE1() {
    QSet<unsigned> ids;
    for (const state& st : slr1.states_) {
        if (std::ranges::any_of(
                st.items_, [](const Lr0Item& it) { return it.IsComplete(); }))
            ids.insert(st.id_);
    }
    return ids;
}

QMap<unsigned, unsigned> SLRTutorWindow::solutionForE2() {
    QMap<unsigned, unsigned> result;
    for (const state& st : slr1.states_) {
        unsigned count = 0;
        for (const Lr0Item& it : st.items_)
            if (it.IsComplete())
                ++count;
        if (count)
            result[st.id_] = count;
    }
    return result;
}

QSet<unsigned> SLRTutorWindow::solutionForF() {
    QSet<unsigned> ids;
    for (const state* st : std::as_const(statesWithLr0Conflict))
        ids.insert(st->id_);
    return ids;
}

QSet<QString> SLRTutorWindow::solutionForFA() {
    QSet<QString> symbols;

    for (const Lr0Item& it : currentConflictState.items_) {
        if (!it.IsComplete())
            continue;

        // FOLLOW del antecedente
        std::unordered_set<std::string> fol = slr1.Follow(it.antecedent_);

        for (const std::string& s : fol)
            symbols.insert(QString::fromStdString(s));
    }
    return symbols;
}

QSet<QString> SLRTutorWindow::solutionForG() {
    QSet<QString> symbols;
    for (const Lr0Item& it : currentReduceState.items_) {
        if (!it.IsComplete())
            continue;
        std::unordered_set<std::string> fol = slr1.Follow(it.antecedent_);
        for (const std::string& s : fol)
            symbols.insert(QString::fromStdString(s));
    }
    symbols.remove(QString::fromStdString(slr1.gr_.st_.EPSILON_));
    return symbols;
}

/************************************************************
 *                          FEEDBACK                        *
 * Returns pedagogical feedback based on the current state.
 * This is shown to the user when their answer is incorrect,
 * or optionally when help is requested.
 ************************************************************/
QString SLRTutorWindow::feedback() {
    switch (currentState) {
    // ====== A: Initial item and closure ======================
    case StateSlr::A:
        return feedbackForA();
    case StateSlr::A1:
        return feedbackForA1();
    case StateSlr::A2:
        return feedbackForA2();
    case StateSlr::A3:
        return feedbackForA3();
    case StateSlr::A4:
        return feedbackForA4();
    case StateSlr::A_prime:
        return feedbackForAPrime();

        // ====== B: Number of LR(0) states ========================
    case StateSlr::B:
        return feedbackForB();

        // ====== C: State analysis (items and transitions) ========
    case StateSlr::C:
        return feedbackForC();
    case StateSlr::CA:
        return feedbackForCA();
    case StateSlr::CB:
        return feedbackForCB();

        // ====== D: Table structure ===============================
    case StateSlr::D:
        return feedbackForD();
    case StateSlr::D1:
        return feedbackForD1();
    case StateSlr::D2:
        return feedbackForD2();
    case StateSlr::D_prime:
        return feedbackForDPrime();

        // ====== E: Completed items per state =====================
    case StateSlr::E:
        return feedbackForE();
    case StateSlr::E1:
        return feedbackForE1();
    case StateSlr::E2:
        return feedbackForE2();

        // ====== F: Conflict detection and resolution =============
    case StateSlr::F:
        return feedbackForF();
    case StateSlr::FA:
        return feedbackForFA();

        // ====== G: Apply REDUCE actions ==========================
    case StateSlr::G:
        return feedbackForG();

    case StateSlr::H:
        return tr("La tabla no es correcta.");

        // ====== Undefined state fallback =========================
    default:
        return tr("Error interno. Estado actual desconocido a la hora de dar "
                  "retroalimentación.");
    }
}

QString SLRTutorWindow::feedbackForA() {
    return tr("El estado inicial se construye a partir del cierre del ítem "
              "asociado al axioma: S -> · "
              "A $. Esto representa que aún no se ha leído nada y se quiere "
              "derivar desde el símbolo "
              "inicial.");
}

QString SLRTutorWindow::feedbackForA1() {
    return tr("El axioma es el símbolo desde el que comienza toda la "
              "derivación. En esta "
              "gramática, el axioma es: %1.")
        .arg(QString::fromStdString(grammar.axiom_));
}

QString SLRTutorWindow::feedbackForA2() {
    return tr("El símbolo que sigue al (·) indica cuál es el siguiente símbolo "
              "que debe ser "
              "procesado. En este ítem, ese símbolo es: %1.")
        .arg(QString::fromStdString(grammar.g_.at(grammar.axiom_).at(0).at(0)));
}

QString SLRTutorWindow::feedbackForA3() {
    QString antecedent =
        QString::fromStdString(grammar.g_.at(grammar.axiom_).at(0).at(0));
    QString result =
        tr("Como el símbolo tras el · es %1, se debe expandir sus producciones "
           "en el cierre. Las reglas cuyo antecedente es %1 son:\n")
            .arg(antecedent);

    for (auto it = sortedGrammar.constBegin(); it != sortedGrammar.constEnd();
         ++it) {
        if (it->first == antecedent) {
            QStringList symbols = it->second.toList();
            QString     ruleStr =
                QString("%1 -> %2").arg(it->first, symbols.join(" "));
            result += ruleStr + "\n";
        }
    }

    return result;
}

QString SLRTutorWindow::feedbackForA4() {
    Lr0Item init{grammar.axiom_, grammar.g_.at(grammar.axiom_).at(0),
                 grammar.st_.EPSILON_, grammar.st_.EOL_};
    std::unordered_set<Lr0Item> item{init};
    return tr("El cierre incluye todas las producciones de los no terminales "
              "que aparecen tras el "
              "·, "
              "añadidas recursivamente.\n") +
           QString(TeachClosure(item));
}

QString SLRTutorWindow::feedbackForAPrime() {
    const auto& st = std::ranges::find_if(
        slr1.states_, [](const state& st) { return st.id_ == 0; });
    QString items = QString::fromStdString(slr1.PrintItems(st->items_));
    return tr("El estado inicial (I0) es el cierre del ítem con el axioma. "
              "Contiene todos los "
              "ítems posibles a partir de ese punto.\n%1")
        .arg(items);
}

QString SLRTutorWindow::feedbackForB() {
    return tr("Se ha(n) generado %1 estado(s) hasta ahora. Cada transición "
              "sobre un símbolo "
              "genera un nuevo estado si lleva a un conjunto distinto de "
              "ítems.")
        .arg(userMadeStates.size());
}

QString SLRTutorWindow::feedbackForC() {
    return tr("El estado I%2 contiene %1 ítem(s).")
        .arg(currentSlrState.items_.size())
        .arg(currentStateId);
}

QString SLRTutorWindow::feedbackForCA() {
    QStringList expected = solutionForCA();
    QString     text     = ui->userResponse->toPlainText().trimmed();
    QStringList resp =
        text.split(',', Qt::SkipEmptyParts).replaceInStrings(re, "");
    QSet<QString> setResp(resp.begin(), resp.end());
    QSet<QString> duplicates;
    for (const QString& part : std::as_const(resp)) {
        QString trimmed = part.trimmed();
        if (resp.count(trimmed) > 1)
            duplicates.insert(trimmed);
        setResp.insert(trimmed);
        resp.append(trimmed);
    }
    QSet<QString> setSol(expected.begin(), expected.end());

    QString base;
    if (std::ranges::any_of(currentSlrState.items_, [](const Lr0Item& item) {
            return item.IsComplete();
        })) {
        base = tr("Los símbolos son: %1.\nCuando un ítem es de la forma X → α· "
                  "o X "
                  "-> EPSILON · "
                  "(ítem completo), el símbolo siguiente es siempre EPSILON. "
                  "En estos casos "
                  "podrás aplicar un reduce, recuérdalo.")
                   .arg(expected.join(", "));
    } else {
        base = tr("Los símbolos que aparecen tras el punto (·) en los ítems "
                  "determinan "
                  "posibles transiciones. En este estado, esos símbolos son: "
                  "%1.")
                   .arg(expected.join(", "));
    }
    if (text.isEmpty()) {
        return tr("No has indicado ningún símbolo.\n") + base;
    }
    if (resp.size() == 1 && resp[0].contains(' ')) {
        return tr("Recuerda separar los símbolos con comas.\n") + base;
    }

    QSet<QString> missing = setSol - setResp;
    QSet<QString> rest    = setResp - setSol;
    QString       msg;
    if (!duplicates.isEmpty()) {
        msg += tr("Has repetido símbolos: %1.\n")
                   .arg(QStringList(duplicates.values()).join(", "));
    }
    if (!missing.isEmpty()) {
        msg += tr("Te han faltado símbolos.\n");
    }
    if (!rest.isEmpty()) {
        msg += tr("No aparecen tras el punto: %1.\n")
                   .arg(QStringList(rest.values()).join(", "));
    }

    return msg + base;
}

QString SLRTutorWindow::feedbackForCB() {
    return QString(TeachDeltaFunction(currentSlrState.items_,
                                      followSymbols[currentFollowSymbolsIdx]));
}

QString SLRTutorWindow::feedbackForD() {
    return tr("La tabla SLR(1) tiene una fila por cada estado y columnas por "
              "cada símbolo "
              "terminal y no terminal (sin ε).");
}

QString SLRTutorWindow::feedbackForD1() {
    int     actual       = slr1.states_.size();
    QString feedbackBase = tr("Se han generado %1 estados.").arg(actual);

    QString text    = ui->userResponse->toPlainText().trimmed();
    bool    ok      = false;
    int     userVal = text.toInt(&ok);

    if (text.isEmpty()) {
        return tr("No has indicado ningún número de estados. Escribe un "
                  "entero. En el panel derecho "
                  "tienes todos los estados que has generado.");
    }
    if (!ok) {
        return tr("Formato inválido: escribe sólo dígitos para el número de "
                  "estados.\n") +
               feedbackBase;
    }
    if (userVal != actual) {
        return tr("Has escrito %1, pero el número real de estados en el "
                  "autómata es %2.\n"
                  "Recuerda que en el panel derecho tienes todos los estados "
                  "(y transiciones) "
                  "que has generado.")
                   .arg(userVal)
                   .arg(actual) +
               "\n" + feedbackBase;
    }
    return feedbackBase;
}

QString SLRTutorWindow::feedbackForD2() {
    int     actual       = solutionForD2().toInt();
    QString feedbackBase = tr("Hay un total de %1 símbolos gramaticales, "
                              "excluyendo EPSILON e incluyendo $.")
                               .arg(actual);

    QString text    = ui->userResponse->toPlainText().trimmed();
    bool    ok      = false;
    int     userVal = text.toInt(&ok);

    if (text.isEmpty()) {
        return tr("No has indicado ningún número de símbolos. Escribe un valor "
                  "entero.");
    }
    if (!ok) {
        return tr(
            "Formato incorrecto: usa sólo dígitos para el conteo de símbolos.");
    }
    if (userVal != actual) {
        return tr("Has puesto %1 símbolos, pero deberías contar tanto los "
                  "terminales como los "
                  "no terminales "
                  "excluyendo EPSILON e incluyendo $, lo que da %2.\n"
                  "Revisa tu conjunto de símbolos de la gramática.")
                   .arg(userVal)
                   .arg(actual) +
               "\n" + feedbackBase;
    }
    return feedbackBase;
}

QString SLRTutorWindow::feedbackForDPrime() {
    int     solRows      = solutionForD1().toInt();
    int     solCols      = solutionForD2().toInt();
    QString feedbackBase = tr("La tabla SLR(1) tiene %1 filas (estados) y %2 "
                              "columnas (símbolos, sin ε y con $).")
                               .arg(solRows)
                               .arg(solCols);

    QString     text  = ui->userResponse->toPlainText().trimmed();
    QStringList parts = text.split(',', Qt::SkipEmptyParts);
    if (text.isEmpty()) {
        return tr("No has indicado ningún valor. Debías escribir "
                  "filas,columnas separados por "
                  "coma.\n") +
               feedbackBase;
    }
    if (parts.size() != 2) {
        return tr("Formato inválido: se esperaban dos valores separados por "
                  "una coma, p.ej. "
                  "“5,12”.\n") +
               feedbackBase;
    }

    bool ok1 = false, ok2 = false;
    int  userRows = parts[0].trimmed().toInt(&ok1);
    int  userCols = parts[1].trimmed().toInt(&ok2);
    if (!ok1 || !ok2) {
        return tr("Ambos valores debían ser enteros.\n") + feedbackBase;
    }

    QString msg;
    if (userRows != solRows) {
        msg += tr("Número de filas: pusiste %1 pero hay %2 estados.\n")
                   .arg(userRows)
                   .arg(solRows);
    }
    if (userCols != solCols) {
        msg += tr("Número de columnas: pusiste %1 pero hay %2 símbolos.\n")
                   .arg(userCols)
                   .arg(solCols);
    }

    return feedbackBase;
}

QString SLRTutorWindow::feedbackForE() {
    return tr("Un estado es candidato para una acción REDUCE si contiene algún "
              "ítem de la forma X -> "
              "α · o X -> EPSILON ·, es decir, con el punto al final (ítem "
              "completo).");
}

QString SLRTutorWindow::feedbackForE1() {
    QSet<unsigned> sol = solutionForE1();

    QString text = ui->userResponse->toPlainText().trimmed();
    if (text.isEmpty()) {
        return tr("No has indicado ningún estado. Debes listar los IDs "
                  "separados por comas.\n"
                  "Recuerda que solo los estados con ítems completos pueden "
                  "hacer REDUCE.");
    }
    QStringList    parts = text.split(',', Qt::SkipEmptyParts);
    QSet<unsigned> user;
    for (auto& p : parts) {
        bool     ok = false;
        unsigned v  = p.trimmed().toUInt(&ok);
        if (!ok) {
            return tr("Formato inválido: cada ID debe ser un número entero. "
                      "Usa comas para separar.\n"
                      "Ejemplo: 2,5,7");
        }
        user.insert(v);
    }

    QSet<unsigned> missing = sol - user;
    QSet<unsigned> rest    = user - sol;
    QString        msg;
    QStringList    missingList, restList;
    for (const unsigned val : std::as_const(missing)) {
        missingList.append(QString::number(val));
    }
    for (const unsigned val : std::as_const(rest)) {
        restList.append(QString::number(val));
    }
    if (!missing.isEmpty()) {
        msg += tr("Te faltan estos estados con REDUCE posible: ") +
               missingList.join(", ") + ".\n";
    }
    if (!rest.isEmpty()) {
        msg += tr("Has incluido estados sin ítems completos: ") +
               restList.join(", ") + ".\n";
    }

    return msg + tr("Solo los estados con ítems completos (punto al final) "
                    "pueden hacer REDUCE.");
}

QString SLRTutorWindow::feedbackForE2() {
    QMap<unsigned int, unsigned int> sol = solutionForE2();
    QStringList                      pairs;
    for (auto it = sol.cbegin(); it != sol.cend(); ++it)
        pairs << QString("%1:%2").arg(it.key()).arg(it.value());
    return tr("Detalle de ítems completos por estado → ") + pairs.join(", ");
}

QString SLRTutorWindow::feedbackForF() {
    QString txt = tr("Un conflicto LR(0) ocurre cuando un mismo estado "
                     "contiene tanto ítems completos "
                     "(REDUCE) "
                     "como ítems con símbolo tras el · (SHIFT).");

    QSet<unsigned> sol = solutionForF();

    QString text = ui->userResponse->toPlainText().trimmed();
    if (text.isEmpty()) {
        return txt + tr("\nNo has listado ningún estado conflictivo. ");
    }
    QStringList    parts = text.split(',', Qt::SkipEmptyParts);
    QSet<unsigned> user;
    for (auto& p : parts) {
        bool     ok = false;
        unsigned v  = p.trimmed().toUInt(&ok);
        if (!ok) {
            return tr("Formato inválido: usa números separados por comas.");
        }
        user.insert(v);
    }

    QSet<unsigned> missing = sol - user;
    QSet<unsigned> rest    = user - sol;
    QString        msg;
    QStringList    missingList, restList;
    for (const unsigned val : std::as_const(missing)) {
        missingList.append(QString::number(val));
    }
    for (const unsigned val : std::as_const(rest)) {
        restList.append(QString::number(val));
    }
    if (!sol.isEmpty()) {
        if (!missing.isEmpty()) {
            msg += tr("Faltan estos estados conflictivos: ") +
                   missingList.join(", ") + ".\n";
        }
        if (!rest.isEmpty()) {
            msg += tr("Has marcado estados sin conflicto: ") +
                   restList.join(", ") +
                   tr(". En esos estados no hay confusión sobre qué acción "
                      "aplicar.\n");
        }
    } else {
        msg += tr("No debería haber conflictos, pero has listado algunos.\n");
    }
    if (msg.isEmpty()) {
        msg = tr("Revisa cada estado en busca de ítems completos y "
                 "desplazables juntos.");
    }

    return txt + "\n" + msg;
}

QString SLRTutorWindow::feedbackForFA() {
    unsigned stId    = currentConflictStateId;
    QString  txtBase = tr("En el estado I%1 se produce un conflicto LR(0). Un "
                           "ítem completo compite con otro "
                           "desplazable. Debes escribir los terminales en los "
                           "que la tabla aplicará REDUCE. Los puedes calcular "
                           "calculando SIG del antecedente.\n")
                          .arg(stId);

    QStringList   sol = solutionForFA().values();
    QSet<QString> solSet(sol.begin(), sol.end());

    QString text = ui->userResponse->toPlainText().trimmed();
    if (text.isEmpty()) {
        return txtBase + tr("No has indicado ningún terminal.\n");
    }
    QStringList   parts = text.split(',', Qt::SkipEmptyParts);
    QSet<QString> user;
    for (auto& p : parts) {
        user.insert(p.trimmed());
    }

    QSet<QString> missing = solSet - user;
    QSet<QString> rest    = user - solSet;
    QString       msg;
    if (!missing.isEmpty()) {
        msg += tr("Te faltan estos terminales para REDUCE: ") +
               QStringList(missing.values()).join(", ") + ".\n";
    }
    if (!rest.isEmpty()) {
        msg += tr("Estos no se usan para REDUCE en I%1: ").arg(stId) +
               QStringList(rest.values()).join(", ") + ".\n";
    }
    if (msg.isEmpty()) {
        msg = tr("Recuerda que solo se reduce en los terminales de SIG; en los "
                 "demás se realiza "
                 "SHIFT. Puedes apoyarte en la definición de SIG.\n");
    }

    return txtBase + msg;
}

QString SLRTutorWindow::feedbackForG() {
    unsigned stId = currentReduceStateId;
    QString  txtBase =
        tr("En el estado I%1, se aplica REDUCE solo en ciertos terminales.\n")
            .arg(stId);

    QStringList   sol = solutionForG().values();
    QSet<QString> solSet(sol.begin(), sol.end());

    QString text = ui->userResponse->toPlainText().trimmed();
    if (text.isEmpty()) {
        return txtBase + tr("No has listado ningún terminal para REDUCE.\n");
    }
    QStringList   parts = text.split(',', Qt::SkipEmptyParts);
    QSet<QString> user;
    for (auto& p : parts) {
        user.insert(p.trimmed());
    }

    QSet<QString> missing = solSet - user;
    QSet<QString> rest    = user - solSet;
    QString       msg;
    if (!missing.isEmpty()) {
        msg += tr("Faltan estos terminales: ") +
               QStringList(missing.values()).join(", ") + ".\n";
    }
    if (!rest.isEmpty()) {
        msg += tr("No deberías hacer REDUCE en: ") +
               QStringList(rest.values()).join(", ") +
               tr(". ¡No pertenecen al conjunto de símbolos siguientes!\n");
    }
    if (msg.isEmpty()) {
        msg = tr("Asegúrate de usar solo los terminales en SIG del antecedente "
                 "de la producción.");
    }

    return txtBase + msg;
}

/************************************************************
 *                         HELPERS                          *
 ************************************************************/

// HELPER FUNCTIONS
// ----------------------------------------------------------------------
std::vector<std::string>
SLRTutorWindow::qvectorToStdVector(const QVector<QString>& qvec) {
    std::vector<std::string> result;
    result.reserve(qvec.size());
    for (const auto& qstr : qvec) {
        result.push_back(qstr.toStdString());
    }
    return result;
}

// Convierte std::vector<std::string> a QVector<QString>
QVector<QString>
SLRTutorWindow::stdVectorToQVector(const std::vector<std::string>& vec) {
    QVector<QString> result;
    result.reserve(vec.size());
    for (const auto& str : vec) {
        result.push_back(QString::fromStdString(str));
    }
    return result;
}

// Convierte std::unordered_set<std::string> a QSet<QString>
QSet<QString> SLRTutorWindow::stdUnorderedSetToQSet(
    const std::unordered_set<std::string>& uset) {
    QSet<QString> result;
    for (const auto& str : uset) {
        result.insert(QString::fromStdString(str));
    }
    return result;
}

// Convierte QSet<QString> a std::unordered_set<std::string>
std::unordered_set<std::string>
SLRTutorWindow::qsetToStdUnorderedSet(const QSet<QString>& qset) {
    std::unordered_set<std::string> result;
    for (const auto& qstr : qset) {
        result.insert(qstr.toStdString());
    }
    return result;
}

std::unordered_set<Lr0Item>
SLRTutorWindow::ingestUserItems(const QString& userResponse) {
    std::unordered_set<Lr0Item> items;
    QStringList lines = userResponse.split('\n', Qt::SkipEmptyParts);

    for (const QString& line : std::as_const(lines)) {
        std::string token    = line.trimmed().toStdString();
        size_t      arrowpos = token.find("->");
        if (arrowpos == std::string::npos) {
            return {};
        }
        std::string antecedent = token.substr(0, arrowpos);
        std::string consequent = token.substr(arrowpos + 2);

        auto trim = [](std::string& s) {
            size_t start = s.find_first_not_of(" \t");
            size_t end   = s.find_last_not_of(" \t");
            if (start == std::string::npos) {
                s.clear();
            } else {
                s = s.substr(start, end - start + 1);
            }
        };

        trim(antecedent);
        trim(consequent);

        consequent.erase(
            std::remove_if(consequent.begin(), consequent.end(),
                           [](char c) { return c == ' ' || c == '\t'; }),
            consequent.end());

        size_t dotpos = consequent.find('.');
        if (dotpos == std::string::npos) {
            return {};
        }
        std::string before_dot = consequent.substr(0, dotpos);
        std::string after_dot  = consequent.substr(dotpos + 1);

        std::vector<std::string> splitted_before_dot{grammar.Split(before_dot)};
        std::vector<std::string> splitted_after_dot{grammar.Split(after_dot)};

        std::vector<std::string> splitted{splitted_before_dot.begin(),
                                          splitted_before_dot.end()};
        splitted.insert(splitted.end(), splitted_after_dot.begin(),
                        splitted_after_dot.end());
        size_t dot_idx = splitted_before_dot.size();
        items.emplace(antecedent, splitted, static_cast<unsigned int>(dot_idx),
                      grammar.st_.EPSILON_, grammar.st_.EOL_);
    }
    return items;
}

std::vector<std::pair<std::string, std::vector<std::string>>>
SLRTutorWindow::ingestUserRules(const QString& userResponse) {
    std::stringstream ss(userResponse.toStdString());
    std::string       token;
    std::vector<std::pair<std::string, std::vector<std::string>>> rules;

    QStringList lines = userResponse.split('\n', Qt::SkipEmptyParts);
    for (const QString& line : std::as_const(lines)) {
        std::string token = line.trimmed().toStdString();

        size_t arrowpos = token.find("->");
        if (arrowpos == std::string::npos) {
            return {};
        }
        std::string antecedent = token.substr(0, arrowpos);
        std::string consequent = token.substr(arrowpos + 2);

        auto trim = [](std::string& s) {
            size_t start = s.find_first_not_of(" \t");
            size_t end   = s.find_last_not_of(" \t");
            if (start == std::string::npos) {
                s.clear();
            } else {
                s = s.substr(start, end - start + 1);
            }
        };

        trim(antecedent);
        trim(consequent);

        consequent.erase(
            std::remove_if(consequent.begin(), consequent.end(),
                           [](char c) { return c == ' ' || c == '\t'; }),
            consequent.end());

        std::vector<std::string> splitted{grammar.Split(consequent)};

        rules.emplace_back(antecedent, splitted);
    }
    return rules;
}

QString SLRTutorWindow::FormatGrammar(const Grammar& grammar) {
    QString result;
    int     ruleCount = 0;

    auto formatProductions =
        [&ruleCount](const QString& lhs, const std::vector<production>& prods) {
            QString out;
            QString headerText = lhs + " → ";
            QString indent(headerText.length(), ' ');

            for (size_t i = 0; i < prods.size(); ++i) {
                const auto& prod = prods[i];
                QString label    = "(" + QString::number(ruleCount++) + ")    ";

                if (i == 0) {
                    out += label + headerText;
                } else {
                    out += label + indent + "| ";
                }

                for (const auto& symbol : prod) {
                    out += QString::fromStdString(symbol) + " ";
                }
                out = out.trimmed();
                out += "\n";
            }
            return out;
        };

    for (const QString& lhs : std::as_const(sortedNonTerminals)) {
        const auto& prods = grammar.g_.at(lhs.toStdString());
        result += formatProductions(lhs, prods);
    }

    return result;
}

void SLRTutorWindow::fillSortedGrammar() {
    QVector<QPair<QString, QVector<QString>>> rules;
    QPair<QString, QVector<QString>>          rule;

    for (const QString& nt : std::as_const(sortedNonTerminals)) {
        const auto& prods = grammar.g_.at(nt.toStdString());
        for (const auto& prod : prods) {
            QPair<QString, QVector<QString>> rule{nt, {}};
            for (const std::string& symbol : prod) {
                rule.second.push_back(QString::fromStdString(symbol));
            }
            rules.push_back(rule);
        }
    }
    sortedGrammar = rules;
}

void SLRTutorWindow::on_userResponse_textChanged() {
    QTextDocument* doc = ui->userResponse->document();
    QFontMetrics   fm(ui->userResponse->font());

    const int lineHeight = fm.lineSpacing();
    const int maxLines   = 4;
    const int minLines   = 1;

    int lineCount = doc->blockCount();
    lineCount     = std::clamp(lineCount, minLines, maxLines);

    int padding       = 20;
    int desiredHeight = lineCount * lineHeight + padding;

    const int minHeight = 45;
    ui->userResponse->setMinimumHeight(minHeight);

    QPropertyAnimation* animation =
        new QPropertyAnimation(ui->userResponse, "minimumHeight");
    animation->setDuration(120);
    animation->setStartValue(ui->userResponse->height());
    animation->setEndValue(std::max(minHeight, desiredHeight));
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    ui->userResponse->setMaximumHeight(maxLines * lineHeight + padding);
}

QString
SLRTutorWindow::TeachClosure(const std::unordered_set<Lr0Item>& initialItems) {
    QString                     output;
    std::unordered_set<Lr0Item> items = initialItems;

    output += tr("Para el estado:\n");
    output += QString::fromStdString(slr1.PrintItems(items));

    std::unordered_set<std::string> visited;
    TeachClosureStep(items, items.size(), visited, 0, output);

    output += tr("Cierre:\n");
    for (const auto& item : items) {
        output += "  - " + QString::fromStdString(item.ToString()) + "\n";
    }

    return output;
}

void SLRTutorWindow::TeachClosureStep(std::unordered_set<Lr0Item>&     items,
                                      unsigned int                     size,
                                      std::unordered_set<std::string>& visited,
                                      int depth, QString& output) {
    QString indent(depth * 2, ' ');

    std::unordered_set<Lr0Item> newItems;

    output +=
        indent + tr("- Coge los ítems con un no terminal después del ·:\n");

    for (const auto& item : items) {
        std::string next = item.NextToDot();
        if (next == slr1.gr_.st_.EPSILON_ || slr1.gr_.st_.IsTerminal(next)) {
            continue;
        }

        output += indent + "  - " + tr("Ítem: ") +
                  QString::fromStdString(item.ToString()) + "\n";

        if (!slr1.gr_.st_.IsTerminal(next) && !visited.contains(next)) {
            output += indent + tr("    - Encontrado un no terminal: %1\n")
                                   .arg(QString::fromStdString(next));
            output += indent + tr("    - Añade todas las producciones de %1 "
                                  "con el · al inicio:\n")
                                   .arg(QString::fromStdString(next));

            const auto& rules = slr1.gr_.g_.at(next);
            for (const auto& rule : rules) {
                Lr0Item newItem(next, rule, 0, slr1.gr_.st_.EPSILON_,
                                slr1.gr_.st_.EOL_);
                newItems.insert(newItem);

                output += indent + "      - " + tr("Añadido: ") +
                          QString::fromStdString(newItem.ToString()) + "\n";
            }

            visited.insert(next);
        }
    }

    items.insert(newItems.begin(), newItems.end());

    if (size != items.size()) {
        output +=
            indent + tr("- Se han añadido nuevos ítems. Repite el proceso.\n");
        TeachClosureStep(items, items.size(), visited, depth + 1, output);
    } else {
        output +=
            indent +
            tr("- No se han añadido nuevos ítems. El cierre está completo.\n");
    }
}

QString
SLRTutorWindow::TeachDeltaFunction(const std::unordered_set<Lr0Item>& items,
                                   const QString&                     symbol) {
    if (symbol == QString::fromStdString(slr1.gr_.st_.EPSILON_)) {
        return tr("Sin importar el estado, δ(I, ε) = ∅.\n");
    }

    QString output;
    output += tr("Sea I:\n\n");
    output += QString::fromStdString(slr1.PrintItems(items));
    output += "\n";

    output += tr("Para encontrar δ(I, %1):\n").arg(symbol);
    output += tr("1. Busca los ítems con %1 después del ·. Es decir, ítems de "
                 "la forma α·%1β\n")
                  .arg(symbol);

    std::unordered_set<Lr0Item> filtered;
    for (const auto& item : items) {
        if (item.NextToDot() == symbol.toStdString()) {
            filtered.insert(item);
        }
    }

    if (filtered.empty()) {
        output += tr("2. No hay ítems. Por tanto δ(I, %1) = ∅\n").arg(symbol);
        return output;
    }

    output += tr("2. Sea J:\n\n");
    output += QString::fromStdString(slr1.PrintItems(filtered));
    output += "\n";

    output += tr("3. Avanza el · una posición:\n\n");
    std::unordered_set<Lr0Item> advanced;
    for (const auto& item : filtered) {
        Lr0Item moved = item;
        moved.AdvanceDot();
        advanced.insert(moved);
    }
    output += QString::fromStdString(slr1.PrintItems(advanced));
    output += "\n";

    output += tr("4. δ(I, %1) = CIERRE(J)\n").arg(symbol);
    output += tr("5. Cierre de J:\n\n");
    slr1.Closure(advanced);
    output += QString::fromStdString(slr1.PrintItems(advanced));

    return output;
}

void SLRTutorWindow::setupTutorial() {
    userMadeStates.insert(*std::ranges::find_if(
        slr1.states_, [](const state& st) { return st.id_ == 0; }));
    userMadeTransitions[0]["A"] = slr1.transitions_.at(0).at("A");
    updateProgressPanel();
    ui->userResponse->setDisabled(true);
    ui->confirmButton->setDisabled(true);
    tm->addStep(this->window(), tr("<h3>Tutor SLR(1)</h3>"
                                   "<p>Esta es la ventana del tutor de "
                                   "analizadores sintácticos SLR(1).</p>"));

    tm->addStep(ui->gr, tr("<h3>Gramática</h3>"
                           "<p>Como se puede ver, la gramática ahora es más "
                           "compleja. Se genera "
                           "aleatoriamente.</p>"));

    tm->addStep(ui->gr, tr("<h3>Gramática</h3>"
                           "<p>Las reglas están numeradas en el tutor SLR(1). "
                           "Te será útil para indicar las "
                           "acciones reduce en la tabla, pues deberás escribir "
                           "el número de la regla "
                           "correspondiente.</p>"));

    tm->addStep(ui->textEdit,
                tr("<h3>Progreso</h3>"
                   "<p>Aquí se registran los pasos, en el analizador SLR(1) "
                   "son distintos: "
                   "estados de la colección LR(0) y transiciones con la "
                   "función delta. Te será útil "
                   "para cuando tengas que rellenar la tabla SLR(1).</p>"));

    tm->addStep(ui->listWidget,
                tr("<h3>Formato de respuesta</h3>"
                   "<p>Observa como el tutor ahora te pide otro formato de "
                   "respuesta. Una regla gramatical "
                   "o "
                   "ítem LR (una regla gramatical con el (.) por línea. "
                   "Recuerda que con Ctrl+Enter puedes "
                   "insertar una nueva línea. Veamos unos ejemplos.</p>"));

    tm->addStep(
        ui->listWidget,
        tr("<h3>Ejemplo: regla gramatical</h3>"
           "<p>Supón que tienes esta producción en la gramática: X -> a b | "
           "c</p>"
           "<p>La respuesta correcta para esa pregunta sería exactamente:</p>"
           "<pre>X -> a b</pre>"
           "<pre>X -> c</pre>"
           "<p>Una sola regla por línea, tal cual aparece arriba.</p>"));

    tm->addStep(ui->listWidget,
                tr("<h3>Ejemplo: ítem LR(0)</h3>"
                   "<p>Un ítem LR añade un punto “.” para indicar la posición "
                   "en la regla.</p>"
                   "<p>Por ejemplo, un ítem correspondiente a la misma "
                   "producción sería:</p>"
                   "<pre>X -> a . b</pre>"
                   "<p>Observa el punto justo antes de “b”—ese es el formato "
                   "exigido. En caso de "
                   "varios ítems, simplemente coloca uno por línea.</p>"));

    tm->addStep(ui->listWidget, tr("<h3>Ejemplo: lista de símbolos</h3>"
                                   "<p>Al igual que el LL(1), se te puede "
                                   "pedir una lista de símbolos separados por "
                                   "coma.</p>"));

    tm->addStep(this->window(), tr("<h3>Finalización</h3>"
                                   "<p>Al igual que el tutor LL(1), podrás "
                                   "exportar toda la conversación y las tablas "
                                   "de análisis en formato PDF.</p>"));

    tm->addStep(nullptr, "");

    connect(tm, &TutorialManager::stepStarted, this, [this](int idx) {
        if (idx == 13) {
            tm->finishSLR1();
            this->close();
        }
    });
}
