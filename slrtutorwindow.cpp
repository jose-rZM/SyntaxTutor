#include "slrtutorwindow.h"
#include <QEasingCurve>
#include "ui_slrtutorwindow.h"

SLRTutorWindow::SLRTutorWindow(const Grammar& grammar, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SLRTutorWindow)
    , grammar(grammar)
    , slr1(grammar)
{
    slr1.MakeParser();
#ifdef QT_DEBUG
    slr1.DebugStates();
    slr1.DebugActions();
#endif
    std::ranges::for_each(slr1.states_, [this](const state &st) {
        bool hasComplete = false;
        bool hasIncomplete = false;

        for (const Lr0Item &it : st.items_) {
            if (it.IsComplete()) {
                hasComplete = true;
            } else {
                hasIncomplete = true;
            }

            if (hasComplete && hasIncomplete) {
                statesWithLr0Conflict.append(&st);
                conflictStatesIdQueue.push(st.id_);
                break;
            }
        }
    });

    std::ranges::for_each(slr1.states_, [this](const state &st) {
        if (statesWithLr0Conflict.contains(&st)) {
            return;
        }
        for (const Lr0Item &it : st.items_) {
            if (it.IsComplete()) {
                reduceStatesIdQueue.push(st.id_);
                break;
            }
        }
    });

    fillSortedGrammar();
    ui->setupUi(this);
    ui->confirmButton->setIcon(QIcon(":/resources/send.svg"));
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(10);
    shadow->setOffset(0);
    shadow->setColor(QColor::fromRgb(0, 200, 214));
    ui->confirmButton->setGraphicsEffect(shadow);

    ui->cntRight->setText(QString::number(cntRightAnswers));
    ui->cntWrong->setText(QString::number(cntWrongAnswers));

    ui->userResponse->setFont(QFont("Noto Sans", 15));
    ui->userResponse->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->userResponse->setPlaceholderText("Introduce aquí tu respuesta.");

    formattedGrammar = FormatGrammar(grammar);

    sortedNonTerminals = stdUnorderedSetToQSet(slr1.gr_.st_.non_terminals_).values();
    std::sort(sortedNonTerminals.begin(),
              sortedNonTerminals.end(),
              [&grammar](const QString &a, const QString &b) {
                  if (a == grammar.axiom_)
                      return true;
                  if (b == grammar.axiom_)
                      return false;
                  return a < b;
              });

    ui->gr->setFont(QFont("Noto Sans", 14));
    ui->gr->setText(formattedGrammar);

    ui->listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->listWidget->verticalScrollBar()->setSingleStep(10);

    updateProgressPanel();
    addMessage(QString("La gramática es:\n" + formattedGrammar), false);

    currentState = StateSlr::E;
    addDivisorLine("Estado inicial");
    addMessage(generateQuestion(), false);

    QFont chatFont("Noto Sans", 12);
    ui->listWidget->setFont(chatFont);

    connect(ui->userResponse,
            &CustomTextEdit::sendRequested,
            this,
            &SLRTutorWindow::on_confirmButton_clicked);
}

SLRTutorWindow::~SLRTutorWindow()
{
    delete ui;
}

