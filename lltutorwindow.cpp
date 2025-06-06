#include "lltutorwindow.h"
#include "tutorialmanager.h"
#include "ui_lltutorwindow.h"
#include <QAbstractButton>
#include <QFontDatabase>
#include <QRandomGenerator>
#include <QRegularExpression>

LLTutorWindow::LLTutorWindow(const Grammar& grammar, TutorialManager* tm,
                             QWidget* parent)
    : QMainWindow(parent), ui(new Ui::LLTutorWindow), grammar(grammar),
      ll1(this->grammar), tm(tm) {
    // ====== Parser & Grammar Setup ===========================
    ll1.CreateLL1Table();
#ifdef QT_DEBUG
    ll1.PrintTable();
#endif
    fillSortedGrammar();

    // ====== UI Setup ==========================================
    ui->setupUi(this);

    // -- Confirm Button Icon & Shadow
    ui->confirmButton->setIcon(QIcon(":/resources/send.svg"));
    auto* shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(10);
    shadow->setOffset(0);
    shadow->setColor(QColor::fromRgb(0, 200, 214));
    ui->confirmButton->setGraphicsEffect(shadow);

    ui->textEdit->setFont(QFontDatabase::font("Noto Sans", "Regular", 12));

    // -- User Response Box
    ui->userResponse->setFont(QFontDatabase::font("Noto Sans", "Regular", 12));
    ui->userResponse->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->userResponse->setPlaceholderText(tr("Introduce aquí tu respuesta."));

    // -- Chat Font
    QFont chatFont = QFontDatabase::font("Noto Sans", "Regular", 12);
    ui->listWidget->setFont(chatFont);
    ui->listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->listWidget->verticalScrollBar()->setSingleStep(10);

    // ====== Grammar Display & Formatting ======================
    formattedGrammar = FormatGrammar(this->grammar);
    ui->gr->setFont(QFontDatabase::font("Noto Sans", "Regular", 14));
    ui->gr->setText(formattedGrammar);

    sortedNonTerminals =
        stdUnorderedSetToQSet(ll1.gr_.st_.non_terminals_).values();
    std::sort(sortedNonTerminals.begin(), sortedNonTerminals.end(),
              [&grammar](const QString& a, const QString& b) {
                  if (a.toStdString() == grammar.axiom_)
                      return true;
                  if (b.toStdString() == grammar.axiom_)
                      return false;
                  return a < b;
              });

    // ====== Progress / State Setup ============================
    ui->cntRight->setText(QString::number(cntRightAnswers));
    ui->cntWrong->setText(QString::number(cntWrongAnswers));

    updateProgressPanel();
    addMessage(tr("La gramática es:\n") + formattedGrammar, false);

    currentState = State::A;
    addDivisorLine("Estado inicial");
    addMessage(generateQuestion(), false);

    ui->userResponse->clear();

    // ====== Signal Connections ================================
    connect(ui->userResponse, &CustomTextEdit::sendRequested, this,
            &LLTutorWindow::on_confirmButton_clicked);

    if (tm) {
        setupTutorial();
    }
}

LLTutorWindow::~LLTutorWindow() {
    delete ui;
}

