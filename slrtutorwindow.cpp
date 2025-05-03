#include "slrtutorwindow.h"
#include "ui_slrtutorwindow.h"

SLRTutorWindow::SLRTutorWindow(const Grammar& grammar, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SLRTutorWindow)
    , grammar(grammar)
    , slr1(grammar)
{
    slr1.MakeParser();
    slr1.DebugStates();
    slr1.DebugActions();
    ui->setupUi(this);
    ui->gr->setFont(QFont("Courier New", 14));
    ui->gr->setText(FormatGrammar(grammar));
    addMessage(QString("La gramática es:\n" + FormatGrammar(grammar)), false);
    
    currentState = StateSlr::B;
    addMessage(generateQuestion(), false);

    QFont chatFont("Arial", 12);
    ui->listWidget->setFont(chatFont);
    connect(ui->userResponse, &QLineEdit::returnPressed, this, &SLRTutorWindow::on_confirmButton_clicked);

}

SLRTutorWindow::~SLRTutorWindow()
{
    delete ui;
}

void SLRTutorWindow::addMessage(const QString& text, bool isUser) {
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

void SLRTutorWindow::on_confirmButton_clicked()
{
    QString userResponse;
    bool isCorrect;
    userResponse = ui->userResponse->text().trimmed();
    addMessage(userResponse, true);

    isCorrect = verifyResponse(userResponse);
    if (!isCorrect) {
        addMessage(feedback(), false);
    }
    updateState(isCorrect);
    if (currentState == StateSlr::fin) {
        QMessageBox::information(this, "fin", "fin");
        close();
    }
    addMessage(generateQuestion(), false);
    ui->userResponse->clear();
}

/************************************************************
 *                      QUESTIONS                           *
 ************************************************************/

QString SLRTutorWindow::generateQuestion() {
    switch(currentState) {
    case StateSlr::A:
        return "¿Cuál es el estado inicial? Formato: X -> a·b,X -> ·b,X -> EPSILON";
    case StateSlr::A1:
        return "¿Cuál es el axioma de la gramática?";
    case StateSlr::A2:
        return "¿Cuál es el símbolo que sigue al ·?";
    case StateSlr::A3:
        return "En caso de ser no terminal, ¿cualés son las reglas cuyo antecedente es el símbolo que sigue al ·?";
    case StateSlr::A4:
        return "¿Cuál es el cierre del axioma?";
    case StateSlr::A_prime:
        return "Entonces, ¿cuál es el estado inicial?";
    case StateSlr::B:
        return "¿Cuántos estados tiene el analizador ahora?";
    case StateSlr::C: {
        auto currState = std::ranges::find_if(slr1.states_, [&](const state& st)
                { return st.id_ == currentStateId; });
        currentSlrState = *currState;
        return QString("¿Cuántos ítems tiene el estado %1?\n%2")
            .arg(currentStateId)
            .arg(QString::fromStdString(slr1.PrintItems(currentSlrState.items_)));
    }
    case StateSlr::CA:
        return QString("¿Cuáles son los símbolos que aparecen después del ·? Formato: X,Y,Z");
    case StateSlr::CB: {
        QString currentSymbol = followSymbols.at(currentFollowSymbolsIdx);
        if (currentSymbol.toStdString() == slr1.gr_.st_.EPSILON_) {
            return QString("Calcula DELTA(I%1, %2). Deja la entrada vacía si no se genera un estado.")
                    .arg(currentStateId)
                    .arg(currentSymbol);
        } else {
            nextStateId = slr1.transitions_
                             .at(static_cast<unsigned int>(currentStateId))
                             .at(currentSymbol.toStdString());
            return QString("Calcula DELTA(I%1, %2), este será el estado número %3")
                .arg(currentStateId)
                .arg(currentSymbol)
                .arg(nextStateId);
        }
    }
    default:
        return "";
    }
}

/************************************************************
 *                      UPDATE STATE                        *
 ************************************************************/

void SLRTutorWindow::updateState(bool isCorrect) {
    switch(currentState) {
    case StateSlr::A:
        currentState = isCorrect ? StateSlr::B : StateSlr::A1;
        break;
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
    case StateSlr::A_prime:
        currentState = isCorrect ? StateSlr::B : StateSlr::B;
        break;
    case StateSlr::B:
        currentState = isCorrect ? StateSlr::C : StateSlr::C;
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
        break;
    }
    }
}

