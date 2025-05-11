#include "lltutorwindow.h"
#include "ui_lltutorwindow.h"

LLTutorWindow::LLTutorWindow(const Grammar& grammar, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LLTutorWindow)
    , grammar(grammar)
    , ll1(grammar)
{
    ll1.CreateLL1Table();
    ll1.PrintTable();
    fillSortedGrammar();

    ui->setupUi(this);
    ui->confirmButton->setIcon(QIcon(":/resources/send.svg"));
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(10);
    shadow->setOffset(0);
    shadow->setColor(QColor("#00C8D6"));
    ui->confirmButton->setGraphicsEffect(shadow);

    ui->cntRight->setText(QString::number(cntRightAnswers));
    ui->cntWrong->setText(QString::number(cntWrongAnswers));

    ui->userResponse->setFont(QFont("Noto Sans", 15));
    ui->userResponse->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->userResponse->setPlaceholderText("Introduce aquí tu respuesta.");

    formattedGrammar = FormatGrammar(grammar);

    ui->gr->setFont(QFont("Noto Sans", 14));
    ui->gr->setText(formattedGrammar);

    ui->listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->listWidget->verticalScrollBar()->setSingleStep(10);

    updateProgressPanel();
    addMessage(QString("La gramática es:\n" + formattedGrammar), false);

    currentState = State::A;
    addDivisorLine("Estado inicial");
    addMessage(generateQuestion(), false);
    ui->userResponse->clear();

    QFont chatFont("Noto Sans", 12);
    ui->listWidget->setFont(chatFont);
    connect(ui->userResponse,
            &CustomTextEdit::sendRequested,
            this,
            &LLTutorWindow::on_confirmButton_clicked);
}

LLTutorWindow::~LLTutorWindow()
{
    delete ui;
}

void LLTutorWindow::exportConversationToPdf(const QString &filePath)
{
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
        Generado automáticamente por SyntaxTutor el )"
            + QDate::currentDate().toString("dd/MM/yyyy") + R"(</div>
    )";

    html += "<h2>Conversación</h2>";

    for (auto it = conversationLog.constBegin(); it != conversationLog.constEnd(); ++it) {
        const MessageLog &message = *it;
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

    html += "<h2>Cabeceras</h2>";
    for (const auto &[nt, _] : sortedGrammar) {
        const auto &first = stdUnorderedSetToQSet(ll1.first_sets_[nt.toStdString()]).values();
        html += "CAB(" + nt + ") = {";
        html += first.join(",");
        html += "}<br>";
    }

    html += "<h2>Siguientes</h2>";
    for (const auto &[nt, _] : sortedGrammar) {
        const auto &follow = stdUnorderedSetToQSet(ll1.follow_sets_[nt.toStdString()]).values();
        html += "SIG(" + nt + ") = {" + follow.join(',') + "}<br>";
    }

    html += "<h2>Símbolos directores</h2>";
    for (const auto &[nt, production] : sortedGrammar) {
        const auto predSymbols = stdUnorderedSetToQSet(
                                     ll1.PredictionSymbols(nt.toStdString(),
                                                           qvectorToStdVector(production)))
                                     .values();
        html += "SD(" + nt + " → " + production.join(' ') + ") = {" + predSymbols.join(',')
                + "}<br>";
    }
    html += R"(<div class='page-break'></div>)";
    html += R"(<div class="container"><table border='1' cellspacing='0' cellpadding='5'>)";
    html += "<tr><th>No terminal / Símbolo</th>";
    for (const auto &s : ll1.gr_.st_.terminals_) {
        html += "<th>" + QString::fromStdString(s) + "</th>";
    }
    html += "</tr>";
    for (const auto &[nt, _] : sortedGrammar) {
        html += "<tr><td align='center'>" + nt + "</td>";
        for (const auto &s : ll1.gr_.st_.terminals_) {
            html += "<td align='center'>";
            if (ll1.ll1_t_[nt.toStdString()].contains(s)) {
                html += stdVectorToQVector(ll1.ll1_t_[nt.toStdString()][s][0]).join(' ');
            } else {
                html += "-";
            }
            html += "</td>";
        }
        html += "</tr>";
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

void LLTutorWindow::updateProgressPanel()
{
    QString text;
    text = "Nada que mostrar.";
    ui->textEdit->setText(text);
}

void LLTutorWindow::addMessage(const QString& text, bool isUser) {
    if (text.isEmpty() && !isUser) {
        return; // Because State C
    }
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

void LLTutorWindow::showTable()
{
    QStringList colHeaders;
    QStringList rowHeaders;

    for (const auto &symbol : ll1.gr_.st_.terminals_) {
        if (symbol == ll1.gr_.st_.EPSILON_) {
            continue;
        }
        colHeaders << QString::fromStdString(symbol);
    }

    for (const auto &symbol : ll1.gr_.st_.non_terminals_) {
        rowHeaders << QString::fromStdString(symbol);
    }

    LLTableDialog dialog(rowHeaders, colHeaders, this, &rawTable);
    if (dialog.exec() == QDialog::Accepted) {
        rawTable.clear();
        rawTable = dialog.getTableData();

        lltable.clear();

        for (int i = 0; i < rawTable.size(); ++i) {
            qDebug() << "Fila" << i << ":" << rawTable[i];

            const QString &rowHeader = rowHeaders[i];

            for (int j = 0; j < rawTable[i].size(); ++j) {
                const QString &colHeader = colHeaders[j];
                const QString &cellContent = rawTable[i][j];

                if (!cellContent.isEmpty()) {
                    QStringList production = stdVectorToQVector(
                        ll1.gr_.Split(cellContent.toStdString()));
                    lltable[rowHeader][colHeader] = production;
                }
            }
        }
    } else {
        rawTable.clear();
    }
    on_confirmButton_clicked();
}

void LLTutorWindow::addDivisorLine(const QString &stateName)
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

void LLTutorWindow::wrongAnimation()
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
                label->setStyleSheet(R"(
                    background-color: #00ADB5;
                    color: white;
                    padding: 12px 16px;
                    border-top-left-radius: 18px;
                    border-top-right-radius: 0px;
                    border-bottom-left-radius: 18px;
                    border-bottom-right-radius: 18px;
                    border-right: 1.5px solid red;
                    font-size: 14px;
                    font-family: 'Noto Sans';
                )");
            }
        });
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void LLTutorWindow::wrongUserResponseAnimation()
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

