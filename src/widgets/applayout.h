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

#ifndef APPLAYOUT_H
#define APPLAYOUT_H

#include <QEvent>
#include <QLayout>
#include <QObject>
#include <QWidget>

/**
 * @brief Layout helpers that keep text from being clipped.
 *
 * Qt sizes word-wrapped text by height-for-width, but only some containers
 * honour it. Top-level windows and widgets placed directly in a
 * QMainWindow or a QStackedWidget are given the height their layout guesses
 * for an arbitrary width, so a title that wraps to one more line than the
 * guess is cut in half. The effect depends on font, text size and window
 * width, which is why it only shows on some systems.
 */
namespace AppLayout {

/// @brief Event filter behind @ref keepHeightForWidth.
class HeightForWidthKeeper : public QObject {
  public:
    explicit HeightForWidthKeeper(QWidget* widget)
        : QObject(widget), widget_(widget) {
        widget_->installEventFilter(this);
    }

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (watched == widget_ && (event->type() == QEvent::Resize ||
                                   event->type() == QEvent::LayoutRequest ||
                                   event->type() == QEvent::Show)) {
            update();
        }
        return false;
    }

  private:
    void update() {
        QLayout* layout = widget_->layout();
        if (layout == nullptr || !layout->hasHeightForWidth()) {
            return;
        }
        // The minimum, not the preferred height: the preferred one also
        // counts what spacers would like to have, which over-constrains.
        const int needed =
            layout->totalMinimumHeightForWidth(widget_->width());
        if (needed <= 0) {
            return;
        }
        if (widget_->minimumHeight() != needed) {
            widget_->setMinimumHeight(needed);
        }
        // A window is not resized by its minimum alone once it is shown.
        if (widget_->isWindow() && widget_->height() < needed) {
            widget_->resize(widget_->width(), needed);
        }
    }

    QWidget* widget_;
};

/**
 * @brief Keeps @p widget at least as tall as its layout needs at its
 * current width.
 *
 * Re-evaluated whenever the widget is resized or its layout changes, so it
 * follows the window width, the language and the text size. Use it on
 * windows and on containers whose parent ignores height-for-width.
 */
inline void keepHeightForWidth(QWidget* widget) {
    new HeightForWidthKeeper(widget);
}

} // namespace AppLayout

#endif // APPLAYOUT_H