/************************************************************
 *                      VERIFY RESPONSES                    *
 ************************************************************/

bool SLRTutorWindow::verifyResponse(const QString& userResponse) {
    switch(currentState) {
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
    default:
        return "";
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
    QStringList response = userResponse.split(",");
    QStringList expected = solutionForCA();
    QSet<QString> responseSet(response.begin(), response.end());
    QSet<QString> expectedSet(expected.begin(), expected.end());
    return responseSet == expectedSet;
}

bool SLRTutorWindow::verifyResponseForCB(const QString& userResponse) {
    if (followSymbols.at(currentFollowSymbolsIdx).toStdString() == slr1.gr_.st_.EPSILON_) {
        return userResponse.isEmpty();
    } else {
        const auto response = ingestUserItems(userResponse);
        bool aux = response == solutionForCB();
        return response == solutionForCB();
    }
}

/************************************************************
 *                         SOLUTIONS                        *
 ************************************************************/

std::unordered_set<Lr0Item> SLRTutorWindow::solutionForA() {
    const auto expected = std::ranges::find_if(slr1.states_, [](const state& st) {
        return st.id_ == 0;
    });
    return expected->items_;
}

QString SLRTutorWindow::solutionForA1() {
    return QString::fromStdString(grammar.axiom_);
}

QString SLRTutorWindow::solutionForA2() {
    return QString::fromStdString(grammar.g_.at(grammar.axiom_).at(0).at(0));
}

std::vector<std::pair<std::string, std::vector<std::string>>> SLRTutorWindow::solutionForA3() {
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
    items.emplace(grammar.axiom_, grammar.g_.at(grammar.axiom_).at(0), grammar.st_.EPSILON_, grammar.st_.EOL_);
    slr1.Closure(items);
    return items;
}

unsigned SLRTutorWindow::solutionForB() {
    return currentTotalStates;
}

unsigned SLRTutorWindow::solutionForC() {
    return currentSlrState.items_.size();
}

QStringList SLRTutorWindow::solutionForCA() {
    QSet<QString> following_symbols;
    std::ranges::for_each(currentSlrState.items_, [&following_symbols](const Lr0Item& item) {
        following_symbols.insert(QString::fromStdString(item.NextToDot()));
    });

    // FILL FOLLOW SYMBOLS FOR CB QUESTION
    followSymbols = following_symbols.values();
    currentFollowSymbolsIdx = 0;

    return followSymbols;
}

std::unordered_set<Lr0Item> SLRTutorWindow::solutionForCB() {
    auto st =  std::ranges::find_if(slr1.states_, [this](const state& st) {
        return st.id_ == nextStateId;
        })->items_;
    return st;
}

/************************************************************
 *                         FEEDBACK                         *
 ************************************************************/

QString SLRTutorWindow::feedback() {
    switch(currentState) {
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
    default:
        return "sa liao, no tendria que haber llegado aquí";
    }
}

QString SLRTutorWindow::feedbackForA() {
    return "El estado inicial se construye con el axioma";
}

QString SLRTutorWindow::feedbackForA1() {
    return QString("El axioma de la gramática es %1")
        .arg(QString::fromStdString(grammar.axiom_));
}

QString SLRTutorWindow::feedbackForA2() {
    return QString("El símbolo que sigue al · es %1")
        .arg(QString::fromStdString(grammar.g_.at(grammar.axiom_).at(0).at(0)));
}

QString SLRTutorWindow::feedbackForA3() {
    QString antecedent = QString::fromStdString(grammar.g_.at(grammar.axiom_).at(0).at(0));
    QString result = QString("Las reglas cuyo antecedente es '%1' son:\n").arg(antecedent);

    for (const auto& rule : sortedGrammar) {
        if (rule.first == antecedent) {
            QStringList symbols = rule.second.toList();
            QString ruleStr = QString("%1 → %2").arg(rule.first, symbols.join(" "));
            result += ruleStr + "\n";
        }
    }

    return result;
}

QString SLRTutorWindow::feedbackForA4() {
    Lr0Item init {grammar.axiom_, grammar.g_.at(grammar.axiom_).at(0), grammar.st_.EPSILON_, grammar.st_.EOL_};
    std::unordered_set<Lr0Item> item{init};
    return QString::fromStdString(slr1.TeachClosure(item));
}

QString SLRTutorWindow::feedbackForAPrime() {
    const auto& st = std::ranges::find_if(slr1.states_, [](const state& st) {
        return st.id_ == 0;
    });
    QString items = QString::fromStdString(slr1.PrintItems(st->items_));
    return QString("El estado inicial es el cierre del cierre del axioma:\n%1")
        .arg(items);
}

QString SLRTutorWindow::feedbackForB() {
    if (currentTotalStates == 1) {
        return "Actualmente se ha generado un solo estado";
    } else {
        return QString("Se han generado %1 estados").arg(currentTotalStates);
    }
}

QString SLRTutorWindow::feedbackForC() {
    return QString("Hay %1 ítems en el estado %2")
        .arg(currentSlrState.items_.size())
        .arg(currentStateId);
}

QString SLRTutorWindow::feedbackForCA() {
    QStringList following = solutionForCA();
    return QString("Los símbolos son: %1").arg(following.join(", "));
}

QString SLRTutorWindow::feedbackForCB() {
    return QString::fromStdString(slr1.TeachDeltaFunction(currentSlrState.items_, followSymbols[currentFollowSymbolsIdx].toStdString()));
}


/************************************************************
 *                         HELPERS                          *
 ************************************************************/

// HELPER FUNCTIONS ----------------------------------------------------------------------
std::vector<std::string> SLRTutorWindow::qvectorToStdVector(const QVector<QString>& qvec) {
    std::vector<std::string> result;
    result.reserve(qvec.size());
    for (const auto& qstr : qvec) {
        result.push_back(qstr.toStdString());
    }
    return result;
}

// Convierte std::vector<std::string> a QVector<QString>
QVector<QString> SLRTutorWindow::stdVectorToQVector(const std::vector<std::string>& vec) {
    QVector<QString> result;
    result.reserve(vec.size());
    for (const auto& str : vec) {
        result.push_back(QString::fromStdString(str));
    }
    return result;
}

// Convierte std::unordered_set<std::string> a QSet<QString>
QSet<QString> SLRTutorWindow::stdUnorderedSetToQSet(const std::unordered_set<std::string>& uset) {
    QSet<QString> result;
    for (const auto& str : uset) {
        result.insert(QString::fromStdString(str));
    }
    return result;
}

// Convierte QSet<QString> a std::unordered_set<std::string>
std::unordered_set<std::string> SLRTutorWindow::qsetToStdUnorderedSet(const QSet<QString>& qset) {
    std::unordered_set<std::string> result;
    for (const auto& qstr : qset) {
        result.insert(qstr.toStdString());
    }
    return result;
}

std::unordered_set<Lr0Item> SLRTutorWindow::ingestUserItems(const QString& userResponse) {
    std::stringstream ss(userResponse.toStdString());
    char del = ',';
    std::string token;
    std::unordered_set<Lr0Item> items;

    while (std::getline(ss, token, del)) {
        size_t arrowpos = token.find("->");
        if (arrowpos == std::string::npos) {
            return {};
        }
        std::string antecedent = token.substr(0, arrowpos);
        std::string consequent = token.substr(arrowpos + 2);

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
        items.emplace(antecedent, splitted, static_cast<unsigned int>(dot_idx), grammar.st_.EPSILON_, grammar.st_.EOL_);
    }
    return items;
}

std::vector<std::pair<std::string, std::vector<std::string>>> SLRTutorWindow::ingestUserRules(const QString& userResponse) {
    std::stringstream ss(userResponse.toStdString());
    char del = ',';
    std::string token;
    std::vector<std::pair<std::string, std::vector<std::string>>> rules;

    while (std::getline(ss, token, del)) {
        size_t arrowpos = token.find("->");
        if (arrowpos == std::string::npos) {
            return {};
        }
        std::string antecedent = token.substr(0, arrowpos);
        std::string consequent = token.substr(arrowpos + 2);

        std::vector<std::string> splitted{grammar.Split(consequent)};

        rules.emplace_back(antecedent, splitted);
    }
    return rules;
}

QString SLRTutorWindow::FormatGrammar(const Grammar& grammar) {
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