void LLTutorWindow::exportConversationToPdf(const QString& filePath) {
    QTextDocument doc;
    QString       html;
    doc.setDefaultFont(QFontDatabase::font("Noto Sans", "Regular", 12));
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
                "<span style='background-color:red;'>" + safeText + "</span>";
        } else {
            html += safeText;
        }
        html += "</div>";
    }

    html += "</body></html>";
    html += R"(<div class='page-break'></div>)";

    html += "<h2>" + tr("Cabeceras") + "</h2>";
    for (const auto& nt : std::as_const(sortedNonTerminals)) {
        const auto& first =
            stdUnorderedSetToQSet(ll1.first_sets_[nt.toStdString()]).values();
        html += tr("CAB") + "(" + nt + ") = {";
        html += first.join(",");
        html += "}<br>";
    }

    html += "<h2>" + tr("Siguientes") + "</h2>";
    for (const auto& nt : std::as_const(sortedNonTerminals)) {
        const auto& follow =
            stdUnorderedSetToQSet(ll1.follow_sets_[nt.toStdString()]).values();
        html += tr("SIG") + "(" + nt + ") = {" + follow.join(',') + "}<br>";
    }

    html += "<h2>" + tr("Símbolos directores") + "</h2>";
    for (const auto& [nt, production] : std::as_const(sortedGrammar)) {
        const auto predSymbols =
            stdUnorderedSetToQSet(
                ll1.PredictionSymbols(nt.toStdString(),
                                      qvectorToStdVector(production)))
                .values();
        html += "SD(" + nt + " → " + production.join(' ') + ") = {" +
                predSymbols.join(',') + "}<br>";
    }
    html += R"(<div class='page-break'></div>)";
    html +=
        R"(<div class="container"><table border='1' cellspacing='0' cellpadding='5'>)";
    html += "<tr><th>" + tr("No terminal / Símbolo") + "</th>";
    for (const auto& s : ll1.gr_.st_.terminals_) {
        html += "<th>" + QString::fromStdString(s) + "</th>";
    }
    html += "</tr>";
    for (const auto& nt : std::as_const(sortedNonTerminals)) {
        html += "<tr><td align='center'>" + nt + "</td>";
        for (const auto& s : ll1.gr_.st_.terminals_) {
            html += "<td align='center'>";
            if (ll1.ll1_t_[nt.toStdString()].contains(s)) {
                html += stdVectorToQVector(ll1.ll1_t_[nt.toStdString()][s][0])
                            .join(' ');
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

void LLTutorWindow::updateProgressPanel() {
    QString html = R"(
        <html>
        <body style="font-family: 'Noto Sans'; font-size: 11pt; color: #f0f0f0; background-color: #1e1e1e;">
    )";

    // === CABECERAS (First) ===
    html += "<div style='color:#00ADB5; font-weight:bold; "
            "margin-top:12px;'>Conjuntos CAB"
            ":</div><ul style='margin-left:16px;'>";
    for (const auto& [symbol, cabSet] : userCAB.asKeyValueRange()) {
        html +=
            QString("<li> CAB(%1) = %2</li>").arg(symbol, "{" + cabSet + "}");
    }
    html += "</ul>";

    // === SIGUIENTES (Follow) ===
    html += "<div style='color:#00ADB5; font-weight:bold; "
            "margin-top:12px;'>Conjuntos SIG"
            ":</div><ul style='margin-left:16px;'>";
    for (const auto& [symbol, sigSet] : userSIG.asKeyValueRange()) {
        html +=
            QString("<li> SIG(%1) = %2</li>").arg(symbol, "{" + sigSet + "}");
    }
    html += "</ul>";

    // === SELECTORES ===
    html += "<div style='color:#00ADB5; font-weight:bold; "
            "margin-top:12px;'>Conjuntos SD"
            ":</div><ul style='margin-left:16px;'>";
    for (const auto& [rule, sdSet] : userSD.asKeyValueRange()) {
        html += QString("<li> SD(%1) = %2</li>").arg(rule, "{" + sdSet + "}");
    }
    html += "</ul>";

    html += "</body></html>";

    ui->textEdit->setHtml(html);
}

void LLTutorWindow::addMessage(const QString& text, bool isUser) {
    if (text.isEmpty() && !isUser) {
        return; // Because State C
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

    QLabel* header = new QLabel(isUser ? "Usuario" : "Tutor");
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

void LLTutorWindow::showTableForCPrime() {
    QStringList colHeaders;

    for (const auto& symbol : ll1.gr_.st_.terminals_) {
        if (symbol == ll1.gr_.st_.EPSILON_) {
            continue;
        }
        colHeaders << QString::fromStdString(symbol);
    }
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
    auto*              dialog =
        new LLTableDialog(sortedNonTerminals, colHeaders, this, &rawTable);
    dialog->setStyleSheet(darkQss);

    connect(dialog, &LLTableDialog::submitted, this,
            [this, dialog, colHeaders](const QVector<QVector<QString>>& data) {
                rawTable.clear();
                rawTable = data;

                lltable.clear();

                for (int i = 0; i < rawTable.size(); ++i) {
                    const QString& rowHeader = sortedNonTerminals[i];

                    for (int j = 0; j < rawTable[i].size(); ++j) {
                        const QString& colHeader   = colHeaders[j];
                        const QString& cellContent = rawTable[i][j];

                        if (!cellContent.isEmpty()) {
                            QStringList production = stdVectorToQVector(
                                ll1.gr_.Split(cellContent.toStdString()));
                            if (production.empty() && !cellContent.isEmpty()) {
                                // Split could not process the string
                                production = {cellContent};
                            }
                            lltable[rowHeader][colHeader] = production;
                        }
                    }
                }
                on_confirmButton_clicked();

                dialog->deleteLater();
            });

    connect(dialog, &QDialog::rejected, this, [this, dialog]() {
        rawTable.clear();
        QMessageBox msg(this);
        msg.setWindowTitle(tr("Cancelar tabla LL(1)"));
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

void LLTutorWindow::showTable() {
    QStringList colHeaders;

    for (const auto& symbol : ll1.gr_.st_.terminals_) {
        if (symbol == ll1.gr_.st_.EPSILON_) {
            continue;
        }
        colHeaders << QString::fromStdString(symbol);
    }
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
    auto*              dialog =
        new LLTableDialog(sortedNonTerminals, colHeaders, this, &rawTable);
    dialog->setStyleSheet(darkQss);
    currentDlg = dialog;

    connect(dialog,
            &LLTableDialog::submitted,
            this,
            [this, colHeaders](const QVector<QVector<QString>>& data) {
                handleTableSubmission(data, colHeaders);
            });

    connect(dialog, &QDialog::rejected, this, [this, dialog]() {
        rawTable.clear();
        QMessageBox msg(this);
        msg.setWindowTitle(tr("Cancelar tabla LL(1)"));
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

void LLTutorWindow::handleTableSubmission(const QVector<QVector<QString>>& raw,
                                          const QStringList& colHeaders) {
    rawTable = raw;
    lltable.clear();
    for (int i = 0; i < raw.size(); ++i) {
        const auto& rowH = sortedNonTerminals[i];
        for (int j = 0; j < raw[i].size(); ++j) {
            const auto& colH  = colHeaders[j];
            const auto& cells = raw[i][j];
            if (cells.isEmpty())
                continue;
            QStringList prod =
                stdVectorToQVector(ll1.gr_.Split(cells.toStdString()));
            if (prod.empty())
                prod = {cells};
            lltable[rowH][colH] = prod;
        }
    }

    ++lltries;
    lastWrongCells.clear();
    bool ok = verifyResponseForC();

    if (ok) {
        currentDlg->accept();
        on_confirmButton_clicked();
        currentDlg = nullptr;
        return;
    }

    if (lltries <= kMaxHighlightTries) {
        // convertir (NT,T) -> (fila,col)
        QList<QPair<int, int>> coords;
        for (auto& [nt, t] : lastWrongCells) {
            int r = sortedNonTerminals.indexOf(nt);
            int c = colHeaders.indexOf(t);
            if (r >= 0 && c >= 0)
                coords.append({r, c});
        }
        currentDlg->highlightIncorrectCells(coords);
        QMessageBox::information(
            currentDlg, "Errores",
            "Las celdas marcadas en rojo son incorrectas.");
    } else if (lltries < kMaxTotalTries) {
        QMessageBox::information(
            currentDlg, "Vuelve a intentarlo",
            "Recuerda las reglas de colocación de producciones.");
    } else {
        if (currentDlg)
            currentDlg->accept();
        on_confirmButton_clicked();
        currentDlg = nullptr;
    }
}

void LLTutorWindow::addDivisorLine(const QString& stateName) {
    QWidget*     dividerWidget = new QWidget;
    QHBoxLayout* layout        = new QHBoxLayout(dividerWidget);
    layout->setContentsMargins(10, 5, 10, 5);
    layout->setSpacing(10); // espacio entre líneas y texto

    QFrame* lineLeft = new QFrame;
    lineLeft->setFrameShape(QFrame::HLine);
    lineLeft->setStyleSheet("color: #CCCCCC;");
    lineLeft->setMinimumWidth(20);
    lineLeft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QLabel* label     = new QLabel(stateName);
    QFont   labelFont = QFontDatabase::font("Noto Sans", "Regular", 11);
    labelFont.setItalic(true);
    label->setFont(labelFont);
    label->setStyleSheet(R"(
        color: #888888;
        font-size: 11px;
        background: transparent;
    )");

    QFrame* lineRight = new QFrame;
    lineRight->setFrameShape(QFrame::HLine);
    lineRight->setStyleSheet("color: #CCCCCC;");
    lineRight->setMinimumWidth(20);
    lineRight->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    layout->addWidget(lineLeft);
    layout->addWidget(label);
    layout->addWidget(lineRight);

    QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
    dividerWidget->setLayout(layout);
    item->setSizeHint(dividerWidget->sizeHint());

    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, dividerWidget);
    ui->listWidget->scrollToBottom();
}

void LLTutorWindow::wrongAnimation() {
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

        QObject::connect(animation, &QPropertyAnimation::finished, this,
                         [label]() {
                             if (label && label->graphicsEffect()) {
                                 label->graphicsEffect()->deleteLater();
                                 label->setGraphicsEffect(nullptr);
                             }
                         });
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void LLTutorWindow::wrongUserResponseAnimation() {
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

void LLTutorWindow::animateLabelPop(QLabel* label) {
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

void LLTutorWindow::animateLabelColor(QLabel* label, const QColor& flashColor) {
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

void LLTutorWindow::markLastUserIncorrect() {
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

void LLTutorWindow::on_confirmButton_clicked() {
    QString userResponse;
    bool    isCorrect;
    if (currentState != State::C && currentState != State::C_prime) {
        userResponse = ui->userResponse->toPlainText().trimmed();
        addMessage(userResponse, true);
        isCorrect = verifyResponse(userResponse);
    } else {
        isCorrect = verifyResponseForC();
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

    if (currentState == State::fin) {
        QMessageBox end(this);
        end.setWindowTitle(tr("Fin del ejercicio"));
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
            QString filePath = QFileDialog::getSaveFileName(this,
                                                            tr("Guardar conversación"),
                                                            "conver.pdf",
                                                            tr("Archivo PDF (*.pdf)"));

            if (!filePath.isEmpty()) {
                exportConversationToPdf(filePath);
            }
        } else {
            close();
        }
    }
    ui->userResponse->clear();
    addMessage(generateQuestion(), false);
}

/************************************************************
 *                    GENERATE QUESTION                     *
 * Returns the current question string to be shown to the
 * student based on the internal teaching state.
 ************************************************************/
QString LLTutorWindow::generateQuestion() {
    QPair<QString, QVector<QString>> rule;

    switch (currentState) {
    // ====== A: Estructura de la tabla LL(1) ==================
    case State::A:
        return tr("¿Cuántas filas y columnas tiene la tabla LL(1)?\n"
                  "Formato de respuesta: filas,columnas");

    case State::A1:
        return tr("¿Cuántos símbolos no terminales tiene la gramática?");

    case State::A2:
        return tr("¿Cuántos símbolos terminales tiene la gramática?");

    case State::A_prime:
        return tr("Entonces, basándote en los símbolos identificados,\n"
                  "¿cuántas filas y columnas tiene la tabla LL(1)? Formato: "
                  "filas,columnas");

        // ====== B: Análisis de símbolos directores ===============
    case State::B:
        rule = sortedGrammar.at(currentRule);
        return tr("¿Cuáles son los símbolos directores (SD) de esta regla?\n%1 "
                  "→ %2\n"
                  "Formato: a,b,c")
            .arg(rule.first)
            .arg(rule.second.join(" "));

    case State::B1:
        rule = sortedGrammar.at(currentRule);
        return tr("¿Cuál es el conjunto cabecera (CAB) del consecuente?\n%1 → "
                  "%2\n"
                  "Formato: a,b,c")
            .arg(rule.first)
            .arg(rule.second.join(" "));

    case State::B2:
        rule = sortedGrammar.at(currentRule);
        return tr("¿Cuál es el conjunto SIG (símbolos siguientes) del "
                  "antecedente?\n%1 → %2\n"
                  "Formato: a,b,c")
            .arg(rule.first)
            .arg(rule.second.join(" "));

    case State::B_prime:
        rule = sortedGrammar.at(currentRule);
        return tr("Entonces, ¿cuáles son los símbolos directores (SD) de la "
                  "regla?\n%1 → %2\n"
                  "Formato: a,b,c")
            .arg(rule.first)
            .arg(rule.second.join(" "));

        // ====== C: Mostrar tabla final al alumno =================
    case State::C:
        addMessage(tr("Rellena la tabla LL(1), en el panel derecho puedes "
                      "consultar todos los "
                      "cálculos que has realizado durante el ejercicio."),
                   false);
        lastUserMessage = nullptr;
        ui->userResponse->setDisabled(true);
        ui->confirmButton->setDisabled(true); // handled externally
        showTable();
        return "";

    case State::C_prime:
        showTableForCPrime();
        return "";
        // ====== Fallback case ====================================
    default:
        return "";
    }
}

/************************************************************
 *                     UPDATE STATE                         *
 * Updates the tutor's current state based on whether the
 * user's response was correct. Controls the progression
 * through the LL(1) pedagogical flow.
 ************************************************************/
void LLTutorWindow::updateState(bool isCorrect) {
    switch (currentState) {
    // ====== A: Structure-related questions (table size) ======
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
        currentState = State::B;
        break;

    // ====== B: Questions about prediction set (SD), FIRST, FOLLOW ======
    case State::B: {
        if (isCorrect) {
            QString key = sortedGrammar.at(currentRule).first + " -> " +
                          sortedGrammar.at(currentRule).second.join(' ');
            userSD[key] = solutionForB().values().join(", ");
            userCAB[sortedGrammar.at(currentRule).second.join(' ')] =
                solutionForB1().values().join(", ");
            userSIG[sortedGrammar.at(currentRule).first] =
                solutionForB2().values().join(", ");
            updateProgressPanel();
            currentRule++;
            currentState =
                static_cast<qsizetype>(currentRule) >= sortedGrammar.size()
                    ? State::C
                    : State::B;
        } else {
            currentState = State::B1;
        }
        break;
    }
    case State::B1:
        if (isCorrect) {
            userCAB[sortedGrammar.at(currentRule).second.join(' ')] =
                solutionForB1().values().join(", ");
            updateProgressPanel();
        }
        currentState = isCorrect ? State::B2 : State::B1;
        break;

    case State::B2:
        if (isCorrect) {
            userSIG[sortedGrammar.at(currentRule).first] =
                solutionForB2().values().join(", ");
            updateProgressPanel();
        }
        currentState = isCorrect ? State::B_prime : State::B2;
        break;

    case State::B_prime: {
        QString key = sortedGrammar.at(currentRule).first + " -> " +
                      sortedGrammar.at(currentRule).second.join(' ');
        userSD[key] = solutionForB().values().join(", ");
        updateProgressPanel();
        currentRule++;
        currentState =
            static_cast<qsizetype>(currentRule) >= sortedGrammar.size()
                ? State::C
                : State::B;
        break;
    }
    case State::C:
        if (lltries >= kMaxTotalTries) {
            currentState = State::C_prime;
            break;
        }
        currentState = isCorrect ? State::fin : State::C;
        break;

    case State::C_prime:
        currentState = isCorrect ? State::fin : State::C_prime;
        break;

    // ====== Final state: ends the tutor ======
    case State::fin:
        QMessageBox::information(this, "Fin", "fin");
        close();
        break;
    }
}

/************************************************************
 *                  VERIFY USER RESPONSE                    *
 * Dispatches validation to the appropriate method based on
 * the current tutor state. Each state has its own logic.
 ************************************************************/
bool LLTutorWindow::verifyResponse(const QString& userResponse) {
    switch (currentState) {
    // ====== A: Table structure questions ======
    case State::A:
        return verifyResponseForA(userResponse);
    case State::A1:
        return verifyResponseForA1(userResponse);
    case State::A2:
        return verifyResponseForA2(userResponse);
    case State::A_prime:
        return verifyResponseForA(userResponse);

    // ====== B: Rule prediction and set calculations ======
    case State::B:
        return verifyResponseForB(userResponse);
    case State::B1:
        return verifyResponseForB1(userResponse);
    case State::B2:
        return verifyResponseForB2(userResponse);
    case State::B_prime:
        return verifyResponseForB(userResponse);

    // ====== C: Final LL(1) table ======
    case State::C:
    case State::C_prime:
        return verifyResponseForC();

    // ====== Fallback ======
    default:
        return false;
    }
}

bool LLTutorWindow::verifyResponseForA(const QString& userResponse) {
    QStringList userResp = userResponse.split(',', Qt::SkipEmptyParts);
    return userResp == solutionForA();
}

bool LLTutorWindow::verifyResponseForA1(const QString& userResponse) {
    return userResponse == solutionForA1();
}

bool LLTutorWindow::verifyResponseForA2(const QString& userResponse) {
    return userResponse == solutionForA2();
}

bool LLTutorWindow::verifyResponseForB(const QString& userResponse) {
    QStringList userResponseSplitted =
        userResponse.split(",", Qt::SkipEmptyParts);
    QSet<QString> userSet;
    for (const auto& s : std::as_const(userResponseSplitted)) {
        userSet.insert(s.trimmed());
    }
    return userSet == solutionForB();
}

bool LLTutorWindow::verifyResponseForB1(const QString& userResponse) {
    QStringList userResponseSplitted =
        userResponse.split(",", Qt::SkipEmptyParts);
    QSet<QString> userSet;
    for (const auto& s : std::as_const(userResponseSplitted)) {
        userSet.insert(s.trimmed());
    }
    return userSet == solutionForB1();
}

bool LLTutorWindow::verifyResponseForB2(const QString& userResponse) {
    QStringList userResponseSplitted =
        userResponse.split(",", Qt::SkipEmptyParts);
    QSet<QString> userSet;
    for (const auto& s : std::as_const(userResponseSplitted)) {
        userSet.insert(s.trimmed());
    }
    return userSet == solutionForB2();
}

bool LLTutorWindow::verifyResponseForC() {
    lastWrongCells.clear();

    for (const auto& [nonTerminal, columns] : ll1.ll1_t_) {
        const QString nt = QString::fromStdString(nonTerminal);

        for (const auto& [terminal, productions] : columns) {
            const QString t        = QString::fromStdString(terminal);
            const auto&   expected = productions[0];

            const QStringList entry = lltable.value(nt).value(t);

            if (expected.empty() && entry.isEmpty())
                continue;

            if (expected != qvectorToStdVector(entry))
                lastWrongCells.emplace_back(nt, t);
        }
    }

    for (auto itNT = lltable.cbegin(); itNT != lltable.cend(); ++itNT) {
        const QString nt      = itNT.key();
        auto          itSysNT = ll1.ll1_t_.find(nt.toStdString());

        for (auto itT = itNT->cbegin(); itT != itNT->cend(); ++itT) {
            const QString t = itT.key();

            bool wrong = false;

            if (itSysNT == ll1.ll1_t_.end()) {
                wrong = true;
            } else {
                auto itSysT = itSysNT->second.find(t.toStdString());
                if (itSysT == itSysNT->second.end()) {
                    wrong = true;
                } else if (itSysT->second[0].empty()) {
                    wrong = true;
                }
            }
            if (wrong)
                lastWrongCells.emplace_back(nt, t);
        }
    }

    return lastWrongCells.empty();
}

QStringList LLTutorWindow::solutionForA() {
    int nt = grammar.st_.non_terminals_.size();
    int t  = grammar.st_.terminals_.contains(grammar.st_.EPSILON_)
                 ? grammar.st_.terminals_.size() - 1
                 : grammar.st_.terminals_.size();
    return {QString::number(nt), QString::number(t)};
}

QString LLTutorWindow::solutionForA1() {
    int     nt = grammar.st_.non_terminals_.size();
    QString solution(QString::number(nt));
    return solution;
}

QString LLTutorWindow::solutionForA2() {
    int t = grammar.st_.terminals_.contains(grammar.st_.EPSILON_)
                ? grammar.st_.terminals_.size() - 2
                : grammar.st_.terminals_.size() - 1;

    QString solution(QString::number(t));
    return solution;
}

QSet<QString> LLTutorWindow::solutionForB() {
    const auto&                     current = sortedGrammar[currentRule];
    std::unordered_set<std::string> result  = ll1.PredictionSymbols(
        current.first.toStdString(), qvectorToStdVector(current.second));
    QSet<QString> solution = stdUnorderedSetToQSet(result);
    return solution;
}

QSet<QString> LLTutorWindow::solutionForB1() {
    const auto&                     current = sortedGrammar[currentRule];
    std::unordered_set<std::string> result;
    ll1.First(qvectorToStdVector(current.second), result);
    QSet<QString> solution = stdUnorderedSetToQSet(result);
    return solution;
}

QSet<QString> LLTutorWindow::solutionForB2() {
    const auto&                     current = sortedGrammar[currentRule];
    std::unordered_set<std::string> result =
        ll1.Follow(current.first.toStdString());
    QSet<QString> solution = stdUnorderedSetToQSet(result);
    return solution;
}

/************************************************************
 *                          FEEDBACK                        *
 * Returns appropriate explanatory feedback based on the
 * current state of the LL(1) tutor. Shown when the user's
 * answer is incorrect or guidance is requested.
 ************************************************************/
QString LLTutorWindow::feedback() {
    switch (currentState) {
    // ====== A: Table structure questions ======
    case State::A:
        return feedbackForA();
    case State::A1:
        return feedbackForA1();
    case State::A2:
        return feedbackForA2();
    case State::A_prime:
        return feedbackForAPrime();

    // ====== B: Rule-wise set analysis and prediction =========
    case State::B:
        return feedbackForB();
    case State::B1:
        return feedbackForB1();
    case State::B2:
        return feedbackForB2();
    case State::B_prime:
        return feedbackForBPrime();

    // ====== C: LL(1) table validation ======
    case State::C:
        return feedbackForC();

    case State::C_prime:
        return feedbackForCPrime();

    // ====== Fallback case ======
    default:
        return "No feedback provided.";
    }
}

QString LLTutorWindow::feedbackForA() {
    QString feedback = tr(
        "La tabla LL(1) tiene:\n"
        " - Una fila por cada símbolo NO TERMINAL\n"
        " - Una columna por cada TERMINAL (incluyendo $ y excluyendo EPSILON)\n"
        "Esto define el tamaño de la tabla como filas × columnas.");

    QString userText = ui->userResponse->toPlainText().trimmed();

    if (!userText.isEmpty()) {
        QStringList resp = userText.split(',', Qt::SkipEmptyParts);

        if (resp.size() == 1 && resp[0] == userText) {
            return tr("Parece que no has seguido el formato correctamente. "
                      "Debes separar el número "
                      "de "
                      "filas y columnas con una coma.\n") +
                   feedback;
        } else {
            if (resp.size() != 2) {
                return tr("No has seguido el formato correspondiente "
                          "(filas,columnas).\n") +
                       feedback;
            } else {
                QStringList sol = solutionForA();
                if (sol[0] == resp[0] && sol[1] != resp[1]) {
                    return tr("No has contado bien el número de símbolos "
                              "terminales.\n") +
                           feedback;
                } else if (sol[0] != resp[0] && sol[1] == resp[1]) {
                    return tr("No has contado bien el número de símbolos no "
                              "terminales.\n") +
                           feedback;
                } else {
                    return feedback;
                }
            }
        }
    } else {
        return feedback;
    }
}

QString LLTutorWindow::feedbackForA1() {
    QSet<QString> non_terminals =
        stdUnorderedSetToQSet(grammar.st_.non_terminals_);
    QList<QString> l(non_terminals.begin(), non_terminals.end());
    return tr("Los NO TERMINALES son los que aparecen como antecedente en "
              "alguna regla.\n"
              "En esta gramática: %1")
        .arg(l.join(", "));
}

QString LLTutorWindow::feedbackForA2() {
    QSet<QString> terminals =
        stdUnorderedSetToQSet(grammar.st_.terminals_wtho_eol_);
    QList<QString> l(terminals.begin(), terminals.end());

    if (ll1.gr_.st_.terminals_.contains(ll1.gr_.st_.EPSILON_)) {
        l.removeOne(ll1.gr_.st_.EPSILON_);
        return tr("Los TERMINALES son todos los símbolos que aparecen en los "
                  "consecuentes\n"
                  "y que NO son no terminales, excluyendo el símbolo de fin de "
                  "entrada ($). La "
                  "cadena EPSILON, tampoco cuenta como símbolo terminal, pues "
                  "es un metasímbolo "
                  "que representa la cadena vacía.\n"
                  "En esta gramática: %1")
            .arg(l.join(", "));
    } else {
        return tr("Los TERMINALES son todos los símbolos que aparecen en los "
                  "consecuentes\n"
                  "y que NO son no terminales, excluyendo el símbolo de fin de "
                  "entrada ($).\n"
                  "En esta gramática: %1")
            .arg(l.join(", "));
    }
}

QString LLTutorWindow::feedbackForAPrime() {
    QStringList sol = solutionForA();
    return tr("Como hay %1 símbolos no terminales (filas) y %2 terminales "
              "(columnas, "
              "incluyendo $ y excluyendo EPSILON),\n"
              "el tamaño de la tabla LL(1) será: %1.")
        .arg(sol[0])
        .arg(sol[1]);
}

QString LLTutorWindow::feedbackForB() {
    QString feedbackBase =
        tr("Para una regla X → Y, sus símbolos directores (SD) indican "
           "en qué columnas debe colocarse la producción en la tabla LL(1).\n"
           "La fórmula es: SD(X → Y) = CAB(Y) - {ε} ∪ SIG(X) si ε ∈ CAB(Y)");

    QStringList resp =
        ui->userResponse->toPlainText()
            .trimmed()
            .split(',', Qt::SkipEmptyParts)
            .replaceInStrings(QRegularExpression("^\\s+|\\s+$"), "");
    QSet<QString> setSol = solutionForB();
    QSet<QString> setResp(resp.begin(), resp.end());

    if (resp.isEmpty()) {
        return tr("No has indicado ningún símbolo director.\n") + feedbackBase;
    }
    if (resp.size() == 1 && resp[0].contains(' ')) {
        return tr("Parece que no has separado los símbolos con comas "
                  "correctamente.\n") +
               feedbackBase;
    }

    if (resp.contains(QString::fromStdString(ll1.gr_.st_.EPSILON_))) {
        return tr("Has introducido EPSILON, los símbolos directores no pueden "
                  "contenerlo.\n") +
               feedbackBase;
    }

    QSet<QString> missing = setSol - setResp;
    QSet<QString> rest    = setResp - setSol;
    QString       msg;
    if (!missing.isEmpty()) {
        msg += tr("Te han faltado símbolos.\n");
    }
    if (!rest.isEmpty()) {
        msg += tr("Has incluido símbolos que no corresponden: ") +
               QStringList(rest.values()).join(", ") + ".\n";
    }
    return msg + feedbackBase;
}

void LLTutorWindow::feedbackForB1TreeGraphics() {
    std::unordered_set<std::string> first_set;
    std::vector<std::pair<std::string, std::vector<std::string>>>
        active_derivations;

    auto treeroot =
        buildTreeNode(qvectorToStdVector(sortedGrammar.at(currentRule).second),
                      first_set, 0, active_derivations);

    showTreeGraphics(std::move(treeroot));
}

QString LLTutorWindow::feedbackForB1() {
    feedbackForB1TreeWidget();
    feedbackForB1TreeGraphics();
    std::unordered_set<std::string> result;
    ll1.First(qvectorToStdVector(sortedGrammar.at(currentRule).second), result);
    QString cab       = sortedGrammar.at(currentRule).second.join(' ');
    QString resultSet = stdUnorderedSetToQSet(result).values().join(", ");
    return tr("Se calcula CABECERA del consecuente: CAB(%1)\n"
              "Con esto se obtienen los terminales que pueden aparecer al "
              "comenzar a derivar %1.\n"
              "Resultado: { %2 }")
        .arg(cab, resultSet);
}

QString LLTutorWindow::feedbackForB2() {
    const QString nt = sortedGrammar.at(currentRule).first;
    QString       feedbackBase =
        tr("Cuando CAB(α) contiene ε, se necesita SIG(%1) para completar "
           "los símbolos directores.\n%2")
            .arg(nt, TeachFollow(nt));

    QStringList resp =
        ui->userResponse->toPlainText()
            .trimmed()
            .split(',', Qt::SkipEmptyParts)
            .replaceInStrings(QRegularExpression("^\\s+|\\s+$"), "");
    QSet<QString> setSol = solutionForB2();
    QSet<QString> setResp(resp.begin(), resp.end());

    if (resp.isEmpty()) {
        return tr("No has indicado ningún símbolo de SIG(%1).\n").arg(nt) +
               feedbackBase;
    }

    if (resp.size() == 1 &&
        resp[0] == ui->userResponse->toPlainText().trimmed()) {
        return tr("Recuerda separar los símbolos de SIG(%1) con comas.\n")
                   .arg(nt) +
               feedbackBase;
    }

    QSet<QString> missing = setSol - setResp;
    QSet<QString> rest    = setResp - setSol;
    QString       msg;
    if (!missing.isEmpty()) {
        msg += tr("Te han faltado símbolos.\n");
    }
    if (!rest.isEmpty()) {
        msg += tr("No forman parte de SIG(%1): %2.\n")
                   .arg(nt, QStringList(rest.values()).join(", "));
    }

    return msg + feedbackBase;
}

QString LLTutorWindow::feedbackForBPrime() {
    const auto& rule = sortedGrammar.at(currentRule);
    QString     feedbackBase =
        tr("Un símbolo director indica cuándo se puede aplicar una "
           "producción durante el análisis.\n"
           "%1")
            .arg(TeachPredictionSymbols(rule.first,
                                        qvectorToStdVector(rule.second)));

    QStringList resp =
        ui->userResponse->toPlainText()
            .trimmed()
            .split(',', Qt::SkipEmptyParts)
            .replaceInStrings(QRegularExpression("^\\s+|\\s+$"), "");
    QSet<QString> setSol = solutionForB();
    QSet<QString> setResp(resp.begin(), resp.end());

    if (resp.isEmpty()) {
        return tr("No has indicado ningún símbolo director.\n") + feedbackBase;
    }
    if (resp.size() == 1 && resp[0].contains(' ')) {
        return tr("No has seguido el formato indicado (símbolos separados por "
                  "coma).\n") +
               feedbackBase;
    }

    QSet<QString> missing = setSol - setResp;
    QSet<QString> rest    = setResp - setSol;
    QString       msg;
    if (!missing.isEmpty()) {
        msg += tr("Te han faltado estos símbolos directores: ") +
               QStringList(missing.values()).join(", ") + ".\n";
    }
    if (!rest.isEmpty()) {
        msg += tr("Estos no son símbolos directores válidos: ") +
               QStringList(rest.values()).join(", ") + ".\n";
    }
    return msg + feedbackBase;
}

QString LLTutorWindow::feedbackForC() {
    return tr("La tabla tiene errores.\n"
              "Recuerda: una producción A → α se coloca en la celda (A, β) si "
              "β ∈ SD(A → α).\n"
              "Si ε ∈ CAB(α), también debe colocarse en (A, b) para cada b ∈ "
              "SIG(A). Se ha marcado en "
              "rojo las celdas incorrectas.");
}

QString LLTutorWindow::feedbackForCPrime() {
    return TeachLL1Table();
}

void LLTutorWindow::addWidgetMessage(QWidget* widget) {
    QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(widget->sizeHint());
    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, widget);
}

void LLTutorWindow::feedbackForB1TreeWidget() {
    QTreeWidget* treeWidgetFeedback = new QTreeWidget();
    treeWidgetFeedback->setHeaderLabel(tr("Derivación de CABECERA"));

    QTreeWidgetItem* root = new QTreeWidgetItem({QString::fromStdString(
        "CAB(" + sortedGrammar.at(currentRule).second.join(' ').toStdString() +
        ")")});

    treeWidgetFeedback->addTopLevelItem(root);

    std::unordered_set<std::string> first_set;
    std::unordered_set<std::string> processing;
    TeachFirstTree(qvectorToStdVector(sortedGrammar.at(currentRule).second),
                   first_set, 0, processing, root);
    treeWidgetFeedback->setStyleSheet(R"(
   QTreeWidget {
        background-color: #1F1F1F;
        color: #E0E0E0;
        font: 10pt "Noto Sans";
        border: none;
        outline: 0;
    }

    QTreeView::item {
        padding: 6px 10px;
        margin: 2px;
        border-radius: 6px;
    }

    QTreeView::item:hover {
        background-color: #333333;
    }

    QTreeView::item:selected {
        background-color: #0078D7;
        color: white;
    }

    QHeaderView::section {
        background-color: #2A2A2A;
        color: #CCCCCC;
        padding: 4px;
        border: none;
        font-weight: bold;
    }
    )");
    treeWidgetFeedback->setVerticalScrollMode(
        QAbstractItemView::ScrollPerPixel);
    // treeWidgetFeedback->resize(500, 300); // Opcional: tamaño fijo

    addWidgetMessage(treeWidgetFeedback);
}

QString LLTutorWindow::FormatGrammar(const Grammar& grammar) {
    QString                                        result;
    const std::string&                             axiom = grammar.axiom_;
    std::map<std::string, std::vector<production>> sortedRules(
        grammar.g_.begin(), grammar.g_.end());

    auto formatProductions = [](const QString&                 lhs,
                                const std::vector<production>& prods) {
        QString out;
        QString header     = lhs + " → ";
        int     indentSize = header.length();
        QString indent(indentSize, ' ');

        bool first = true;
        for (const auto& prod : prods) {
            if (first) {
                out += header;
                first = false;
            } else {
                out += indent + "| ";
            }

            for (const auto& symbol : prod) {
                out += QString::fromStdString(symbol) + " ";
            }
            out = out.trimmed();
            out += "\n";
        }
        return out;
    };

    auto axIt = grammar.g_.find(axiom);
    if (axIt != grammar.g_.end()) {
        result +=
            formatProductions(QString::fromStdString(axiom), axIt->second);
    }

    for (const auto& [lhs, productions] : sortedRules) {
        if (lhs == axiom)
            continue;
        result += formatProductions(QString::fromStdString(lhs), productions);
    }

    return result;
}

void LLTutorWindow::fillSortedGrammar() {
    auto it = grammar.g_.find(grammar.axiom_);
    QVector<QPair<QString, QVector<QString>>> rules;
    QPair<QString, QVector<QString>>          rule(
        {QString::fromStdString(grammar.axiom_), {}});

    if (it != grammar.g_.end()) {
        for (const auto& prod : it->second) {
            for (const auto& symbol : prod) {
                rule.second.push_back(QString::fromStdString(symbol));
            }
        }
    }
    rules.push_back(rule);
    std::map<std::string, std::vector<production>> sortedRules(
        grammar.g_.begin(), grammar.g_.end());
    for (const auto& [lhs, productions] : sortedRules) {
        if (lhs == grammar.axiom_)
            continue;
        rule = {QString::fromStdString(lhs), {}};
        for (const auto& prod : productions) {
            for (const auto& symbol : prod) {
                rule.second.push_back(QString::fromStdString(symbol));
            }
            rules.push_back(rule);
            rule = {QString::fromStdString(lhs), {}};
        }
    }
    sortedGrammar = rules;
}

// HELPER FUNCTIONS
// ----------------------------------------------------------------------
std::vector<std::string>
LLTutorWindow::qvectorToStdVector(const QVector<QString>& qvec) {
    std::vector<std::string> result;
    result.reserve(qvec.size());
    for (const auto& qstr : qvec) {
        result.push_back(qstr.toStdString());
    }
    return result;
}

// Convierte std::vector<std::string> a QVector<QString>
QVector<QString>
LLTutorWindow::stdVectorToQVector(const std::vector<std::string>& vec) {
    QVector<QString> result;
    result.reserve(vec.size());
    for (const auto& str : vec) {
        result.push_back(QString::fromStdString(str));
    }
    return result;
}

// Convierte std::unordered_set<std::string> a QSet<QString>
QSet<QString> LLTutorWindow::stdUnorderedSetToQSet(
    const std::unordered_set<std::string>& uset) {
    QSet<QString> result;
    for (const auto& str : uset) {
        result.insert(QString::fromStdString(str));
    }
    return result;
}

// Convierte QSet<QString> a std::unordered_set<std::string>
std::unordered_set<std::string>
LLTutorWindow::qsetToStdUnorderedSet(const QSet<QString>& qset) {
    std::unordered_set<std::string> result;
    for (const auto& qstr : qset) {
        result.insert(qstr.toStdString());
    }
    return result;
}

void LLTutorWindow::on_userResponse_textChanged() {
    QTextDocument* doc = ui->userResponse->document();
    QFontMetrics   fm(ui->userResponse->font());

    const int lineHeight = fm.lineSpacing();
    const int maxLines   = 4;
    const int minLines   = 1;

    int lineCount = doc->blockCount();
    lineCount     = std::clamp(lineCount, minLines, maxLines);

    int padding       = 20;
    int desiredHeight = lineCount * lineHeight + padding;

    // Establecer mínimo fijo (respetado por el layout)
    const int minHeight = 45;
    ui->userResponse->setMinimumHeight(minHeight);

    // Animar el cambio de altura real
    QPropertyAnimation* animation =
        new QPropertyAnimation(ui->userResponse, "minimumHeight");
    animation->setDuration(120);
    animation->setStartValue(ui->userResponse->height());
    animation->setEndValue(
        std::max(minHeight, desiredHeight)); // nunca menos de minHeight
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    // Establece también el máximo para limitar el crecimiento
    ui->userResponse->setMaximumHeight(maxLines * lineHeight + padding);
}

void LLTutorWindow::TeachFirstTree(const std::vector<std::string>&  symbols,
                                   std::unordered_set<std::string>& first_set,
                                   int                              depth,
                                   std::unordered_set<std::string>& processing,
                                   QTreeWidgetItem*                 parent) {
    if (symbols.empty())
        return;

    std::string              current_symbol = symbols[0];
    std::vector<std::string> remaining_symbols(symbols.begin() + 1,
                                               symbols.end());

    auto* node = new QTreeWidgetItem();
    node->setText(0, tr("Paso %1: %2")
                         .arg(depth + 1)
                         .arg(QString::fromStdString(current_symbol)));
    parent->addChild(node);

    if (ll1.gr_.st_.IsTerminal(current_symbol)) {
        if (current_symbol == ll1.gr_.st_.EPSILON_ &&
            !remaining_symbols.empty()) {
            return;
        }
        if (current_symbol == ll1.gr_.st_.EOL_) {
            node->addChild(new QTreeWidgetItem({tr("Añadir $, se ha llegado al final")}));
        } else {
            node->addChild(
                new QTreeWidgetItem({tr("Terminal → Añadir a CAB")}));
        }
        return;
    }

    if (processing.contains(current_symbol)) {
        node->addChild(new QTreeWidgetItem(
            {tr("Evitando ciclo en %1")
                 .arg(QString::fromStdString(current_symbol))}));
        return;
    }

    processing.insert(current_symbol);
    const auto& productions = ll1.gr_.g_.at(current_symbol);
    for (const auto& prod : productions) {
        std::string prod_str = current_symbol + " → ";
        for (const auto& s : prod)
            prod_str += s + " ";

        auto* prod_node =
            new QTreeWidgetItem({QString::fromStdString(prod_str)});
        node->addChild(prod_node);

        std::vector<std::string> new_symbols = prod;
        new_symbols.insert(new_symbols.end(), remaining_symbols.begin(),
                           remaining_symbols.end());
        TeachFirstTree(new_symbols, first_set, depth + 1, processing,
                       prod_node);

        if (std::find(prod.begin(), prod.end(), ll1.gr_.st_.EPSILON_) !=
            prod.end()) {
            auto* eps_node = new QTreeWidgetItem(
                {tr("Contiene ε → seguir con resto: %1")
                     .arg(stdVectorToQVector(remaining_symbols).join(' '))});
            prod_node->addChild(eps_node);
            TeachFirstTree(remaining_symbols, first_set, depth + 1, processing,
                           eps_node);
        }
    }

    processing.erase(current_symbol);
}

std::unique_ptr<LLTutorWindow::TreeNode> LLTutorWindow::buildTreeNode(
    const std::vector<std::string>&  symbols,
    std::unordered_set<std::string>& first_set, int depth,
    std::vector<std::pair<std::string, std::vector<std::string>>>&
        active_derivations) {
    if (symbols.empty())
        return nullptr;

    std::string              current = symbols[0];
    std::vector<std::string> rest(symbols.begin() + 1, symbols.end());

    auto node = std::make_unique<TreeNode>();
    node->label =
        tr("CAB(%1%2)")
            .arg(QString::fromStdString(current))
            .arg(rest.empty() ? "" : " " + stdVectorToQVector(rest).join(' '));

    if (ll1.gr_.st_.IsTerminal(current)) {
        if (current == ll1.gr_.st_.EPSILON_ && !rest.empty()) {
            return nullptr;
        }
        auto child = std::make_unique<TreeNode>();
        child->label = tr("Añadir %1 a CAB").arg(QString::fromStdString(current));
        node->children.push_back(std::move(child));
        return node;
    }

    for (const auto& prod : ll1.gr_.g_.at(current)) {
        auto derivation_key = std::make_pair(current, prod);

        if (std::count(active_derivations.begin(), active_derivations.end(),
                       derivation_key) > 0) {
            auto cycle   = std::make_unique<TreeNode>();
            cycle->label = tr("Evitar ciclo: %1 → %2")
                               .arg(QString::fromStdString(current))
                               .arg(stdVectorToQVector(prod).join(' '));
            node->children.push_back(std::move(cycle));
            continue;
        }

        active_derivations.push_back(derivation_key);

        QString prodStr = QString::fromStdString(current) + " → " +
                          stdVectorToQVector(prod).join(' ');
        auto prodNode                     = std::make_unique<TreeNode>();
        prodNode->label                   = prodStr;
        std::vector<std::string> new_syms = prod;
        new_syms.insert(new_syms.end(), rest.begin(), rest.end());

        if (auto sub = buildTreeNode(new_syms, first_set, depth + 1,
                                     active_derivations))
            prodNode->children.push_back(std::move(sub));

        if (std::find(prod.begin(), prod.end(), ll1.gr_.st_.EPSILON_) !=
            prod.end()) {
            auto epsNode   = std::make_unique<TreeNode>();
            epsNode->label = tr("ε → continuar con: %1")
                                 .arg(stdVectorToQVector(rest).join(' '));
            if (auto sub = buildTreeNode(rest, first_set, depth + 1,
                                         active_derivations))
                epsNode->children.push_back(std::move(sub));
            prodNode->children.push_back(std::move(epsNode));
        }

        node->children.push_back(std::move(prodNode));
        active_derivations.pop_back();
    }
    return node;
}

int LLTutorWindow::computeSubtreeWidth(const std::unique_ptr<TreeNode>& node,
                                       int hSpacing) {
    if (!node || node->children.empty())
        return hSpacing;

    int width = 0;
    for (const auto& child : node->children) {
        width += computeSubtreeWidth(child, hSpacing);
    }
    return std::max(width, hSpacing);
}

void LLTutorWindow::drawTree(const std::unique_ptr<TreeNode>& root,
                             QGraphicsScene* scene, QPointF pos, int hSpacing,
                             int vSpacing) {
    if (!root)
        return;

    QGraphicsTextItem* textItem = scene->addText(root->label);
    QFont              font("Noto Sans", 10);
    font.setBold(true);
    textItem->setFont(font);
    textItem->setDefaultTextColor(Qt::white);

    QRectF  textRect    = textItem->boundingRect();
    QPointF centeredPos = pos - QPointF(textRect.width() / 2, 0);
    textItem->setPos(centeredPos);

    if (root->children.empty())
        return;

    // Subtree width
    int              totalWidth = 0;
    std::vector<int> subtreeWidths;
    for (const auto& child : root->children) {
        int w = computeSubtreeWidth(child, hSpacing);
        subtreeWidths.push_back(w);
        totalWidth += w;
    }

    int  xOffset = pos.x() - totalWidth / 2;
    int  yChild  = pos.y() + vSpacing;
    QPen pen(Qt::white);

    for (size_t i = 0; i < root->children.size(); ++i) {
        int     childWidth   = subtreeWidths[i];
        int     childCenterX = xOffset + childWidth / 2;
        QPointF childPos(childCenterX, yChild);

        scene->addLine(pos.x(), pos.y() + textRect.height(), childPos.x(),
                       childPos.y(), pen);
        drawTree(root->children[i], scene, childPos, hSpacing, vSpacing);

        xOffset += childWidth;
    }
}

#include <QWheelEvent>
bool LLTutorWindow::eventFilter(QObject* obj, QEvent* event) {
    if (auto* view = qobject_cast<QGraphicsView*>(obj)) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
            if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
                double scaleFactor = 1.15;
                if (wheelEvent->angleDelta().y() > 0)
                    view->scale(scaleFactor, scaleFactor);
                else
                    view->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
                return true; // Se maneja el evento
            }
        }

        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent    = static_cast<QKeyEvent*>(event);
            double     scaleFactor = 1.15;

            if (keyEvent->modifiers() & Qt::ControlModifier) {
                if (keyEvent->key() == Qt::Key_Plus ||
                    keyEvent->key() == Qt::Key_Equal) {
                    view->scale(scaleFactor, scaleFactor);
                    return true;
                } else if (keyEvent->key() == Qt::Key_Minus) {
                    view->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
                    return true;
                } else if (keyEvent->key() == Qt::Key_0) {
                    view->resetTransform();
                    view->fitInView(view->scene()->itemsBoundingRect(),
                                    Qt::KeepAspectRatio);
                    return true;
                }
            }
        }
    }

    return QWidget::eventFilter(obj, event); // por defecto
}

void LLTutorWindow::showTreeGraphics(
    std::unique_ptr<LLTutorWindow::TreeNode> root) {
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Árbol de derivación CABECERA");

    QGraphicsScene* scene = new QGraphicsScene(dialog);

    drawTree(root, scene, QPointF(0, 0), 220, 100);

    QGraphicsView* view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setMinimumSize(1000, 700);
    view->setAlignment(Qt::AlignCenter);
    view->installEventFilter(this);
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->addWidget(view);
    dialog->setLayout(layout);

    dialog->show();
}

QString LLTutorWindow::TeachFollow(const QString& nt) {
    QString output;
    output += tr("Encontrar los símbolos siguientes a %1:\n").arg(nt);

    const std::string nt_str = nt.toStdString();

    if (nt_str == ll1.gr_.axiom_) {
        output += tr("Como %1 es el axioma, SIG(%1) = { %2 }\n")
                      .arg(nt, QString::fromStdString(ll1.gr_.st_.EOL_));
        return output;
    }

    // Step 1: Find all rules where the non-terminal appears in the consequent
    std::vector<std::pair<std::string, production>> rules_with_nt;
    for (const auto& [antecedent, productions] : ll1.gr_.g_) {
        for (const auto& prod : productions) {
            auto it = std::find(prod.begin(), prod.end(), nt_str);
            if (it != prod.end()) {
                rules_with_nt.emplace_back(antecedent, prod);
            }
        }
    }
    if (rules_with_nt.empty()) {
        output += tr("1. %1 no aparece en ningún consecuente.\n").arg(nt);
        return output;
    }

    output +=
        tr("1. Busca las reglas donde %1 está en el consecuente:\n").arg(nt);
    for (const auto& [antecedent, prod] : rules_with_nt) {
        output += "   - " + QString::fromStdString(antecedent) + " → " +
                  QStringList::fromVector(stdVectorToQVector(prod)).join(" ") +
                  "\n";
    }

    // Step 2: Compute Follow for each occurrence of the non-terminal in the
    // rules
    std::unordered_set<std::string> follow_set;
    for (const auto& [antecedent, prod] : rules_with_nt) {
        for (auto it = prod.begin(); it != prod.end(); ++it) {
            if (*it == nt_str) {
                // Case 1: Non-terminal is not at the end of the production
                if (std::next(it) != prod.end()) {
                    std::vector<std::string> remaining_symbols(std::next(it),
                                                               prod.end());
                    std::unordered_set<std::string> first_of_remaining;
                    ll1.First(remaining_symbols, first_of_remaining);
                    QString rem_qstr =
                        QStringList::fromVector(
                            stdVectorToQVector(remaining_symbols))
                            .join(" ");
                    QString first_qstr =
                        QStringList::fromVector(
                            stdUnorderedSetToQSet(first_of_remaining).values())
                            .join(" ");

                    output += tr("2. Calcula la cabecera de la subcadena "
                                 "después de %1: { %2 } = { "
                                 "%3 }\n")
                                  .arg(nt, rem_qstr, first_qstr);

                    // Add First(remaining_symbols) to Follow(non_terminal)
                    for (const std::string& symbol : first_of_remaining) {
                        if (symbol != ll1.gr_.st_.EPSILON_) {
                            follow_set.insert(symbol);
                        }
                    }

                    // If ε ∈ First(remaining_symbols), add Follow(antecedent)
                    if (first_of_remaining.find(ll1.gr_.st_.EPSILON_) !=
                        first_of_remaining.end()) {
                        std::unordered_set<std::string> ant_follow(
                            ll1.Follow(antecedent));
                        QString ant_follow_str =
                            QStringList::fromVector(
                                stdUnorderedSetToQSet(ant_follow).values())
                                .join(" ");

                        output += tr("   - Como ε ∈ CAB, agrega SIG(%1) = { %2 "
                                     "} a SIG(%3)\n")
                                      .arg(QString::fromStdString(antecedent),
                                           ant_follow_str, nt);
                        follow_set.insert(ant_follow.begin(), ant_follow.end());
                    }
                }
                // Case 2: Non-terminal is at the end of the production
                else {
                    std::unordered_set<std::string> ant_follow(
                        ll1.Follow(antecedent));
                    QString ant_follow_str =
                        QStringList::fromVector(
                            stdUnorderedSetToQSet(ant_follow).values())
                            .join(" ");
                    output += tr("2. %1 está al final de la producción. Agrega "
                                 "SIG(%2) = { %3 } a "
                                 "SIG(%1)\n")
                                  .arg(nt, QString::fromStdString(antecedent),
                                       ant_follow_str);
                    follow_set.insert(ant_follow.begin(), ant_follow.end());
                }
            }
        }
    }

    // Step 3: Display the final Follow set
    QString final_follow =
        QStringList::fromVector(stdUnorderedSetToQSet(follow_set).values())
            .join(" ");
    output += tr("3. Conjunto SIG(%1) = { %2 }\n").arg(nt, final_follow);

    return output;
}

QString LLTutorWindow::TeachPredictionSymbols(const QString&    ant,
                                              const production& consequent) {
    QString output;

    QString consequent_str =
        QStringList::fromVector(stdVectorToQVector(consequent)).join(" ");
    output += tr("Encontrar los símbolos directores de: %1 → %2:\n")
                  .arg(ant, consequent_str);

    // Step 1: Compute First(consequent)
    std::unordered_set<std::string> first_of_consequent;
    ll1.First(consequent, first_of_consequent);

    QString first_str = QStringList::fromVector(
                            stdUnorderedSetToQSet(first_of_consequent).values())
                            .join(" ");
    output +=
        tr("1. Calcula CAB(%1) = { %2 }\n").arg(consequent_str, first_str);

    // Step 2: Initialize prediction symbols with First(consequent) excluding ε
    std::unordered_set<std::string> prediction_symbols;
    for (const std::string& symbol : first_of_consequent) {
        if (symbol != ll1.gr_.st_.EPSILON_) {
            prediction_symbols.insert(symbol);
        }
    }

    QString pred_str = QStringList::fromVector(
                           stdUnorderedSetToQSet(prediction_symbols).values())
                           .join(" ");
    output += tr("2. Inicializa los símbolos directores con CAB(%1) excepto ε: "
                 "{ %2 }\n")
                  .arg(consequent_str, pred_str);

    // Step 3: If ε ∈ First(consequent), add Follow(antecedent) to prediction
    // symbols
    if (first_of_consequent.find(ll1.gr_.st_.EPSILON_) !=
        first_of_consequent.end()) {
        output += tr("   - Como ε ∈ CAB(%1), agrega SIG(%2) a los símbolos "
                     "directores.\n")
                      .arg(consequent_str, ant);

        const auto follow_ant = ll1.Follow(ant.toStdString());
        prediction_symbols.insert(follow_ant.begin(), follow_ant.end());

        QString follow_str =
            QStringList::fromVector(stdUnorderedSetToQSet(follow_ant).values())
                .join(" ");
        output += tr("     SIG(%1) = { %2 }\n").arg(ant, follow_str);
    }

    // Step 4: Display the final prediction symbols
    QString final_str = QStringList::fromVector(
                            stdUnorderedSetToQSet(prediction_symbols).values())
                            .join(" ");
    output +=
        tr("3. Entonces, los símbolos directores de %1 → %2 son: { %3 }\n")
            .arg(ant, consequent_str, final_str);

    return output;
}

QString LLTutorWindow::TeachLL1Table() {
    QString output;
    output += tr("1. Proceso para construir la tabla LL(1):\n");
    output += tr("La tabla LL(1) se construye definiendo todos los símbolos "
                 "directores para cada regla.\n");

    size_t i = 1;
    for (const auto& [nt, prods] : ll1.gr_.g_) {
        for (const production& prod : prods) {
            std::unordered_set<std::string> pred =
                ll1.PredictionSymbols(nt, prod);

            QString prod_str =
                QStringList::fromVector(stdVectorToQVector(prod)).join(" ");
            QString pred_str =
                QStringList::fromVector(stdUnorderedSetToQSet(pred).values())
                    .join(" ");

            output += tr("  %1. SD(%2 → %3) = { %4 }\n")
                          .arg(i++)
                          .arg(QString::fromStdString(nt))
                          .arg(prod_str)
                          .arg(pred_str);
        }
    }

    output += tr(
        "2. Una gramática cumple la condición LL(1) si para cada no terminal, "
        "ninguna de "
        "sus producciones tiene símbolos directores en común.\n"
        "Es decir, para cada regla A → X y A → Y, SD(A → X) ∩ SD(A → Y) = ∅\n");

    bool has_conflicts = false;
    for (const auto& [nt, cols] : ll1.ll1_t_) {
        for (const auto& col : cols) {
            if (col.second.size() > 1) {
                has_conflicts = true;
                output += tr("- Conflicto en %1:\n")
                              .arg(QString::fromStdString(col.first));
                for (const production& prod : col.second) {
                    QString prod_str =
                        QStringList::fromVector(stdVectorToQVector(prod))
                            .join(" ");
                    output += tr("  SD(%1 → %2)\n")
                                  .arg(QString::fromStdString(nt), prod_str);
                }
            }
        }
    }

    if (!has_conflicts) {
        output +=
            tr("3. Los conjuntos de símbolos directores no se solapan. La "
               "gramática es "
               "LL(1). La tabla LL(1) se construye de la siguiente forma.\n");

        output += tr("4. Ten una fila por cada símbolo no terminal (%1 filas), "
                     "y una columna por "
                     "cada terminal excepto epsilon más %2 (%3 columnas).\n")
                      .arg(ll1.gr_.st_.non_terminals_.size())
                      .arg(QString::fromStdString(ll1.gr_.st_.EOL_))
                      .arg(ll1.gr_.st_.terminals_.contains(ll1.gr_.st_.EPSILON_)
                               ? ll1.gr_.st_.terminals_.size() - 1
                               : ll1.gr_.st_.terminals_.size());

        output += tr("5. Coloca α en la celda (A,β) si β ∈ SD(A → α), déjala "
                     "vacía en otro caso.\n");

        for (const auto& [nt, cols] : ll1.ll1_t_) {
            for (const auto& col : cols) {
                QString prod_str = QStringList::fromVector(
                                       stdVectorToQVector(col.second.at(0)))
                                       .join(" ");
                output += tr("  - ll1(%1, %2) = %3\n")
                              .arg(QString::fromStdString(nt))
                              .arg(QString::fromStdString(col.first))
                              .arg(prod_str);
            }
        }
    } else {
        output += tr("3. Como al menos dos conjuntos se solapan con el mismo "
                     "terminal, la "
                     "gramática no es LL(1).\n");
    }

    return output;
}

void LLTutorWindow::setupTutorial() {
    QString key = "S -> A $";
    userSD[key] = solutionForB().values().join(", ");
    userCAB[sortedGrammar.at(currentRule).second.join(' ')] =
        solutionForB1().values().join(", ");
    userSIG[sortedGrammar.at(currentRule).first] =
        solutionForB2().values().join(", ");
    updateProgressPanel();
    ui->userResponse->setDisabled(true);
    ui->confirmButton->setDisabled(true);
    tm->addStep(this->window(), tr("<h3>Tutor LL(1)</h3>"
                                   "<p>Esta es la ventana del tutor de "
                                   "analizadores sintácticos LL(1).</p>"));

    tm->addStep(ui->listWidget,
                tr("<h3>Mensajes</h3>"
                   "<p>Aquí el tutor pregunta y muestra feedback.</p>"
                   "<p>Para enviar tu respuesta pulsa el botón <b>Enviar</b> o "
                   "Enter. Puedes insertar "
                   "una nueva línea con Ctrl+Enter si el formato lo requiere. "
                   "Aunque en el tutor "
                   "LL(1) no es necesario.</p>"));

    tm->addStep(ui->listWidget, tr("<h3>Formato de respuesta</h3>"
                                   "<p>El tutor te indicará el formato de "
                                   "respuesta en cada pregunta. En LL(1), "
                                   "siempre son o listas de símbolos separados "
                                   "por coma o números.</p>"));

    tm->addStep(ui->listWidget,
                tr("<h3>Ejemplo práctico</h3>"
                   "<p>Supón que te piden el conjunto cabecera de una cadena. "
                   "La respuesta correcta "
                   "sería una lista de símbolos, por ejemplo: a,b,c.</p>"
                   "<p>Si te preguntasen el número de símbolos de la "
                   "gramática, bastaría con "
                   "responder con un número.</p>"));

    tm->addStep(
        ui->gr,
        tr("<h3>Gramática</h3>"
           "<p>En esta sección se ve la gramática que estás analizando.</p>"
           "<p>Consulta los símbolos y producciones para responder. Como norma "
           "general, los "
           "símbolos en mayúscula serán los no terminales, los que están en "
           "minúscula, los "
           "terminales, la cadena \"EPSILON\" representará la cadena vacía y $ "
           "representa el "
           "fin de línea.</p>"));

    tm->addStep(ui->textEdit, tr("<h3>Progreso</h3>"
                                 "<p>Aquí se registran los pasos que das: "
                                 "conjuntos cabecera (CAB), siguientes (SIG) y "
                                 "símbolos directores (SD).</p>"));

    tm->addStep(
        ui->cntRight,
        tr("<h3>Respuestas correctas</h3>"
           "<p>Aquí podrás ver el número de respuestas correctas.</p>"));

    tm->addStep(ui->cntWrong,
                tr("<h3>Respuestas incorrectas</h3>"
                   "<p>Y aquí el número de respuestas incorrectas. Si te "
                   "equivocas, verás una breve "
                   "animación en el mensaje.</p>"));

    tm->addStep(this->window(),
                tr("<h3>Finalización</h3>"
                   "<p>Una vez termines el ejercicio entero, podrás exportar "
                   "toda la conversación "
                   "a PDF. En "
                   "ese PDF se incluye la tabla de análisis LL(1).</p>"));

    tm->addStep(nullptr, "");

    connect(tm, &TutorialManager::stepStarted, this, [this](int idx) {
        if (idx == 14) {
            tm->finishLL1();
        }
    });
}
