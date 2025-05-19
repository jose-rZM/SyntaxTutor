#include "tutorialmanager.h"
#include <QVBoxLayout>

TutorialManager::TutorialManager(QWidget *rootWindow)
    : QObject(rootWindow)
    , m_root(rootWindow)
{
    m_root->installEventFilter(this);
}

void TutorialManager::finishLL1()
{
    emit ll1Finished();
}

void TutorialManager::finishSLR1()
{
    emit slr1Finished();
}

bool TutorialManager::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == m_root && ev->type() == QEvent::Resize) {
        repositionOverlay();
        return false;
    }
    return QObject::eventFilter(obj, ev);
}

void TutorialManager::clearSteps()
{
    if (m_overlay)
        hideOverlay();
    m_steps.clear();
    m_index = -1;
}

void TutorialManager::setRootWindow(QWidget *newRoot)
{
    if (m_overlay) {
        hideOverlay();
    }
    m_root->removeEventFilter(this);
    m_root = newRoot;
    m_root->installEventFilter(this);
}

void TutorialManager::repositionOverlay()
{
    if (!m_overlay)
        return;

    m_overlay->setGeometry(m_root->rect());

    const auto &step = m_steps[m_index];
    if (step.target) {
        QPoint topLeft = step.target->mapTo(m_root, QPoint(0, 0));
        QRect r(topLeft, step.target->size());
        m_highlight->setGeometry(r.adjusted(-6, -6, 6, 6));
    }

    int w = m_root->width() * 0.6;
    int h = 180;
    int x = 20;
    int y = m_root->height() - h - 20;
    m_textBox->setGeometry(x, y, w, h);

    int bx = m_root->width() - m_nextBtn->width() - 20;
    int by = m_root->height() - m_nextBtn->height() - 20;
    m_nextBtn->move(bx, by);
}

void TutorialManager::addStep(QWidget *target, const QString &htmlText)
{
    m_steps.append({target, htmlText});
}

void TutorialManager::start()
{
    m_index = -1;
    nextStep();
}

void TutorialManager::nextStep()
{
    hideOverlay();

    ++m_index;

    if (m_index >= m_steps.size()) {
        emit tutorialFinished();
        return;
    }

    emit stepStarted(m_index);

    showOverlay();
}

#include <QScrollBar>
void TutorialManager::showOverlay()
{
    if (m_overlay) {
        hideOverlay();
    }

    m_overlay = new QWidget(m_root);
    m_overlay->setStyleSheet("background:rgba(0,0,0,0.6);");
    m_overlay->show();

    m_highlight = new QFrame(m_overlay);
    m_highlight->setStyleSheet("border:2px solid #00ADB5; background:transparent;");
    m_highlight->show();

    m_textBox = new QTextBrowser(m_overlay);
    m_textBox->setStyleSheet(R"(
        background: qlineargradient(
        x1:0, y1:0, x2:0, y2:1,
        stop:0 #3b3b3b, stop:1 #2e2e2e
        );
        color:#EEEEEE;
        padding:12px;
        border-radius:8px;
    )");
    m_textBox->verticalScrollBar()->setStyleSheet(R"(
    QScrollBar:vertical {
        background: #2E2E2E;
        width: 12px;
        margin: 0px;
        border-radius: 6px;
    }
    QScrollBar::handle:vertical {
        background: #555555;
        min-height: 20px;
        border-radius: 6px;
    }
    QScrollBar::handle:vertical:hover {
        background: #777777;
    }
    QScrollBar::add-line, QScrollBar::sub-line {
        height: 0px;
    }
    QScrollBar::add-page, QScrollBar::sub-page {
        background: none;
    }
)");
    m_textBox->setHtml(m_steps[m_index].htmlText);
    m_textBox->show();

    m_nextBtn = new QPushButton("&Siguiente", m_overlay);
    m_nextBtn->setCursor(Qt::PointingHandCursor);
    m_nextBtn->setStyleSheet(R"(
    QPushButton {
        background-color: #00ADB5;
        color: #FFFFFF;
        border: none;
        padding: 10px 20px;
        border-radius: 8px;
        font-size: 14px;
        font-family: 'Noto Sans';
        font-weight: bold;
    }
    QPushButton:hover {
        background-color: #00CED1;
    }
    QPushButton:pressed {
        background-color: #007F86;
    }
    )");
    connect(m_nextBtn, &QPushButton::clicked, this, &TutorialManager::nextStep);
    m_nextBtn->show();

    repositionOverlay();
}

void TutorialManager::hideOverlay()
{
    delete m_overlay;
    m_overlay = nullptr;
    m_highlight = nullptr;
    m_textBox = nullptr;
    m_nextBtn = nullptr;
}