void LLTutorWindow::animateLabelPop(QLabel *label)
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

void LLTutorWindow::animateLabelColor(QLabel *label, const QColor &flashColor)
{
    int durationMs = 400;
    QString originalStyle = label->styleSheet();
    label->setStyleSheet(QString("color: %1;").arg(flashColor.name()));

    QTimer::singleShot(durationMs, label, [label, originalStyle]() {
        label->setStyleSheet(originalStyle);
    });
}

void LLTutorWindow::on_confirmButton_clicked()
{
    QString userResponse;
    bool isCorrect;
    if (currentState != State::C) {
        userResponse = ui->userResponse->toPlainText().trimmed();
        addMessage(userResponse, true);
        isCorrect = verifyResponse(userResponse);
    } else {
        isCorrect = verifyResponseForC();
        if (!isCorrect)
            ++lltries;
    }


    if (!isCorrect) {
        ui->cntWrong->setText(QString::number(++cntWrongAnswers));
        animateLabelPop(ui->cross);
        animateLabelColor(ui->cross, QColor("#cc3333"));
        addMessage(feedback(), false);
        wrongAnimation();
        wrongUserResponseAnimation();
    } else {
        ui->cntRight->setText(QString::number(++cntRightAnswers));
        animateLabelPop(ui->tick);
        animateLabelColor(ui->tick, QColor("#00cc66"));
    }
    updateState(isCorrect);

    if (currentState == State::fin) {
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

void LLTutorWindow::updateState(bool isCorrect) {
    switch (currentState) {
    case State::A:
        currentState = isCorrect ? State::B : State::A1;
        break;
    case State::A1:
        currentState = isCorrect ? State::A2 : State::A1;
        break;
    case State::A2:
        currentState = isCorrect ? State::A_prime : State::A2;
        break;
    case State::A_prime:
        currentState = isCorrect ? State::B : State::B;
        break;
    case State::B:
        if (isCorrect) {
            currentRule++;
            if (static_cast<qsizetype>(currentRule) >= sortedGrammar.size()) {
                currentState = State::C;
            } else {
                currentState = State::B;
            }
        } else {
            currentState = State::B1;
        }
        break;
    case State::B1:
        currentState = isCorrect ? State::B2 : State::B1;
        break;
    case State::B2:
        currentState = isCorrect ? State::B_prime : State::B2;
        break;
    case State::B_prime:
        currentRule++;
        if (static_cast<qsizetype>(currentRule) >= sortedGrammar.size()) {
            currentState = State::C;
        } else {
            currentState = State::B;
        }
        break;
    case State::C:
        currentState = isCorrect ? State::fin : State::C;
        break;
    case State::fin:
        QMessageBox::information(this, "Fin", "fin");
        close();
        break;
    }
}

bool LLTutorWindow::verifyResponse(const QString& userResponse) {
    switch (currentState) {
    case State::A:
        return verifyResponseForA(userResponse);
    case State::A1:
        return verifyResponseForA1(userResponse);
    case State::A2:
        return verifyResponseForA2(userResponse);
    case State::A_prime:
        return verifyResponseForA(userResponse);
    case State::B:
        return verifyResponseForB(userResponse);
    case State::B1:
        return verifyResponseForB1(userResponse);
    case State::B2:
        return verifyResponseForB2(userResponse);
    case State::B_prime:
        return verifyResponseForB(userResponse);
    case State::C:
        return verifyResponseForC();
    default:
        return false;
    }
}

bool LLTutorWindow::verifyResponseForA(const QString& userResponse) {
    return userResponse == solutionForA();
}

bool LLTutorWindow::verifyResponseForA1(const QString& userResponse) {
    return userResponse == solutionForA1();
}

bool LLTutorWindow::verifyResponseForA2(const QString& userResponse) {
    return userResponse == solutionForA2();
}

bool LLTutorWindow::verifyResponseForB(const QString& userResponse) {
    QStringList userResponseSplitted = userResponse.split(",", Qt::SkipEmptyParts);
    QSet<QString> userSet (userResponseSplitted.begin(), userResponseSplitted.end());
    return userSet == solutionForB();
}

bool LLTutorWindow::verifyResponseForB1(const QString& userResponse) {
    QStringList userResponseSplitted = userResponse.split(",", Qt::SkipEmptyParts);
    QSet<QString> userSet (userResponseSplitted.begin(), userResponseSplitted.end());
    return userSet == solutionForB1();
}

bool LLTutorWindow::verifyResponseForB2(const QString& userResponse) {
    QStringList userResponseSplitted = userResponse.split(",", Qt::SkipEmptyParts);
    QSet<QString> userSet (userResponseSplitted.begin(), userResponseSplitted.end());
    return userSet == solutionForB2();
}

bool LLTutorWindow::verifyResponseForC() {
    if (lltable.empty()) {
        return false;
    }
    for (const auto &[nonTerminal, columns] : lltable.asKeyValueRange()) {
        for (const auto &[terminal, production] : columns.asKeyValueRange()) {
            qDebug() << "Cell [" << nonTerminal << "][" << terminal
                     << "]: " << production.join(' ');
            const auto &entry = ll1.ll1_t_[nonTerminal.toStdString()][terminal.toStdString()];
            if (production.isEmpty() && entry.empty()) {
                continue;
            }

            if (production != stdVectorToQVector(entry[0])) {
                return false;
            }
        }
    }
    return true;
}

QString LLTutorWindow::solutionForA() {
    int nt = grammar.st_.non_terminals_.size();
    int t = grammar.st_.terminals_.size();
    QString solution (QString::number(nt) + "," + QString::number(t));
    return solution;
}

QString LLTutorWindow::solutionForA1() {
    int nt = grammar.st_.non_terminals_.size();
    QString solution (QString::number(nt));
    return solution;
}

QString LLTutorWindow::solutionForA2() {
    int t = grammar.st_.terminals_.size() - 1;
    QString solution (QString::number(t));
    return solution;
}

QSet<QString> LLTutorWindow::solutionForB() {
    const auto& current = sortedGrammar[currentRule];
    std::unordered_set<std::string> result = ll1.PredictionSymbols(current.first.toStdString(), qvectorToStdVector(current.second));
    QSet<QString> solution = stdUnorderedSetToQSet(result);
    return solution;
}

QSet<QString> LLTutorWindow::solutionForB1() {
    const auto& current = sortedGrammar[currentRule];
    std::unordered_set<std::string> result;
    ll1.First(qvectorToStdVector(current.second), result);
    QSet<QString> solution = stdUnorderedSetToQSet(result);
    return solution;
}

QSet<QString> LLTutorWindow::solutionForB2() {
    const auto& current = sortedGrammar[currentRule];
    std::unordered_set<std::string> result = ll1.Follow(current.first.toStdString());
    QSet<QString> solution = stdUnorderedSetToQSet(result);
    return solution;
}

QString LLTutorWindow::feedback() {
    switch(currentState) {
    case State::A:
        return feedbackForA();
    case State::A1:
        return feedbackForA1();
    case State::A2:
       return feedbackForA2();
    case State::A_prime:
       return feedbackForAPrime();
    case State::B:
        return feedbackForB();
    case State::B1:
        return feedbackForB1();
    case State::B2:
        return feedbackForB2();
    case State::B_prime:
        return feedbackForBPrime();
    case State::C:
        return feedbackForC();
    default:
        return "No feedback provided.";
    }
}

QString LLTutorWindow::feedbackForA() {
    return "La cantidad de filas y columnas a usar depende del número de símbolos terminales y no terminales";
}

QString LLTutorWindow::feedbackForA1() {
    QSet<QString> non_terminals = stdUnorderedSetToQSet(grammar.st_.non_terminals_);
    QList<QString> l(non_terminals.begin(), non_terminals.end());
    return QString("Los símbolos no terminales son: %1").arg(l.join(" "));
}

QString LLTutorWindow::feedbackForA2() {
    QSet<QString> terminals = stdUnorderedSetToQSet(grammar.st_.terminals_wtho_eol_);
    QList<QString> l(terminals.begin(), terminals.end());
    return QString("Los símbolos terminales son: %1").arg(l.join(" "));
}

QString LLTutorWindow::feedbackForAPrime() {
    return QString("Al haber %1 símbolos no terminales y %2 terminales incluyendo el $, la tabla LL(1) tendrá %1 filas y %2 columnas.")
        .arg(grammar.st_.non_terminals_.size())
        .arg(grammar.st_.terminals_.size());
}

QString LLTutorWindow::feedbackForB() {
    return QString("El conjunto de símbolos directores para una regla se define como SD(X -> Y) = CAB(Y) - { epsilon } U SIG(X)");
}

QString LLTutorWindow::feedbackForB1() {
    return QString::fromStdString(ll1.TeachFirst(qvectorToStdVector(sortedGrammar.at(currentRule).second)));
}

QString LLTutorWindow::feedbackForB2() {
    return QString::fromStdString(ll1.TeachFollow(sortedGrammar.at(currentRule).first.toStdString()));
}

QString LLTutorWindow::feedbackForBPrime() {
    const auto& rule = sortedGrammar.at(currentRule);
    return QString::fromStdString(ll1.TeachPredictionSymbols(rule.first.toStdString(), qvectorToStdVector(rule.second)));
}

QString LLTutorWindow::feedbackForC()
{
    if (lltries > 2) {
        return QString::fromStdString(ll1.TeachLL1Table());
    }
    return "La tabla no es correcta. Cada celda se define de la siguiente forma: Tabla[A,β] = α si "
           "β ∈ SD(A -> α).";
}

QString LLTutorWindow::generateQuestion() {
    QPair<QString, QVector<QString>> rule;
    switch (currentState) {
    case State::A:
        return QString("¿Cuántas filas y columnas tiene la tabla LL(1)? Formato: #,#");
    case State::A1:
        return QString("¿Cuántos símbolos no terminales tiene la gramática?");
    case State::A2:
        return QString("¿Cuántos símbolos terminales tiene la gramática?");
    case State::A_prime:
        return QString("¿Cuántas filas y columnas tiene la tabla LL(1)? Formato: #,#");
    case State::B:
        rule = sortedGrammar.at(currentRule);
        return QString("¿Cuáles son los símbolos directores de %1 -> %2?").arg(rule.first).arg(rule.second.join(" "));
    case State::B1:
        rule = sortedGrammar.at(currentRule);
        return QString("¿Cuál es la cabecera del consecuente? %1 -> %2").arg(rule.first).arg(rule.second.join(" "));
    case State::B2:
        rule = sortedGrammar.at(currentRule);
        return QString("¿Cuál es el símbolo siguiente al antecedente? %1 -> %2").arg(rule.first).arg(rule.second.join(" "));
    case State::B_prime:
        rule = sortedGrammar.at(currentRule);
        return QString("Entonces, ¿cuáles son los símbolos directores de %1 -> %2?").arg(rule.first).arg(rule.second.join(" "));
    case State::C:
        showTable();
        ui->userResponse->setDisabled(true);
        ui->confirmButton->setDisabled(true); // verify delegated to showTable
        return "";
        break;
    default:
        return "";
    }
}

QString LLTutorWindow::FormatGrammar(const Grammar& grammar) {
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

void LLTutorWindow::fillSortedGrammar()
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

// HELPER FUNCTIONS ----------------------------------------------------------------------
std::vector<std::string> LLTutorWindow::qvectorToStdVector(const QVector<QString>& qvec) {
    std::vector<std::string> result;
    result.reserve(qvec.size());
    for (const auto& qstr : qvec) {
        result.push_back(qstr.toStdString());
    }
    return result;
}

// Convierte std::vector<std::string> a QVector<QString>
QVector<QString> LLTutorWindow::stdVectorToQVector(const std::vector<std::string>& vec) {
    QVector<QString> result;
    result.reserve(vec.size());
    for (const auto& str : vec) {
        result.push_back(QString::fromStdString(str));
    }
    return result;
}

// Convierte std::unordered_set<std::string> a QSet<QString>
QSet<QString> LLTutorWindow::stdUnorderedSetToQSet(const std::unordered_set<std::string>& uset) {
    QSet<QString> result;
    for (const auto& str : uset) {
        result.insert(QString::fromStdString(str));
    }
    return result;
}

// Convierte QSet<QString> a std::unordered_set<std::string>
std::unordered_set<std::string> LLTutorWindow::qsetToStdUnorderedSet(const QSet<QString>& qset) {
    std::unordered_set<std::string> result;
    for (const auto& qstr : qset) {
        result.insert(qstr.toStdString());
    }
    return result;
}

void LLTutorWindow::on_userResponse_textChanged()
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
