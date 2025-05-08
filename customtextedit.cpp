#include "customtextedit.h"
#include <QKeyEvent>
#include <QScrollBar>
CustomTextEdit::CustomTextEdit(QWidget *parent)
    : QTextEdit(parent)
{}

void CustomTextEdit::keyPressEvent(QKeyEvent *event)
{
    bool isEnter = (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter);

    if (isEnter) {
        Qt::KeyboardModifiers mods = event->modifiers();

        if (mods == Qt::NoModifier) {
            emit sendRequested();
            return;
        } else if (mods.testFlag(Qt::ControlModifier) || mods.testFlag(Qt::ShiftModifier)) {
            insertPlainText("\n");
            emit textChanged();
            verticalScrollBar()->setValue(verticalScrollBar()->maximum());
            return;
        }
    }
    QTextEdit::keyPressEvent(event);
}
