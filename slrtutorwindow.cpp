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

    currentState = StateSlr::A;
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

}

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
    default:
        return "";
    }
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
