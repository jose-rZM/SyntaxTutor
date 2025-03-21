#include "lltutorwindow.h"
#include "ui_lltutorwindow.h"

#include <QRandomGenerator>
#include <iostream>
LLTutorWindow::LLTutorWindow(const Grammar& grammar, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LLTutorWindow)
    , grammar(grammar)
    , ll1(grammar)
{
    ll1.CreateLL1Table();
    ll1.PrintTable();
    for (const auto& symbol : grammar.st_.terminals_) {
        std::cout << symbol << " ";
    }
    std::cout << std::endl;
    ui->setupUi(this);
    ui->gr->setFont(QFont("Courier New", 14));
    ui->gr->setText(FormatGrammar(grammar));
    addMessage(QString("La gramática es:\n" + FormatGrammar(grammar)), false);

    currentState = State::A;
    addMessage(generateQuestion(), false);
    ui->userResponse->clear();

    QFont chatFont("Arial", 12);
    ui->listWidget->setFont(chatFont);
    connect(ui->userResponse, &QLineEdit::returnPressed, this, &LLTutorWindow::on_confirmButton_clicked);

}

LLTutorWindow::~LLTutorWindow()
{
    delete ui;
}

void LLTutorWindow::addMessage(const QString& text, bool isUser) {
    QWidget* messageWidget = new QWidget;
    QVBoxLayout* mainLayout = new QVBoxLayout(messageWidget);

    QLabel* header = new QLabel(isUser ? "Usuario" : "Tutor");
    header->setAlignment(isUser ? Qt::AlignRight : Qt::AlignLeft);
    header->setStyleSheet(isUser ? "font-weight: bold; color: #00ADB5; font-size: 12px;"
                                 : "font-weight: bold; color: #8E8E93; font-size: 12px;");

    QHBoxLayout* messageLayout = new QHBoxLayout;
    QVBoxLayout* innerLayout = new QVBoxLayout;

    QLabel* label = new QLabel(text);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label->setMaximumWidth(ui->listWidget->width());
    label->setMinimumWidth(100);

    QLabel* timestamp = new QLabel(QTime::currentTime().toString("HH:mm"));
    timestamp->setStyleSheet("font-size: 10px; color: gray; margin-left: 5px;");
    timestamp->setAlignment(Qt::AlignRight);

    int maxWidth = ui->listWidget->width() * 0.8;
    label->setMaximumWidth(maxWidth);
    label->setMinimumWidth(200);

    if (isUser) {
        label->setStyleSheet(R"(
        background-color: #00ADB5;  /* Azul tipo iMessage */
        color: white;
        padding: 12px 16px;
        border-radius: 18px;
        font-size: 14px;
    )");
    } else {
        label->setStyleSheet(R"(
        background-color: #222831;  /* Gris oscuro */
        color: #E0E0E0;  /* Gris claro */
        padding: 12px 16px;
        border-radius: 18px;
        font-size: 14px;
    )");
    }

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
    mainLayout->setContentsMargins(10, 5, 10, 5);
    messageWidget->setLayout(mainLayout);

    QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(messageWidget->sizeHint());

    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, messageWidget);

    ui->listWidget->update();
    ui->listWidget->verticalScrollBar()->setSingleStep(5);
    ui->listWidget->scrollToBottom();

}
void LLTutorWindow::on_confirmButton_clicked()
{
    QString userResponse;
    bool isCorrect;
    if (currentState != State::C) {
        userResponse = ui->userResponse->text().trimmed();
        addMessage(userResponse, true);
        isCorrect = verifyResponse(userResponse);
    } else {
        isCorrect = verifyResponseForC();
    }


    if (!isCorrect) {
        addMessage(feedback(), false);
    }
    updateState(isCorrect);

    if (currentState == State::fin) {
        QMessageBox::information(this, "fin", "fin");
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

    for (const auto& [nt, col] : ll1.ll1_t_) {
        for (const auto & [terminal, prods] : col) {
            if (!lltable[QString::fromStdString(nt)][QString::fromStdString(terminal)].contains(stdVectorToQVector(prods[0]))) {
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
        llTable();
        ui->userResponse->setDisabled(true);
        return "Rellena la tabla LL(1). Cuando acabes dale a confirmar en esta ventana.";
        break;
    default:
        return "";
    }
}

void LLTutorWindow::llTable() {
    QSet<QString> terminals = stdUnorderedSetToQSet(grammar.st_.terminals_);
    QSet<QString> nonterminals = stdUnorderedSetToQSet(grammar.st_.non_terminals_);

    QStringList hd_terminals (terminals.begin(), terminals.end());
    QStringList hd_non_terminals(nonterminals.begin(), nonterminals.end());

    int rows = hd_non_terminals.size();
    int cols = hd_terminals.size();
    DialogTableLL dialog(hd_non_terminals, hd_terminals, rows, cols, this);
    if (dialog.exec() == QDialog::Accepted) {
        QTableWidget *tabla = dialog.getTableWidget();
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                QTableWidgetItem *item = tabla->item(i, j);
                QString cell = item ? item->text() : "";
                if (cell != "") {
                    lltable[hd_non_terminals[i]][hd_terminals[j]].push_back(cell.split(" "));
                }
            }
        }
    }
}

QString LLTutorWindow::FormatGrammar(const Grammar& grammar) {
    QString result;

    result += QString::fromStdString(grammar.axiom_) + "-> ";

    auto it = grammar.g_.find(grammar.axiom_);
    QVector<QPair<QString, QVector<QString>>> rules;
    QPair<QString, QVector<QString>> rule({QString::fromStdString(grammar.axiom_), {}});
    if (it != grammar.g_.end()) {
        for (const auto& prod : it->second) {
            for (const auto& symbol : prod) {
                rule.second.push_back(QString::fromStdString(symbol));
                result += QString::fromStdString(symbol) + " ";
            }
            result += "| ";
        }
        result.chop(3);
    }
    rules.push_back(rule);
    result += "\n";
    std::map<std::string, std::vector<production>> sortedRules(grammar.g_.begin(), grammar.g_.end());
    for (const auto& [lhs, productions] : sortedRules) {
        if (lhs == grammar.axiom_) continue;
        rule = {QString::fromStdString(lhs), {}};
        result += QString::fromStdString(lhs) + " -> ";
        for (const auto& prod : productions) {
            for (const auto& symbol : prod) {
                rule.second.push_back(QString::fromStdString(symbol));
                result += QString::fromStdString(symbol) + " ";
            }
            rules.push_back(rule);
            rule = {QString::fromStdString(lhs), {}};
            result += "| ";
        }
        result.chop(3);
        result += "\n";
    }
    sortedGrammar = rules;
    return result;
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
