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

#include <QString>
#include <QtGlobal>

/**
 * @brief The user's text size preference, as a percentage.
 *
 * Only the preference lives here: how big each piece of text is comes from
 * the type scale in @ref AppTypography, which multiplies its point sizes by
 * @ref factor. The platform's own point-to-pixel mapping is left to Qt, so
 * 100% means "as designed" on every system rather than a DPI correction.
 */
namespace AppTextScale {

/// @brief Smallest text size the user can pick, in percent.
constexpr int kMinPercent = 80;
/// @brief Largest text size the user can pick, in percent.
constexpr int kMaxPercent = 200;
/// @brief Size used until the user picks one, in percent.
constexpr int kDefaultPercent = 100;
/// @brief Sizes offered directly in the menu, in percent.
constexpr int kPresetPercents[] = {90, 100, 115, 130, 150, 175, 200};

/// @brief Keeps a percentage inside the allowed range.
inline int clampPercent(int percent) {
    return qBound(kMinPercent, percent, kMaxPercent);
}

/**
 * @brief The selected size, in percent.
 *
 * An inline function with a static local so every translation unit shares
 * one instance without needing a .cpp.
 */
inline int& currentPercent() {
    static int percent = kDefaultPercent;
    return percent;
}

/// @brief Multiplier currently in effect.
inline double factor() { return currentPercent() / 100.0; }

/// @brief Settings key holding the selected size, in percent.
inline QString settingsKey() { return QStringLiteral("ui/textScalePercent"); }

/// @brief Key used by the first version of this setting, which named steps.
inline QString legacySettingsKey() { return QStringLiteral("ui/textScale"); }

/// @brief Percentage matching one of the original named steps.
inline int percentFromLegacyValue(const QString& value) {
    if (value == QStringLiteral("large")) {
        return 115;
    }
    if (value == QStringLiteral("larger")) {
        return 130;
    }
    return kDefaultPercent;
}

} // namespace AppTextScale

#endif // APPTEXTSCALE_H
