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

#ifndef CUSTOMTEXTEDIT_H
#define CUSTOMTEXTEDIT_H

#include <QTextEdit>

class QPropertyAnimation;

class CustomTextEdit : public QTextEdit {
    Q_OBJECT
    Q_PROPERTY(int growHeight READ growHeight WRITE setGrowHeight)
  public:
    explicit CustomTextEdit(QWidget* parent = nullptr);

    /// User-visible name of the newline shortcut for the current platform.
    static QString newlineShortcutText();

    /// Makes the widget height follow its content, animated, within
    /// [minHeight, maxHeight]. Content beyond maxHeight scrolls.
    void setAutoGrow(int minHeight, int maxHeight);

  signals:
    void sendRequested();

  protected:
    void keyPressEvent(QKeyEvent* event) override;

  private:
    int  growHeight() const;
    void setGrowHeight(int height);
    void updateGrowHeight();

    int                 minGrowHeight_ = 0;
    int                 maxGrowHeight_ = 0;
    QPropertyAnimation* growAnimation_ = nullptr;
};

#endif // CUSTOMTEXTEDIT_H
