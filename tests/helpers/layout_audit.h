#pragma once

#include <QAbstractButton>
#include <QAbstractScrollArea>
#include <QAbstractSlider>
#include <QLabel>
#include <QLineEdit>
#include <QRegion>
#include <QString>
#include <QStringList>
#include <QTextEdit>
#include <QWidget>

namespace LayoutAudit {

namespace detail {

// A pixel of slack absorbs rounding between the size a widget asks for and
// the one a layout hands out; anything beyond that is text that no longer
// fits.
constexpr int kTolerance = 1;

// Widgets inside a scroll area's viewport are allowed to extend past what
// is visible: that is what scrolling is for.
inline bool insideScrollViewport(const QWidget* widget, const QWidget* root) {
    for (const QWidget* w = widget->parentWidget(); w != nullptr && w != root;
         w = w->parentWidget()) {
        const auto* area =
            qobject_cast<const QAbstractScrollArea*>(w->parentWidget());
        if (area != nullptr && area->viewport() == w) {
            return true;
        }
    }
    return false;
}

inline QString describe(const QWidget* widget) {
    QString text;
    if (const auto* label = qobject_cast<const QLabel*>(widget)) {
        text = label->text();
    } else if (const auto* button =
                   qobject_cast<const QAbstractButton*>(widget)) {
        text = button->text();
    }
    text = text.simplified();
    if (text.size() > 40) {
        text = text.left(37) + QStringLiteral("...");
    }
    return QStringLiteral("%1#%2 \"%3\"")
        .arg(QString::fromLatin1(widget->metaObject()->className()),
             widget->objectName(), text);
}

inline bool shorterThan(int actual, int needed) {
    return actual + kTolerance < needed;
}

} // namespace detail

/**
 * @brief Lists every visible widget under @p root whose content is cut.
 *
 * Checks what a user would see as clipped text: a word-wrapped label given
 * less height than its text needs at its actual width, a label, button,
 * slider or line edit squeezed below its natural size, and a widget cut off
 * by an ancestor. Content inside scroll areas may overflow.
 */
inline QStringList findClippedWidgets(QWidget* root) {
    using namespace detail;
    QStringList problems;
    auto report = [&problems](const QWidget* w, const QString& why) {
        problems << QStringLiteral("%1: %2").arg(describe(w), why);
    };

    QList<QWidget*> widgets = root->findChildren<QWidget*>();
    widgets.prepend(root);
    for (QWidget* w : widgets) {
        if (!w->isVisible() || w->width() <= 0 || w->height() <= 0) {
            continue;
        }
        const bool scrolls = insideScrollViewport(w, root);

        if (auto* label = qobject_cast<QLabel*>(w)) {
            if (label->text().isEmpty() && label->pixmap().isNull()) {
                continue;
            }
            if (label->wordWrap()) {
                const int needed = label->heightForWidth(label->width());
                if (shorterThan(label->height(), needed)) {
                    report(w, QStringLiteral("height %1 < %2 for width %3")
                                  .arg(label->height())
                                  .arg(needed)
                                  .arg(label->width()));
                }
            } else {
                const QSize hint = label->minimumSizeHint();
                if (shorterThan(label->width(), hint.width()) ||
                    shorterThan(label->height(), hint.height())) {
                    report(w, QStringLiteral("size %1x%2 < %3x%4")
                                  .arg(label->width())
                                  .arg(label->height())
                                  .arg(hint.width())
                                  .arg(hint.height()));
                }
            }
        } else if (qobject_cast<QAbstractButton*>(w) != nullptr ||
                   qobject_cast<QAbstractSlider*>(w) != nullptr ||
                   qobject_cast<QLineEdit*>(w) != nullptr) {
            // Scroll bars size themselves to whatever they are given.
            if (w->inherits("QScrollBar")) {
                continue;
            }
            const QSize hint = w->minimumSizeHint();
            if (shorterThan(w->width(), hint.width()) ||
                shorterThan(w->height(), hint.height())) {
                report(w, QStringLiteral("size %1x%2 < %3x%4")
                              .arg(w->width())
                              .arg(w->height())
                              .arg(hint.width())
                              .arg(hint.height()));
            }
        }

        if (!scrolls && w != root && w->isWindow() == false) {
            const QRect shown = w->visibleRegion().boundingRect();
            if (!shown.isEmpty() && shown != w->rect() &&
                (shorterThan(shown.width(), w->width()) ||
                 shorterThan(shown.height(), w->height()))) {
                report(w, QStringLiteral("cut by an ancestor: %1x%2 of %3x%4")
                              .arg(shown.width())
                              .arg(shown.height())
                              .arg(w->width())
                              .arg(w->height()));
            }
        }
    }
    return problems;
}

} // namespace LayoutAudit
