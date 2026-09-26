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

#ifndef APPTEXTSCALE_H
#define APPTEXTSCALE_H

#include <QFont>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <qmath.h>

/**
 * @brief User-selectable text size for the whole application.
 *
 *
 * The scale is a plain multiplier applied in two places: @ref scaleStyleSheet
 * rewrites the sizes in the style sheet, and @ref scaled is used by the few
 * fonts that are built in C++ instead of QSS.
 */
namespace AppTextScale {

/// @brief Sizes offered to the user.
enum class Step { Normal, Large, Larger };

/// @brief Multiplier for a step.
inline double factorFor(Step step) {
    switch (step) {
    case Step::Large:
        return 1.15;
    case Step::Larger:
        return 1.30;
    case Step::Normal:
        break;
    }
    return 1.0;
}

/**
 * @brief The selected step.
 *
 * An inline function with a static local so every translation unit shares
 * one instance without needing a .cpp.
 */
inline Step& currentStep() {
    static Step step = Step::Normal;
    return step;
}

/// @brief Multiplier currently in effect.
inline double factor() { return factorFor(currentStep()); }

/// @brief Scales a size given in the design's logical pixels.
inline int scaled(int px) { return qRound(px * factor()); }

/**
 * @brief Rewrites every `font-size: Npx` in a style sheet by @ref factor.
 *
 * Only `px` is touched. Sizes in `pt` belong to the PDF exports, which
 * target paper and must not follow a screen preference.
 */
inline QString scaleStyleSheet(const QString& styleSheet) {
    const double f = factor();
    if (qFuzzyCompare(f, 1.0)) {
        return styleSheet;
    }

    static const QRegularExpression sizeRe(
        QStringLiteral("font-size:\\s*([0-9]+(?:\\.[0-9]+)?)px"));

    QString  out;
    qsizetype last = 0;
    auto     it    = sizeRe.globalMatch(styleSheet);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        out += styleSheet.mid(last, match.capturedStart() - last);
        out += QStringLiteral("font-size: %1px")
                   .arg(qRound(match.captured(1).toDouble() * f));
        last = match.capturedEnd();
    }
    out += styleSheet.mid(last);
    return out;
}

/**
 * @brief The application font as it was before any scaling.
 *
 * Captured once at startup. Text that is not covered by a `font-size` rule
 * in app.qss - the chat bubbles, for one - uses this font, so the scale has
 * to reach it as well. Scaling always starts from this base so repeated
 * changes do not compound.
 */
inline QFont& baseApplicationFont() {
    static QFont font;
    return font;
}

/// @brief The application font with the current factor applied.
inline QFont scaledApplicationFont() {
    QFont font = baseApplicationFont();
    if (font.pixelSize() > 0) {
        font.setPixelSize(scaled(font.pixelSize()));
    } else if (font.pointSizeF() > 0.0) {
        font.setPointSizeF(font.pointSizeF() * factor());
    }
    return font;
}

/// @brief Settings key holding the selected step.
inline QString settingsKey() { return QStringLiteral("ui/textScale"); }

/// @brief Step as stored in QSettings.
inline QString toSettingsValue(Step step) {
    switch (step) {
    case Step::Large:
        return QStringLiteral("large");
    case Step::Larger:
        return QStringLiteral("larger");
    case Step::Normal:
        break;
    }
    return QStringLiteral("normal");
}

/// @brief Step from a stored value, falling back to Normal.
inline Step fromSettingsValue(const QString& value) {
    if (value == QStringLiteral("large")) {
        return Step::Large;
    }
    if (value == QStringLiteral("larger")) {
        return Step::Larger;
    }
    return Step::Normal;
}

} // namespace AppTextScale

#endif // APPTEXTSCALE_H
