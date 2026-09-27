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
#include <QEvent>
#include <QKeyEvent>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QTextDocument>
#include <QtMath>
CustomTextEdit::CustomTextEdit(QWidget* parent) : QTextEdit(parent) {}

void CustomTextEdit::setAutoGrowLines(int minLines, int maxLines) {
    minGrowLines_ = minLines;
    maxGrowLines_ = maxLines;
    if (growAnimation_ == nullptr) {
        growAnimation_ = new QPropertyAnimation(this, "growHeight", this);
        growAnimation_->setDuration(180);
        // No overshoot: this box usually grows by a single line, and an
        // easing that springs past its target and settles back reads as the
        // text bouncing up and down while you type.
        growAnimation_->setEasingCurve(QEasingCurve::OutCubic);
        // documentSizeChanged catches re-wraps caused by width changes,
        // which textChanged would miss.
        connect(document()->documentLayout(),
                &QAbstractTextDocumentLayout::documentSizeChanged, this,
                [this](const QSizeF&) { updateGrowHeight(); });
        connect(this, &QTextEdit::textChanged, this,
                [this]() { updateGrowHeight(); });
    }
    refreshGrowBounds();
    setFixedHeight(minGrowHeight_);
    updateGrowHeight();
}

int CustomTextEdit::heightForLines(int lines) const {
    // Laid out by a document like ours rather than estimated from font
    // metrics: the metrics are off by a pixel at some sizes, which is
    // enough to leave the one-line box and the send button misaligned.
    QTextDocument probe;
    probe.setDefaultFont(document()->defaultFont());
    probe.setDocumentMargin(document()->documentMargin());
    probe.setPlainText(
        QStringList(lines, QStringLiteral("x")).join(QChar(u'\n')));
    return qCeil(probe.size().height()) + chrome_;
}

int CustomTextEdit::growHeight() const {
    return height();
}

void CustomTextEdit::setGrowHeight(int height) {
    // Clamp so the easing overshoot never leaves the allowed range.
    setFixedHeight(qBound(minGrowHeight_, height, maxGrowHeight_));
}

void CustomTextEdit::refreshGrowBounds() {
    // Everything of the widget's height that is not the viewport: the frame
    // plus the style padding. Only re-sampled while no animation is in
    // flight, because mid-resize the viewport lags behind height() and the
    // reading would be off by exactly what is still being animated.
    if (growAnimation_->state() != QAbstractAnimation::Running) {
        const int measured = height() - viewport()->height();
        if (measured > 0) {
            chrome_ = measured;
        }
    }

    // The bounds follow the font and the chrome, so they are re-derived on
    // every pass; both only change on a style or text size change.
    maxGrowHeight_    = heightForLines(maxGrowLines_);
    const int minimum = heightForLines(minGrowLines_);
    if (minimum != minGrowHeight_) {
        minGrowHeight_ = minimum;
        emit minimumGrowHeightChanged(minGrowHeight_);
    }
}

void CustomTextEdit::updateGrowHeight() {
    if (maxGrowLines_ <= 0) {
        return;
    }
    refreshGrowBounds();

    const int contentHeight = qCeil(document()->size().height()) + chrome_;
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
    } else if (target == height()) {
        return;
    }

    // A correction of a pixel or two is not a gesture worth animating, and
    // there used to be a dead band that skipped it instead: the height then
    // stayed wrong until a later edit pushed the difference past the band,
    // which is what made the box appear to settle in two steps. Snap small
    // corrections, animate only real line changes.
    if (qAbs(target - height()) < qMax(2, fontMetrics().height() / 2)) {
        setGrowHeight(target);
        return;
    }

    growAnimation_->setStartValue(height());
    growAnimation_->setEndValue(target);
    growAnimation_->start();
}

void CustomTextEdit::showEvent(QShowEvent* event) {
    QTextEdit::showEvent(event);
    // setAutoGrow is called from the tutor constructors, before the widget
    // has been laid out or styled, so the first measurement can be taken
    // against a viewport and font that are not final yet.
    updateGrowHeight();
}

void CustomTextEdit::changeEvent(QEvent* event) {
    QTextEdit::changeEvent(event);
    if (event->type() == QEvent::FontChange ||
        event->type() == QEvent::StyleChange) {
        updateGrowHeight();
    }
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
