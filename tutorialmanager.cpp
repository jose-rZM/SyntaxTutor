#include "tutorialmanager.h"
#include <QVBoxLayout>

TutorialManager::TutorialManager(QWidget* rootWindow)
    : QObject(rootWindow), m_root(rootWindow)
{
    m_root->installEventFilter(this);
}

bool TutorialManager::eventFilter(QObject* obj, QEvent* ev) {
    if (obj == m_root && ev->type() == QEvent::Resize) {
        // la ventana principal cambió de tamaño: reposicionamos todo
        repositionOverlay();
        return false; // que siga procesando el resize normal
    }
    return QObject::eventFilter(obj, ev);
}

// en tutorialmanager.cpp
void TutorialManager::setRootWindow(QWidget* newRoot) {
    // quitas el filtro del antiguo
    m_root->removeEventFilter(this);
    m_root = newRoot;
    m_root->installEventFilter(this);
    // ocultas cualquier overlay activo antes de seguir
    hideOverlay();
}


void TutorialManager::repositionOverlay() {
    if (!m_overlay) return;

    // 1) overlay siempre cubre toda la ventana
    m_overlay->setGeometry(m_root->rect());

    // 2) reposicionamos el highlight
    const auto &step = m_steps[m_index];
    if (step.target) {
        QPoint topLeft = step.target->mapTo(m_root, QPoint(0,0));
        QRect r(topLeft, step.target->size());
        m_highlight->setGeometry(r.adjusted(-6,-6,6,6));
    }

    // 3) reposicionamos el textbox:
    //    margen 20px de los lados, altura fija, ancho al 60% del root
    int w = m_root->width() * 0.6;
    int h = 180;
    int x = 20;
    int y = m_root->height() - h - 20;
    m_textBox->setGeometry(x, y, w, h);

    // 4) reposicionamos el botón “Siguiente”
    m_nextBtn->move(m_root->width() - 100, m_root->height() - 50);
}


void TutorialManager::addStep(QWidget* target, const QString& htmlText) {
    m_steps.append({target, htmlText});
}

void TutorialManager::start() {
    m_index = -1;
    nextStep();
}

void TutorialManager::nextStep() {
    // escondemos overlay anterior
    hideOverlay();

    // avanzamos índice
    ++m_index;

    if (m_index >= m_steps.size()) {
        emit tutorialFinished();
        return;
    }

    // notificamos que arrancamos el paso m_index
    emit stepStarted(m_index);

    // mostramos el overlay para este paso
    showOverlay();
}

void TutorialManager::showOverlay() {
    // 1) crea overlay, highlight, textbox, nextBtn…
    m_overlay = new QWidget(m_root);
    m_overlay->setStyleSheet("background:rgba(0,0,0,0.6);");
    m_overlay->show(); // todavía sin geometry buena

    m_highlight = new QFrame(m_overlay);
    m_highlight->setStyleSheet("border:2px solid #00ADB5; background:transparent;");
    m_highlight->show();

    m_textBox = new QTextBrowser(m_overlay);
    m_textBox->setStyleSheet(R"(
        background:#333333;
        color:#EEEEEE;
        padding:12px;
        border-radius:8px;
    )");
    m_textBox->setHtml(m_steps[m_index].htmlText);
    m_textBox->show();

    m_nextBtn = new QPushButton("Siguiente", m_overlay);
    m_nextBtn->setCursor(Qt::PointingHandCursor);
    connect(m_nextBtn, &QPushButton::clicked, this, &TutorialManager::nextStep);
    m_nextBtn->show();

    // 2) y luego lo posicionamos bien:
    repositionOverlay();
}

void TutorialManager::hideOverlay() {
    delete m_overlay;
    m_overlay = nullptr;
    m_highlight = nullptr;
    m_textBox = nullptr;
    m_nextBtn = nullptr;
}
