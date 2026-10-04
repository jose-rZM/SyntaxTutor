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

#ifndef AUTOMATONVIEWERDIALOG_H
#define AUTOMATONVIEWERDIALOG_H

#include <QDialog>
#include <QPointer>

class AutomatonView;

/**
 * @class AutomatonViewerDialog
 * @brief Non-modal floating window that hosts the LR(0) automaton view.
 *
 * The dialog is a thin shell: it embeds an externally-owned AutomatonView
 * (so its progressive reveal state is preserved across open/close cycles)
 * and adds a toolbar with zoom in/out, fit-to-view and center-current
 * actions. It never owns the view: on destruction it reparents the view out
 * so the tutor can keep reusing it. Being non-modal, the student can keep it
 * open while continuing the exercise.
 */
class AutomatonViewerDialog : public QDialog {
    Q_OBJECT
  public:
    /**
     * @brief Builds the viewer around an existing automaton view.
     *
     * @param view The automaton view to embed; ownership is not taken.
     * @param parent Parent widget.
     */
    explicit AutomatonViewerDialog(AutomatonView* view,
                                   QWidget*       parent = nullptr);
    ~AutomatonViewerDialog() override;

  private:
    // QPointer so the dialog never dereferences the borrowed view if it is
    // destroyed first (e.g. by the tutor during teardown).
    QPointer<AutomatonView> view_;
};

#endif // AUTOMATONVIEWERDIALOG_H