void SLRTutorWindow::exportConversationToPdf(const QString& filePath) {
    QTextDocument doc;
    QString html;

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
    html += R"(
    <div style="text-align: center; font-size: 8pt; color: #888; margin-top: 60px;">
        Generado automáticamente por SyntaxTutor el )" + QDate::currentDate().toString("dd/MM/yyyy") + R"(</div>
    )";

    html += "<h2>Conversación</h2>";

    for (auto it = conversationLog.constBegin(); it != conversationLog.constEnd(); ++it) {
        const MessageLog& message = *it;
        QString safeText = message.message.toHtmlEscaped().replace("\n", "<br>");
        html += "<div class='entry'>";
        html += "<div class='role'>";
        html += (message.isUser ? "Usuario: " : "Tutor: ");
        html += "</div>";
        html += safeText;
        html += "</div>";
    }

    html += "</body></html>";
    html += R"(<div class='page-break'></div>)";


    html += "<h2>Estados del Autómata</h2>";
    for (size_t i = 0; i < userMadeStates.size(); ++i) {
        const state& st = *std::ranges::find_if(userMadeStates, [i](const state& st) {
            return st.id_ == i;
        });
        html += QString("<h3>Estado %1</h3><ul>").arg(st.id_);
        for (const Lr0Item& item : st.items_) {
            html += "<li>" + QString::fromStdString(item.ToString()).toHtmlEscaped() + "</li>";
        }
        html += "</ul><br>";
    }
    html += R"(<div class='page-break'></div>)";

    html += R"(<h2>Tabla de análisis SLR</h2><br>)";
    html += R"(<div class="container"><table border='1' cellspacing='0' cellpadding='5'>)";
    html += "<tr><th>Estado</th>";
    std::vector<std::string> columns;
    columns.reserve(slr1.gr_.st_.terminals_.size() + slr1.gr_.st_.non_terminals_.size());
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
            QString cell = "-";
            const bool isTerminal = slr1.gr_.st_.IsTerminal(symbol);
            if (!isTerminal) {
                if (trans_entry != slr1.transitions_.end()) {
                    const auto it = transitions.find(symbol);
                    if (it != transitions.end()) {
                        cell = QString::fromStdString(std::to_string(it->second));
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
                                    cell =
                                        QString::fromStdString("S" + std::to_string(shift_it->second));
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
    html += "<h2>Acciones Reduce</h2><br>";
    html += R"(<div class="container">)";
    html += "<table border='1' cellspacing='0' cellpadding='5'>";
    html += "<tr><th>Estado</th><th>Símbolo</th><th>Regla</th>";
    for (const auto& [state, actions] : slr1.actions_) {
        for (const auto& [symbol, action] : actions) {
            if (action.action == SLR1Parser::Action::Reduce) {
                std::string rule = action.item->antecedent_ + " → ";
                for (const auto& sym : action.item->consequent_) {
                    rule += sym + " ";
                }

                html += "<tr>";
                html += "<td align='center'>" + QString::number(state) + "</td>";
                html += "<td align='center'>" + QString::fromStdString(symbol) + "</td>";
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

void SLRTutorWindow::showTable()
{
    QStringList headers;
    for (const auto &symbol : slr1.gr_.st_.terminals_) {
        if (symbol == slr1.gr_.st_.EPSILON_) {
            continue;
        }
        headers << QString::fromStdString(symbol);
    }

    for (const auto &symbol : slr1.gr_.st_.non_terminals_) {
        headers << QString::fromStdString(symbol);
    }

    SLRTableDialog dialog(slr1.states_.size(), headers.size(), headers, this);
    if (dialog.exec() == QDialog::Accepted) {
        auto tabla = dialog.getTableData();

        // Acá podés usar `tabla` como quieras
        for (int i = 0; i < tabla.size(); ++i) {
            qDebug() << "Fila" << i << ":" << tabla[i];
        }

        addMessage("¡Tabla enviada correctamente!", false);
    }
}

void SLRTutorWindow::updateProgressPanel()
{
    QString text;
    if (userMadeStates.empty()) {
        text = "No se han construido estados aún.";
    } else {
        for (size_t i = 0; i < slr1.states_.size(); ++i) {
            auto st = std::ranges::find_if(userMadeStates,
                                           [i](const state &st) { return st.id_ == i; });
            if (st != userMadeStates.end()) {
                text += QString("Estado I%1\n").arg((*st).id_);
                for (const Lr0Item &item : (*st).items_) {
                    text += QString::fromStdString(item.ToString()) + "\n";
                }
            }
        }
    }

    ui->textEdit->setText(text);
}

void SLRTutorWindow::addUserState(unsigned id)
{
    auto st = std::ranges::find_if(slr1.states_, [id](const state &st) { return st.id_ == id; });
    if (st != slr1.states_.end()) {
        userMadeStates.insert(*st);
        updateProgressPanel();
    }
}

void SLRTutorWindow::addMessage(const QString &text, bool isUser)
{
    QString messageText = text;
    // LOG
    if (messageText.isEmpty()) {
        messageText = QString("No se proporcionó respuesta.");
    }
    conversationLog.emplaceBack(messageText, isUser);

    QWidget *messageWidget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(2);                    // Espaciado compacto
    mainLayout->setContentsMargins(10, 5, 10, 5); // Márgenes exteriores reducidos

    QLabel *header = new QLabel(isUser ? "Usuario" : "Tutor");
    header->setAlignment(isUser ? Qt::AlignRight : Qt::AlignLeft);
    header->setStyleSheet(
        isUser ? "font-weight: bold; color: #00ADB5; font-size: 12px; font-family: 'Noto Sans';"
               : "font-weight: bold; color: #BBBBBB; font-size: 12px; font-family: 'Noto Sans';");

    QHBoxLayout *messageLayout = new QHBoxLayout;
    messageLayout->setSpacing(0); // Sin espacio lateral adicional

    QVBoxLayout *innerLayout = new QVBoxLayout;
    innerLayout->setSpacing(0); // Compacta label y timestamp

    QLabel *label = new QLabel(messageText);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    QFontMetrics fm(label->font());
    int textWidth = fm.boundingRect(0, 0, ui->listWidget->width(), 0, Qt::TextWordWrap, text).width();

    int maxWidth = ui->listWidget->width() * 0.8;
    int adjustedWidth = qMin(textWidth + 32, maxWidth);
    label->setMaximumWidth(adjustedWidth);
    label->setMinimumWidth(300);

    if (isUser) {
        if (text.isEmpty()) {
            label->setStyleSheet(R"(
            background-color: #00ADB5;
            color: white;
            padding: 12px 16px;
            border-top-left-radius: 18px;
            border-top-right-radius: 0px;
            border-bottom-left-radius: 18px;
            border-bottom-right-radius: 18px;
            border: 1px solid rgba(0, 0, 0, 0.15);
            font-size: 14px;
            font-family: 'Noto Sans';
            font-style: italic;
        )");
        } else {
            label->setStyleSheet(R"(
            background-color: #00ADB5;
            color: white;
            padding: 12px 16px;
            border-top-left-radius: 18px;
            border-top-right-radius: 0px;
            border-bottom-left-radius: 18px;
            border-bottom-right-radius: 18px;
            border: 1px solid rgba(0, 0, 0, 0.15);
            font-size: 14px;
            font-family: 'Noto Sans';
        )");
        }
    } else {
        label->setStyleSheet(R"(
            background-color: #2F3542;
            color: #F1F1F1;
            padding: 12px 16px;
            border-top-left-radius: 0px;
            border-top-right-radius: 18px;
            border-bottom-left-radius: 18px;
            border-bottom-right-radius: 18px;
            border: 1px solid rgba(255, 255, 255, 0.05);
            font-size: 14px;
            font-family: 'Noto Sans';
        )");
    }

    label->adjustSize(); // Asegura que el QLabel se expanda verticalmente

    QLabel *timestamp = new QLabel(QTime::currentTime().toString("HH:mm"));
    timestamp->setStyleSheet(
        "font-size: 10px; color: gray; margin-left: 5px; font-family: 'Noto Sans';");
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
    messageWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    messageWidget->adjustSize();
    messageWidget->updateGeometry();

    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(messageWidget->sizeHint());

    if (isUser) {
        lastUserMessage = messageWidget;
    }

    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, messageWidget);
    ui->listWidget->update();
    ui->listWidget->scrollToBottom();
}

void SLRTutorWindow::addDivisorLine(const QString &stateName)
{
    QWidget *dividerWidget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(dividerWidget);
    layout->setContentsMargins(10, 5, 10, 5);
    layout->setSpacing(10); // espacio entre líneas y texto

    QFrame *lineLeft = new QFrame;
    lineLeft->setFrameShape(QFrame::HLine);
    lineLeft->setStyleSheet("color: #CCCCCC;");
    lineLeft->setMinimumWidth(20);
    lineLeft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QLabel *label = new QLabel(stateName);
    label->setStyleSheet(R"(
        color: #888888;
        font-size: 11px;
        font-family: 'Noto Sans';
        background: transparent;
        font-style: italic;
    )");

    QFrame *lineRight = new QFrame;
    lineRight->setFrameShape(QFrame::HLine);
    lineRight->setStyleSheet("color: #CCCCCC;");
    lineRight->setMinimumWidth(20);
    lineRight->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    layout->addWidget(lineLeft);
    layout->addWidget(label);
    layout->addWidget(lineRight);

    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    dividerWidget->setLayout(layout);
    item->setSizeHint(dividerWidget->sizeHint());

    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, dividerWidget);
    ui->listWidget->scrollToBottom();
}

void SLRTutorWindow::wrongAnimation()
{
    if (lastUserMessage == nullptr) {
        return;
    }
    QList<QLabel *> labels = lastUserMessage->findChildren<QLabel *>();
    if (labels.size() > 1) {
        QLabel *label = labels[1];

        auto *effect = new QGraphicsColorizeEffect(label);
        effect->setColor(Qt::red);
        label->setGraphicsEffect(effect);

        auto *animation = new QPropertyAnimation(effect, "strength");
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

void SLRTutorWindow::wrongUserResponseAnimation()
{
    QPoint originalPos = ui->userResponse->pos();

    QPropertyAnimation *animation = new QPropertyAnimation(ui->userResponse, "pos");
    animation->setDuration(200);
    animation->setLoopCount(1);

    animation->setKeyValueAt(0, originalPos);
    animation->setKeyValueAt(0.2, originalPos + QPoint(4, 0));
    animation->setKeyValueAt(0.4, originalPos - QPoint(4, 0));
    animation->setKeyValueAt(0.6, originalPos + QPoint(3, 0));
    animation->setKeyValueAt(0.8, originalPos - QPoint(3, 0));
    animation->setKeyValueAt(1, originalPos);

    animation->setEasingCurve(QEasingCurve::OutBounce);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void SLRTutorWindow::animateLabelPop(QLabel *label)
{
    QPropertyAnimation *animation = new QPropertyAnimation(label, "geometry");
    QRect startRect = label->geometry();
    QRect expandedRect = QRect(startRect.x() - 3,
                               startRect.y() - 2,
                               startRect.width() + 5,
                               startRect.height() + 4);

    animation->setDuration(200);
    animation->setKeyValueAt(0, startRect);
    animation->setKeyValueAt(0.5, expandedRect);
    animation->setKeyValueAt(1, startRect);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void SLRTutorWindow::animateLabelColor(QLabel *label, const QColor &flashColor)
{
    int durationMs = 400;
    QString originalStyle = label->styleSheet();
    label->setStyleSheet(QString("color: %1;").arg(flashColor.name()));

    QTimer::singleShot(durationMs, label, [label, originalStyle]() {
        label->setStyleSheet(originalStyle);
    });
}

void SLRTutorWindow::on_confirmButton_clicked()
{
    QString userResponse;
    bool isCorrect;
    userResponse = ui->userResponse->toPlainText().trimmed();
    addMessage(userResponse, true);

    isCorrect = verifyResponse(userResponse);
    if (!isCorrect) {
        ui->cntWrong->setText(QString::number(++cntWrongAnswers));
        animateLabelPop(ui->cross);
        animateLabelColor(ui->cross, QColor::fromRgb(204, 51, 51));
        addMessage(feedback(), false);
        wrongAnimation();
        wrongUserResponseAnimation();

    } else {
        ui->cntRight->setText(QString::number(++cntRightAnswers));
        animateLabelPop(ui->tick);
        animateLabelColor(ui->tick, QColor::fromRgb(0, 204, 102));
    }
    updateState(isCorrect);
    if (currentState == StateSlr::fin) {
        auto reply = QMessageBox::question(this,
                                           "Fin del análisis",
                                           "¿Deseas exportar la conversación?",
                                           QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            QString filePath = QFileDialog::getSaveFileName(this,
                                                            "Guardar conversación",
                                                            "conversacion.pdf",
                                                            "Archivo PDF (*.pdf)");

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
 *                      QUESTIONS                           *
 ************************************************************/

QString SLRTutorWindow::generateQuestion()
{
    switch (currentState) {
    case StateSlr::A:
        return "¿Cuál es el estado inicial del analizador?\nFormato:\nX -> a.b\nX -> .b\nX -> "
               "EPSILON.";
    case StateSlr::A1:
        return "¿Cuál es el axioma de la gramática?";
    case StateSlr::A2:
        return "Dado el ítem: S → · A$\n"
               "¿Qué símbolo aparece justo después del punto (·)?";
    case StateSlr::A3:
        return "Si el símbolo tras el punto (·) es un no terminal,\n"
               "¿cuáles son las reglas cuyo antecedente es ese símbolo?";
    case StateSlr::A4:
        return "¿Cuál es el cierre del ítem inicial?";
    case StateSlr::A_prime:
        return "Entonces, ¿cuál es el estado inicial?";
    case StateSlr::B:
        return "¿Cuántos estados se han generado en la colección LR(0) hasta ahora?";
    case StateSlr::C: {
        currentStateId = statesIdQueue.front();
        statesIdQueue.pop();
        auto currState = std::ranges::find_if(slr1.states_, [&](const state &st) {
            return st.id_ == currentStateId;
        });
        currentSlrState = *currState;
        return QString("Estado I%1:\n%2\n"
                       "¿Cuántos ítems contiene este estado?")
            .arg(currentStateId)
            .arg(QString::fromStdString(slr1.PrintItems(currentSlrState.items_)));
    }
    case StateSlr::CA:
        return "¿Qué símbolos aparecen después del punto (·) en los ítems de este estado?\n"
               "Formato: a,b,c";
    case StateSlr::CB: {
        QString currentSymbol = followSymbols.at(currentFollowSymbolsIdx);
        if (currentSymbol.toStdString() == slr1.gr_.st_.EPSILON_) {
            return QString("Calcula δ(I%1, %2):\n"
                           "Deja la entrada vacía si el resultado es vacío.")
                .arg(currentStateId)
                .arg(currentSymbol);
        } else {
            return QString("Calcula δ(I%1, %2):\n"
                           "¿Qué estado se genera al hacer transición con %2?\n"
                           "Este será el estado número %3")
                .arg(currentStateId)
                .arg(currentSymbol)
                .arg(nextStateId);
        }
    }
    case StateSlr::D:
        return "¿Cuántas filas y columnas tiene la tabla SLR(1)?\n"
               "Formato: filas,columnas";
    case StateSlr::D1:
        return "¿Cuántos estados contiene la colección LR(0)?";
    case StateSlr::D2:
        return "¿Cuántos símbolos terminales y no terminales hay en la gramática?\n"
               "(Excluye ε. Incluye $)";
    case StateSlr::D_prime:
        return "Con los datos anteriores,\n"
               "¿cuál es el tamaño total (filas,columnas) de la tabla SLR(1)?";
    case StateSlr::E:
        return "¿Cuántos estados contienen al menos un ítem completo?";
    case StateSlr::E1:
        return "Indica los ID de los estados con ítems completos, separados por comas.\n"
               "Ejemplo: 2,5,7";
    case StateSlr::E2:
        return "Indica cuántos ítems completos tiene cada estado.\n"
               "Formato: id1:n1, id2:n2, ...";
    case StateSlr::F:
        return "¿Qué estados presentan un CONFLICTO LR(0)?\n"
               "Deja la respuesta vacía si no hay conflictos.\n"
               "Formato: 1,3,7";
    case StateSlr::FA: {
        // cola de estados con conflicto: conflictStatesIdQueue
        currentConflictStateId = conflictStatesIdQueue.front();

        auto it = std::ranges::find_if(slr1.states_, [&](const state &st) {
            return st.id_ == currentConflictStateId;
        });
        currentConflictState = *it;

        // mostramos los ítems para que el alumno se oriente
        return QString("Estado I%1 con conflicto LR(0):\n%2\n"
                       "Indica los símbolos terminales sobre los que debe aplicarse reducción.\n"
                       "Formato: a,b,c (vacío si ninguno).")
            .arg(currentConflictStateId)
            .arg(QString::fromStdString(slr1.PrintItems(currentConflictState.items_)));
    }
    case StateSlr::G: {
        currentReduceStateId = reduceStatesIdQueue.front();

        auto it = std::ranges::find_if(slr1.states_, [&](const state &st) {
            return st.id_ == currentReduceStateId;
        });
        currentReduceState = *it;

        return QString("Estado I%1:\n%2\n"
                       "Indica los TERMINALES sobre los que se aplicará REDUCCIÓN.\n"
                       "Formato: a,b,c — vacío si no se aplica en ninguno.")
            .arg(currentReduceStateId)
            .arg(QString::fromStdString(slr1.PrintItems(currentReduceState.items_)));
    }

    default:
        return "";
    }
}

/************************************************************
 *                      UPDATE STATE                        *
 ************************************************************/

void SLRTutorWindow::updateState(bool isCorrect)
{
    switch (currentState) {
    case StateSlr::A: {
        currentState = isCorrect ? StateSlr::B : StateSlr::A1;
        if (isCorrect) {
            addUserState(0);
            statesIdQueue.push(0);
            addDivisorLine("Construcción de la colección LR(0)");
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
        currentState = isCorrect ? StateSlr::B : StateSlr::B;
        addUserState(0);
        statesIdQueue.push(0);
        addDivisorLine("Construcción de la colección LR(0)");
        break;
    }
    case StateSlr::B:
        if (statesIdQueue.empty()) {
            currentState = StateSlr::D;
            addDivisorLine("Tabla SLR(1)");
        } else {
            currentState = isCorrect ? StateSlr::C : StateSlr::C;
        }
        break;
    case StateSlr::C:
        currentState = isCorrect ? StateSlr::CA : StateSlr::CA;
        break;
    case StateSlr::CA: {
        if (!followSymbols.empty() && currentFollowSymbolsIdx < followSymbols.size()) {
            currentState = isCorrect ? StateSlr::CB : StateSlr::CB;
        } else {
            currentState = isCorrect ? StateSlr::CA : StateSlr::CA;
        }
        break;
    }
    case StateSlr::CB: {
        if (++currentFollowSymbolsIdx < followSymbols.size()) {
            currentState = isCorrect ? StateSlr::CB : StateSlr::CB;
        } else {
            followSymbols.clear();
            currentFollowSymbolsIdx = 0;
            currentState = isCorrect ? StateSlr::B : StateSlr::B;
        }
        addUserState(nextStateId);
        break;
    }
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
        currentState = isCorrect ? StateSlr::E : StateSlr::E;
        break;
    case StateSlr::E:
        currentState = isCorrect ? StateSlr::F : StateSlr::E1;
        break;

    case StateSlr::E1:
        currentState = isCorrect ? StateSlr::E2 : StateSlr::E1;
        break;

    case StateSlr::E2:
        currentState = isCorrect ? StateSlr::F : StateSlr::F;
        break;
    case StateSlr::F: {
        if (!isCorrect) { // vuelve a preguntar
            currentState = StateSlr::F;
            break;
        }
        // cola de conflictos para GA
        conflictStatesIdQueue = {};
        for (const state *st : std::as_const(statesWithLr0Conflict))
            conflictStatesIdQueue.push(st->id_);

        currentState = conflictStatesIdQueue.empty() ? StateSlr::G   // no hay conflictos
                                                     : StateSlr::FA; // hay que resolverlos
        break;
    }

    /* ---------- FA – resolver cada conflicto con FOLLOW -------------- */
    case StateSlr::FA:
        if (!isCorrect) { // repetir misma pregunta
            currentState = StateSlr::FA;
        } else if (!conflictStatesIdQueue.empty()) {
            conflictStatesIdQueue.pop();
            currentState = conflictStatesIdQueue.empty()
                               ? StateSlr::G
                               : StateSlr::FA; // siguiente estado conflictivo
        } else {
            currentState = StateSlr::G; // hechos todos → fase de reducciones
        }
        break;
    /* ---------- G – reducciones normales ---------------------------- */
    case StateSlr::G:
        if (!isCorrect) {
            currentState = StateSlr::G; // repetir misma pregunta
        } else if (!reduceStatesIdQueue.empty()) {
            reduceStatesIdQueue.pop();
            currentState = reduceStatesIdQueue.empty() ? StateSlr::fin
                                                       : StateSlr::G; // siguiente estado
        } else {
            currentState = StateSlr::fin; // todo listo, pasa a tabla / fin
        }
        break;
    case StateSlr::fin:
        break;
    }
}

/************************************************************
 *                      VERIFY RESPONSES                    *
 ************************************************************/

bool SLRTutorWindow::verifyResponse(const QString &userResponse)
{
    switch (currentState) {
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
    case StateSlr::B:
        return verifyResponseForB(userResponse);
    case StateSlr::C:
        return verifyResponseForC(userResponse);
    case StateSlr::CA:
        return verifyResponseForCA(userResponse);
    case StateSlr::CB:
        return verifyResponseForCB(userResponse);
    case StateSlr::D:
        return verifyResponseForD(userResponse);
    case StateSlr::D1:
        return verifyResponseForD1(userResponse);
    case StateSlr::D2:
        return verifyResponseForD2(userResponse);
    case StateSlr::D_prime:
        return verifyResponseForD(userResponse);
    case StateSlr::E:
        return verifyResponseForE(userResponse);
    case StateSlr::E1:
        return verifyResponseForE1(userResponse);
    case StateSlr::E2:
        return verifyResponseForE2(userResponse);
    case StateSlr::F:
        return verifyResponseForF(userResponse);
    case StateSlr::FA:
        return verifyResponseForFA(userResponse);
    case StateSlr::G:
        return verifyResponseForG(userResponse);
    default:
        return "";
    }
}

bool SLRTutorWindow::verifyResponseForA(const QString &userResponse)
{
    std::unordered_set<Lr0Item> response = ingestUserItems(userResponse);
    return response == solutionForA();
}

bool SLRTutorWindow::verifyResponseForA1(const QString &userResponse)
{
    return userResponse == solutionForA1();
}

bool SLRTutorWindow::verifyResponseForA2(const QString &userResponse)
{
    return userResponse == solutionForA2();
}

bool SLRTutorWindow::verifyResponseForA3(const QString &userResponse)
{
    const auto response = ingestUserRules(userResponse);
    return response == solutionForA3();
}

bool SLRTutorWindow::verifyResponseForA4(const QString &userResponse)
{
    std::unordered_set<Lr0Item> response = ingestUserItems(userResponse);
    return response == solutionForA4();
}

bool SLRTutorWindow::verifyResponseForB(const QString &userResponse)
{
    unsigned response = userResponse.toUInt();
    return response == solutionForB();
}

bool SLRTutorWindow::verifyResponseForC(const QString &userResponse)
{
    unsigned response = userResponse.toUInt();
    return response == solutionForC();
}

bool SLRTutorWindow::verifyResponseForCA(const QString &userResponse)
{
    QStringList response = userResponse.split(",", Qt::SkipEmptyParts);
    QStringList expected = solutionForCA();
    QSet<QString> responseSet(response.begin(), response.end());
    QSet<QString> expectedSet(expected.begin(), expected.end());
    return responseSet == expectedSet;
}

bool SLRTutorWindow::verifyResponseForCB(const QString &userResponse)
{
    if (followSymbols.at(currentFollowSymbolsIdx).toStdString() == slr1.gr_.st_.EPSILON_) {
        return userResponse.isEmpty();
    } else {
        const auto response = ingestUserItems(userResponse);
        return response == solutionForCB();
    }
}

bool SLRTutorWindow::verifyResponseForD(const QString &userResponse)
{
    return userResponse == solutionForD();
}

bool SLRTutorWindow::verifyResponseForD1(const QString &userResponse)
{
    return userResponse == solutionForD1();
}

bool SLRTutorWindow::verifyResponseForD2(const QString &userResponse)
{
    return userResponse == solutionForD2();
}

bool SLRTutorWindow::verifyResponseForE(const QString &userResponse)
{
    bool ok = false;
    int value = userResponse.toInt(&ok);
    return ok && value == static_cast<int>(solutionForE());
}

bool SLRTutorWindow::verifyResponseForE1(const QString &userResponse)
{
    QSet<unsigned> expected = solutionForE1();
    QStringList parts = userResponse.split(',', Qt::SkipEmptyParts);
    QSet<unsigned> given;

    for (const QString &s : std::as_const(parts)) {
        bool ok;
        unsigned id = s.trimmed().toUInt(&ok);
        if (!ok)
            return false;
        given.insert(id);
    }
    return given == expected;
}

bool SLRTutorWindow::verifyResponseForE2(const QString &userResponse)
{
    QMap<unsigned, unsigned> expected = solutionForE2(); // id -> nº ítems completos
    QMap<unsigned, unsigned> given;

    for (QString pair : userResponse.split(',', Qt::SkipEmptyParts)) {
        auto kv = pair.split(':', Qt::SkipEmptyParts);
        if (kv.size() != 2)
            return false;

        bool ok1, ok2;
        unsigned id = kv[0].trimmed().toUInt(&ok1);
        unsigned num = kv[1].trimmed().toUInt(&ok2);
        if (!ok1 || !ok2)
            return false;

        given[id] = num;
    }
    return given == expected;
}

bool SLRTutorWindow::verifyResponseForF(const QString &userResponse)
{
    QSet<unsigned> expected = solutionForF(); // ids con conflicto
    QStringList parts = userResponse.split(',', Qt::SkipEmptyParts);
    QSet<unsigned> given;
    for (const QString &s : std::as_const(parts)) {
        bool ok;
        unsigned id = s.trimmed().toUInt(&ok);
        if (!ok)
            return false;
        given.insert(id);
    }
    return given == expected;
}

bool SLRTutorWindow::verifyResponseForFA(const QString &userResponse)
{
    QStringList resp = userResponse.split(',', Qt::SkipEmptyParts);
    QSet<QString> given(resp.begin(), resp.end());
    QSet<QString> expected = solutionForFA();
    return given == expected;
}

bool SLRTutorWindow::verifyResponseForG(const QString &userResponse)
{
    QSet<QString> expected = solutionForG();
    QStringList resp = userResponse.split(',', Qt::SkipEmptyParts);
    QSet<QString> given = QSet<QString>(resp.begin(), resp.end());
    return given == expected;
}

/************************************************************
 *                         SOLUTIONS                        *
 ************************************************************/

std::unordered_set<Lr0Item> SLRTutorWindow::solutionForA()
{
    const auto expected = std::ranges::find_if(slr1.states_,
                                               [](const state &st) { return st.id_ == 0; });
    return expected->items_;
}

QString SLRTutorWindow::solutionForA1()
{
    return QString::fromStdString(grammar.axiom_);
}

QString SLRTutorWindow::solutionForA2()
{
    return QString::fromStdString(grammar.g_.at(grammar.axiom_).at(0).at(0));
}

std::vector<std::pair<std::string, std::vector<std::string>>> SLRTutorWindow::solutionForA3()
{
    std::string next = grammar.g_.at(grammar.axiom_).at(0).at(0);
    std::vector<std::pair<std::string, std::vector<std::string>>> result;

    const auto &rules = grammar.g_.at(next);
    for (const auto &rhs : rules) {
        result.emplace_back(next, rhs);
    }

    return result;
}

std::unordered_set<Lr0Item> SLRTutorWindow::solutionForA4()
{
    std::unordered_set<Lr0Item> items;
    items.emplace(grammar.axiom_,
                  grammar.g_.at(grammar.axiom_).at(0),
                  grammar.st_.EPSILON_,
                  grammar.st_.EOL_);
    slr1.Closure(items);
    return items;
}

unsigned SLRTutorWindow::solutionForB()
{
    return userMadeStates.size();
}

unsigned SLRTutorWindow::solutionForC()
{
    return currentSlrState.items_.size();
}

QStringList SLRTutorWindow::solutionForCA()
{
    QSet<QString> following_symbols;
    std::ranges::for_each(currentSlrState.items_, [&following_symbols](const Lr0Item &item) {
        following_symbols.insert(QString::fromStdString(item.NextToDot()));
    });

    // FILL FOLLOW SYMBOLS FOR CB QUESTION
    followSymbols = following_symbols.values();
    currentFollowSymbolsIdx = 0;

    return followSymbols;
}

std::unordered_set<Lr0Item> SLRTutorWindow::solutionForCB()
{
    auto st = std::ranges::find_if(slr1.states_, [this](const state &st) {
                  return st.id_ == nextStateId;
              })->items_;
    return st;
}

QString SLRTutorWindow::solutionForD()
{
    return QString("%1,%2").arg(solutionForD1()).arg(solutionForD2());
}

QString SLRTutorWindow::solutionForD1()
{
    return QString::number(slr1.states_.size());
}

QString SLRTutorWindow::solutionForD2()
{
    unsigned terminals = slr1.gr_.st_.terminals_.contains(slr1.gr_.st_.EPSILON_)
                             ? slr1.gr_.st_.terminals_.size() - 1
                             : slr1.gr_.st_.terminals_.size();
    unsigned non_terminals = slr1.gr_.st_.non_terminals_.size();
    return QString::number(terminals + non_terminals);
}

std::ptrdiff_t SLRTutorWindow::solutionForE()
{
    return std::ranges::count_if(slr1.states_, [](const state &st) {
        return std::ranges::any_of(st.items_, [](const Lr0Item &item) { return item.IsComplete(); });
    });
}

QSet<unsigned> SLRTutorWindow::solutionForE1()
{
    QSet<unsigned> ids;
    for (const state &st : slr1.states_) {
        if (std::ranges::any_of(st.items_, [](const Lr0Item &it) { return it.IsComplete(); }))
            ids.insert(st.id_);
    }
    return ids;
}

QMap<unsigned, unsigned> SLRTutorWindow::solutionForE2()
{
    QMap<unsigned, unsigned> result;
    for (const state &st : slr1.states_) {
        unsigned count = 0;
        for (const Lr0Item &it : st.items_)
            if (it.IsComplete())
                ++count;
        if (count)
            result[st.id_] = count;
    }
    return result;
}

QSet<unsigned> SLRTutorWindow::solutionForF()
{
    QSet<unsigned> ids;
    for (const state *st : std::as_const(statesWithLr0Conflict))
        ids.insert(st->id_);
    return ids;
}

QSet<QString> SLRTutorWindow::solutionForFA()
{
    QSet<QString> symbols;

    for (const Lr0Item &it : currentConflictState.items_) {
        if (!it.IsComplete())
            continue;

        // FOLLOW del antecedente
        std::unordered_set<std::string> fol = slr1.Follow(it.antecedent_);

        for (const std::string &s : fol)
            symbols.insert(QString::fromStdString(s));
    }
    return symbols;
}

QSet<QString> SLRTutorWindow::solutionForG()
{
    QSet<QString> symbols;
    for (const Lr0Item &it : currentReduceState.items_) {
        if (!it.IsComplete())
            continue;
        std::unordered_set<std::string> fol = slr1.Follow(it.antecedent_);
        for (const std::string &s : fol)
            symbols.insert(QString::fromStdString(s));
    }
    symbols.remove(QString::fromStdString(slr1.gr_.st_.EPSILON_));
    return symbols;
}

/************************************************************
 *                         FEEDBACK                         *
 ************************************************************/

QString SLRTutorWindow::feedback()
{
    switch (currentState) {
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
    case StateSlr::B:
        return feedbackForB();
    case StateSlr::C:
        return feedbackForC();
    case StateSlr::CA:
        return feedbackForCA();
    case StateSlr::CB:
        return feedbackForCB();
    case StateSlr::D:
        return feedbackForD();
    case StateSlr::D1:
        return feedbackForD1();
    case StateSlr::D2:
        return feedbackForD2();
    case StateSlr::D_prime:
        return feedbackForDPrime();
    case StateSlr::E:
        return feedbackForE();
    case StateSlr::E1:
        return feedbackForE1();
    case StateSlr::E2:
        return feedbackForE2();
    case StateSlr::F:
        return feedbackForF();
    case StateSlr::FA:
        return feedbackForFA();
    case StateSlr::G:
        return feedbackForG();
    default:
        return "Error en feedback.";
    }
}

QString SLRTutorWindow::feedbackForA()
{
    return "El estado inicial se construye a partir del cierre del ítem asociado al axioma: S -> · "
           "A$. Esto representa que aún no se ha leído nada y se quiere derivar desde el símbolo "
           "inicial.";
}

QString SLRTutorWindow::feedbackForA1()
{
    return QString("El axioma es el símbolo desde el que comienza toda la derivación. En esta "
                   "gramática, el axioma es: %1.")
        .arg(QString::fromStdString(grammar.axiom_));
}

QString SLRTutorWindow::feedbackForA2()
{
    return QString("El símbolo que sigue al (·) indica cuál es el siguiente símbolo que debe ser "
                   "procesado. En este ítem, ese símbolo es: %1.")
        .arg(QString::fromStdString(grammar.g_.at(grammar.axiom_).at(0).at(0)));
}

QString SLRTutorWindow::feedbackForA3()
{
    QString antecedent = QString::fromStdString(grammar.g_.at(grammar.axiom_).at(0).at(0));
    QString result = QString("Como el símbolo tras el · es %1, se debe expandir sus producciones "
                             "en el cierre. Las reglas cuyo antecedente es %1 son:\n")
                         .arg(antecedent);

    for (auto it = sortedGrammar.constBegin(); it != sortedGrammar.constEnd(); ++it) {
        if (it->first == antecedent) {
            QStringList symbols = it->second.toList();
            QString ruleStr = QString("%1 -> %2").arg(it->first, symbols.join(" "));
            result += ruleStr + "\n";
        }
    }

    return result;
}

QString SLRTutorWindow::feedbackForA4()
{
    Lr0Item init{grammar.axiom_,
                 grammar.g_.at(grammar.axiom_).at(0),
                 grammar.st_.EPSILON_,
                 grammar.st_.EOL_};
    std::unordered_set<Lr0Item> item{init};
    return "El cierre incluye todas las producciones de los no terminales que aparecen tras el ·, "
           "añadidas recursivamente.\n"
           + QString::fromStdString(slr1.TeachClosure(item));
}

QString SLRTutorWindow::feedbackForAPrime()
{
    const auto &st = std::ranges::find_if(slr1.states_, [](const state &st) { return st.id_ == 0; });
    QString items = QString::fromStdString(slr1.PrintItems(st->items_));
    return QString("El estado inicial (I0) es el cierre del ítem con el axioma. Contiene todos los "
                   "ítems posibles a partir de ese punto.\n%1")
        .arg(items);
}

QString SLRTutorWindow::feedbackForB()
{
    return QString("Se ha(n) generado %1 estado(s) hasta ahora. Cada transición sobre un símbolo "
                   "genera un nuevo estado si lleva a un conjunto distinto de ítems.")
        .arg(userMadeStates.size());
}

QString SLRTutorWindow::feedbackForC()
{
    return QString("El estado I%2 contiene %1 ítem(s).")
        .arg(currentSlrState.items_.size())
        .arg(currentStateId);
}

QString SLRTutorWindow::feedbackForCA()
{
    QStringList following = solutionForCA();
    if (std::ranges::any_of(currentSlrState.items_,
                            [](const Lr0Item &item) { return item.IsComplete(); })) {
        return QString("Los símbolos son: %1. Cuando un ítem es de la forma X -> a ·, X -> a·$ o X "
                       "-> EPSILON · "
                       "(completo), el símbolo siguiente es siempre EPSILON. En estas condiciones, "
                       "se puede aplicar un reduce.")
            .arg(following.join(", "));
    } else {
        return QString("Los símbolos que aparecen tras el punto (·) en los ítems determinan "
                       "posibles transiciones. En este estado, esos símbolos son: %1.")
            .arg(following.join(", "));
    }
}

QString SLRTutorWindow::feedbackForCB()
{
    return QString::fromStdString(
        slr1.TeachDeltaFunction(currentSlrState.items_,
                                followSymbols[currentFollowSymbolsIdx].toStdString()));
}

QString SLRTutorWindow::feedbackForD()
{
    return QString("La tabla SLR(1) tiene una fila por cada estado y columnas por cada símbolo "
                   "terminal y no terminal (sin ε).");
}

QString SLRTutorWindow::feedbackForD1()
{
    return QString("Se han generado %1 estados").arg(slr1.states_.size());
}

QString SLRTutorWindow::feedbackForD2()
{
    return QString(
               "Hay un total de %1 de símbolos gramáticas, excluyendo la cadena vacía (EPSILON).")
        .arg(solutionForD2());
}

QString SLRTutorWindow::feedbackForDPrime()
{
    return QString("La tabla SLR(1) tiene tantas filas como estados haya, y tantas columnas como "
                   "símbolos gramáticas, excepto la cadena vacía. Es decir, tiene %1 filas y %2 "
                   "columnas.")
        .arg(solutionForD1())
        .arg(solutionForD2());
}

QString SLRTutorWindow::feedbackForE()
{
    return "Un estado es candidato para una acción REDUCE si contiene algún ítem de la forma X -> "
           "α ·, es decir, con el punto al final.";
}

QString SLRTutorWindow::feedbackForE1()
{
    QStringList ids;
    QSet<unsigned> sol = solutionForE1();
    for (unsigned id : sol)
        ids << QString::number(id);
    return "Los estados con ítems completos son: " + ids.join(", ")
           + ". Estos son los únicos estados donde puede haber acciones REDUCE en la tabla.";
}

QString SLRTutorWindow::feedbackForE2()
{
    QMap<unsigned int, unsigned int> sol = solutionForE2();
    QStringList pairs;
    for (auto it = sol.cbegin(); it != sol.cend(); ++it)
        pairs << QString("%1:%2").arg(it.key()).arg(it.value());
    return "Detalle de ítems completos por estado → " + pairs.join(", ");
}

QString SLRTutorWindow::feedbackForF()
{
    QString txt = "Un conflicto LR(0) ocurre cuando un mismo estado contiene tanto: ítems "
                  "completos (REDUCE) como ítems con algún símbolo tras el · (SHIFT). En estos "
                  "casos, la acción no es única, aparece un conflicto que debe resolverse.";
    if (statesWithLr0Conflict.isEmpty())
        return txt + " En esta colección no aparece ningún conflicto.";
    QStringList ids;
    QSet<unsigned> sol = solutionForF();
    for (unsigned id : std::as_const(sol))
        ids << QString::number(id);
    return txt + " Los estados conflictivos son: " + ids.join(", ");
}

QString SLRTutorWindow::feedbackForFA()
{
    QStringList list = solutionForFA().values();
    return QString("Para resolver el conflicto en I%1, se usan los símbolos SIG del antecedente. "
                   "Solo se aplica REDUCE en los terminales: %2.")
        .arg(currentConflictStateId)
        .arg(list.join(", "));
}

QString SLRTutorWindow::feedbackForG()
{
    QStringList list = solutionForG().values();
    return QString("En el estado %1, se aplica REDUCE en los terminales: %2.")
        .arg(currentReduceStateId)
        .arg(list.join(", "));
}

/************************************************************
 *                         HELPERS                          *
 ************************************************************/

// HELPER FUNCTIONS ----------------------------------------------------------------------
std::vector<std::string> SLRTutorWindow::qvectorToStdVector(const QVector<QString> &qvec)
{
    std::vector<std::string> result;
    result.reserve(qvec.size());
    for (const auto &qstr : qvec) {
        result.push_back(qstr.toStdString());
    }
    return result;
}

// Convierte std::vector<std::string> a QVector<QString>
QVector<QString> SLRTutorWindow::stdVectorToQVector(const std::vector<std::string> &vec)
{
    QVector<QString> result;
    result.reserve(vec.size());
    for (const auto &str : vec) {
        result.push_back(QString::fromStdString(str));
    }
    return result;
}

// Convierte std::unordered_set<std::string> a QSet<QString>
QSet<QString> SLRTutorWindow::stdUnorderedSetToQSet(const std::unordered_set<std::string> &uset)
{
    QSet<QString> result;
    for (const auto &str : uset) {
        result.insert(QString::fromStdString(str));
    }
    return result;
}

// Convierte QSet<QString> a std::unordered_set<std::string>
std::unordered_set<std::string> SLRTutorWindow::qsetToStdUnorderedSet(const QSet<QString> &qset)
{
    std::unordered_set<std::string> result;
    for (const auto &qstr : qset) {
        result.insert(qstr.toStdString());
    }
    return result;
}

std::unordered_set<Lr0Item> SLRTutorWindow::ingestUserItems(const QString &userResponse)
{
    std::unordered_set<Lr0Item> items;
    QStringList lines = userResponse.split('\n', Qt::SkipEmptyParts);

    for (const QString &line : std::as_const(lines)) {
        std::string token = line.trimmed().toStdString();
        size_t arrowpos = token.find("->");
        if (arrowpos == std::string::npos) {
            return {};
        }
        std::string antecedent = token.substr(0, arrowpos);
        std::string consequent = token.substr(arrowpos + 2);

        auto trim = [](std::string &s) {
            size_t start = s.find_first_not_of(" \t");
            size_t end = s.find_last_not_of(" \t");
            if (start == std::string::npos) {
                s.clear();
            } else {
                s = s.substr(start, end - start + 1);
            }
        };

        trim(antecedent);
        trim(consequent);

        consequent.erase(std::remove_if(consequent.begin(),
                                        consequent.end(),
                                        [](char c) { return c == ' ' || c == '\t'; }),
                         consequent.end());

        size_t dotpos = consequent.find('.');
        if (dotpos == std::string::npos) {
            return {};
        }
        std::string before_dot = consequent.substr(0, dotpos);
        std::string after_dot = consequent.substr(dotpos + 1);

        std::vector<std::string> splitted_before_dot{grammar.Split(before_dot)};
        std::vector<std::string> splitted_after_dot{grammar.Split(after_dot)};

        std::vector<std::string> splitted{splitted_before_dot.begin(), splitted_before_dot.end()};
        splitted.insert(splitted.end(), splitted_after_dot.begin(), splitted_after_dot.end());
        size_t dot_idx = splitted_before_dot.size();
        items.emplace(antecedent,
                      splitted,
                      static_cast<unsigned int>(dot_idx),
                      grammar.st_.EPSILON_,
                      grammar.st_.EOL_);
    }
    return items;
}

std::vector<std::pair<std::string, std::vector<std::string>>> SLRTutorWindow::ingestUserRules(
    const QString &userResponse)
{
    std::stringstream ss(userResponse.toStdString());
    std::string token;
    std::vector<std::pair<std::string, std::vector<std::string>>> rules;

    QStringList lines = userResponse.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : std::as_const(lines)) {
        std::string token = line.trimmed().toStdString();

        size_t arrowpos = token.find("->");
        if (arrowpos == std::string::npos) {
            return {};
        }
        std::string antecedent = token.substr(0, arrowpos);
        std::string consequent = token.substr(arrowpos + 2);

        auto trim = [](std::string &s) {
            size_t start = s.find_first_not_of(" \t");
            size_t end = s.find_last_not_of(" \t");
            if (start == std::string::npos) {
                s.clear();
            } else {
                s = s.substr(start, end - start + 1);
            }
        };

        trim(antecedent);
        trim(consequent);

        consequent.erase(std::remove_if(consequent.begin(),
                                        consequent.end(),
                                        [](char c) { return c == ' ' || c == '\t'; }),
                         consequent.end());

        std::vector<std::string> splitted{grammar.Split(consequent)};

        rules.emplace_back(antecedent, splitted);
    }
    return rules;
}

QString SLRTutorWindow::FormatGrammar(const Grammar &grammar)
{
    QString result;
    const std::string &axiom = grammar.axiom_;
    std::map<std::string, std::vector<production>> sortedRules(grammar.g_.begin(), grammar.g_.end());

    auto formatProductions = [](const QString &lhs, const std::vector<production> &prods) {
        QString out;
        QString header = lhs + " → ";
        int indentSize = header.length();
        QString indent(indentSize, ' ');

        bool first = true;
        for (const auto &prod : prods) {
            if (first) {
                out += header;
                first = false;
            } else {
                out += indent + "| ";
            }

            for (const auto &symbol : prod) {
                out += QString::fromStdString(symbol) + " ";
            }
            out = out.trimmed();
            out += "\n";
        }
        return out;
    };

    auto axIt = grammar.g_.find(axiom);
    if (axIt != grammar.g_.end()) {
        result += formatProductions(QString::fromStdString(axiom), axIt->second);
    }

    for (const auto &[lhs, productions] : sortedRules) {
        if (lhs == axiom)
            continue;
        result += formatProductions(QString::fromStdString(lhs), productions);
    }

    return result;
}

void SLRTutorWindow::fillSortedGrammar()
{
    auto it = grammar.g_.find(grammar.axiom_);
    QVector<QPair<QString, QVector<QString>>> rules;
    QPair<QString, QVector<QString>> rule({QString::fromStdString(grammar.axiom_), {}});

    if (it != grammar.g_.end()) {
        for (const auto &prod : it->second) {
            for (const auto &symbol : prod) {
                rule.second.push_back(QString::fromStdString(symbol));
            }
        }
    }
    rules.push_back(rule);
    std::map<std::string, std::vector<production>> sortedRules(grammar.g_.begin(), grammar.g_.end());
    for (const auto &[lhs, productions] : sortedRules) {
        if (lhs == grammar.axiom_)
            continue;
        rule = {QString::fromStdString(lhs), {}};
        for (const auto &prod : productions) {
            for (const auto &symbol : prod) {
                rule.second.push_back(QString::fromStdString(symbol));
            }
            rules.push_back(rule);
            rule = {QString::fromStdString(lhs), {}};
        }
    }
    sortedGrammar = rules;
}

void SLRTutorWindow::on_userResponse_textChanged()
{
    QTextDocument *doc = ui->userResponse->document();
    QFontMetrics fm(ui->userResponse->font());

    const int lineHeight = fm.lineSpacing();
    const int maxLines = 4;
    const int minLines = 1;

    int lineCount = doc->blockCount();
    lineCount = std::clamp(lineCount, minLines, maxLines);

    int padding = 20;
    int desiredHeight = lineCount * lineHeight + padding;

    // Establecer mínimo fijo (respetado por el layout)
    const int minHeight = 45;
    ui->userResponse->setMinimumHeight(minHeight);

    // Animar el cambio de altura real
    QPropertyAnimation *animation = new QPropertyAnimation(ui->userResponse, "minimumHeight");
    animation->setDuration(120);
    animation->setStartValue(ui->userResponse->height());
    animation->setEndValue(std::max(minHeight, desiredHeight)); // nunca menos de minHeight
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    // Establece también el máximo para limitar el crecimiento
    ui->userResponse->setMaximumHeight(maxLines * lineHeight + padding);
}
