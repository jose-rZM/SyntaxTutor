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

#include "automatonviewerdialog.h"
#include "automatonview.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

AutomatonViewerDialog::AutomatonViewerDialog(AutomatonView* view,
                                             QWidget*       parent)
    : QDialog(parent), view_(view) {
    setObjectName("automatonViewerDialog");
    setProperty("automatonViewer", true);
    setWindowTitle(tr("Autómata LR(0)"));
    setModal(false);
    resize(820, 620);
    setMinimumSize(520, 420);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(16, 14, 16, 16);
    rootLayout->setSpacing(12);

    auto* toolbar = new QHBoxLayout();
    toolbar->setSpacing(8);

    auto* hint = new QLabel(
        tr("Ctrl + rueda para acercar, arrastra para desplazar"), this);
    hint->setObjectName("automatonViewerHint");
    toolbar->addWidget(hint);
    toolbar->addStretch(1);

    auto makeToolButton = [this](const QString& text, const QString& tip) {
        auto* button = new QPushButton(text, this);
        button->setObjectName("automatonViewerToolButton");
        button->setToolTip(tip);
        button->setCursor(Qt::PointingHandCursor);
        button->setAutoDefault(false);
        button->setDefault(false);
        return button;
    };

    auto* zoomOutButton = makeToolButton(QStringLiteral("−"), tr("Alejar"));
    auto* zoomInButton  = makeToolButton(QStringLiteral("+"), tr("Acercar"));
    auto* fitButton = makeToolButton(tr("Ajustar"), tr("Ajustar a la vista"));
    auto* centerButton =
        makeToolButton(tr("Centrar estado"), tr("Centrar el estado actual"));

    toolbar->addWidget(zoomOutButton);
    toolbar->addWidget(zoomInButton);
    toolbar->addWidget(fitButton);
    toolbar->addWidget(centerButton);
    rootLayout->addLayout(toolbar);

    view_->setParent(this);
    rootLayout->addWidget(view_, 1);
    view_->show();

    connect(zoomOutButton, &QPushButton::clicked, view_,
            &AutomatonView::zoomOut);
    connect(zoomInButton, &QPushButton::clicked, view_, &AutomatonView::zoomIn);
    connect(fitButton, &QPushButton::clicked, view_, &AutomatonView::fitToView);
    connect(centerButton, &QPushButton::clicked, view_,
            &AutomatonView::centerOnCurrentState);
}

AutomatonViewerDialog::~AutomatonViewerDialog() {
    // The view is owned by the tutor, not by this dialog. Detach it so it
    // survives the dialog and can be embedded again next time.
    if (view_ != nullptr && view_->parent() == this) {
        view_->hide();
        view_->setParent(nullptr);
    }
}
