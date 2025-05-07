#include "customtextedit.h"
#include <QKeyEvent>

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
            return;
        }
    }
    QTextEdit::keyPressEvent(event);
}
