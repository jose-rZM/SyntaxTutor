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
#include <QAbstractTextDocumentLayout>
#include <QKeyEvent>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QtMath>
CustomTextEdit::CustomTextEdit(QWidget* parent) : QTextEdit(parent) {}

void CustomTextEdit::setAutoGrow(int minHeight, int maxHeight) {
    minGrowHeight_ = minHeight;
    maxGrowHeight_ = maxHeight;
    if (growAnimation_ == nullptr) {
        growAnimation_ = new QPropertyAnimation(this, "growHeight", this);
        growAnimation_->setDuration(180);
        QEasingCurve curve(QEasingCurve::OutBack);
        curve.setOvershoot(1.2);
        growAnimation_->setEasingCurve(curve);
        // documentSizeChanged also fires on re-wraps caused by width
        // changes, which textChanged would miss.
        connect(document()->documentLayout(),
                &QAbstractTextDocumentLayout::documentSizeChanged, this,
                [this](const QSizeF&) { updateGrowHeight(); });
    }
    setFixedHeight(minGrowHeight_);
    updateGrowHeight();
}

int CustomTextEdit::growHeight() const {
    return height();
}

void CustomTextEdit::setGrowHeight(int height) {
    // Clamp so the easing overshoot never leaves the allowed range.
    setFixedHeight(qBound(minGrowHeight_, height, maxGrowHeight_));
}

void CustomTextEdit::updateGrowHeight() {
    if (maxGrowHeight_ <= 0) {
        return;
    }
    // Frame and QSS padding live outside the viewport; measuring them live
    // keeps the target exact, so typing on an existing line stays stable.
    const int chrome = height() - viewport()->height();
    const int contentHeight = qCeil(document()->size().height()) + chrome;
    const int target = qBound(minGrowHeight_, contentHeight, maxGrowHeight_);

    // A scrollbar below full height would shrink the viewport, re-wrap the
    // text and feed back into this handler, flickering. Only allow it once
    // the box cannot grow any further.
    const Qt::ScrollBarPolicy policy = (target >= maxGrowHeight_)
                                           ? Qt::ScrollBarAsNeeded
                                           : Qt::ScrollBarAlwaysOff;
    if (verticalScrollBarPolicy() != policy) {
        setVerticalScrollBarPolicy(policy);
    }

    if (growAnimation_->state() == QAbstractAnimation::Running) {
        if (growAnimation_->endValue().toInt() == target) {
            return;
        }
        growAnimation_->stop();
    } else if (qAbs(target - height()) < 3) {
        // Dead band: sub-pixel document jitter must not retrigger the
        // animation; real line changes move by a full line height.
        return;
    }

    growAnimation_->setStartValue(height());
    growAnimation_->setEndValue(target);
    growAnimation_->start();
}

QString CustomTextEdit::newlineShortcutText() {
#ifdef Q_OS_MACOS
    return QStringLiteral("Cmd + Enter");
#else
    return QStringLiteral("Ctrl + Enter");
#endif
}

void CustomTextEdit::keyPressEvent(QKeyEvent* event) {
    bool isEnter =
        (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter);

    if (isEnter) {
        // Keypad Enter carries KeypadModifier; it must still send.
        Qt::KeyboardModifiers mods = event->modifiers() & ~Qt::KeypadModifier;

        if (mods == Qt::NoModifier) {
            emit sendRequested();
            return;
        } else if (mods.testFlag(Qt::ControlModifier) ||
                   mods.testFlag(Qt::ShiftModifier) ||
                   // On macOS Qt maps the physical Ctrl key to MetaModifier.
                   mods.testFlag(Qt::MetaModifier)) {
            insertPlainText("\n");
            emit textChanged();
            verticalScrollBar()->setValue(verticalScrollBar()->maximum());
            return;
        }
    }
    QTextEdit::keyPressEvent(event);
}
