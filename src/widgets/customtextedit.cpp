/*
 * SyntaxTutor - Interactive Tutorial About Syntax Analyzers
 * Copyright (C) 2025 Jose R. (jose-rzm)
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "customtextedit.h"
#include <QKeyEvent>
#include <QScrollBar>
CustomTextEdit::CustomTextEdit(QWidget* parent) : QTextEdit(parent) {}

void CustomTextEdit::keyPressEvent(QKeyEvent* event) {
    bool isEnter =
        (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter);

    if (isEnter) {
        Qt::KeyboardModifiers mods = event->modifiers();

        if (mods == Qt::NoModifier) {
            emit sendRequested();
            return;
        } else if (mods.testFlag(Qt::ControlModifier) ||
                   mods.testFlag(Qt::ShiftModifier)) {
            insertPlainText("\n");
            emit textChanged();
            verticalScrollBar()->setValue(verticalScrollBar()->maximum());
            return;
        }
    }
    QTextEdit::keyPressEvent(event);
}
