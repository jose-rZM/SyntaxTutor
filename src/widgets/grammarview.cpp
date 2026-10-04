#include "grammarview.h"

#include "appfonts.h"
#include "apptypography.h"
#include <QGridLayout>
#include <QLabel>
#include <QSizePolicy>
#include <algorithm>

namespace {

QFont grammarFont() {
    QFont font = appMonospaceFont();
    font.setPointSizeF(AppTypography::points(AppTypography::Role::Reading));
    return font;
}

QLabel* makeCell(const QString& text, const QFont& font, QWidget* parent,
                 const QString& color = QString()) {
    auto* label = new QLabel(text, parent);
    label->setFont(font);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    if (!color.isEmpty()) {
        label->setStyleSheet(QString("color: %1;").arg(color));
    }
    return label;
}

}  // namespace

GrammarView::GrammarView(QWidget* parent) : QFrame(parent), gridLayout(new QGridLayout(this)) {
    setObjectName("grammarView");
    setFrameShape(QFrame::NoFrame);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    gridLayout->setContentsMargins(18, 18, 18, 18);
    gridLayout->setHorizontalSpacing(14);
    gridLayout->setVerticalSpacing(4);
    gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
}

void GrammarView::refresh() {
    if (currentRows.isEmpty()) {
        return;
    }
    setRows(currentRows);
    // Force the grid to recompute now: callers read naturalWidth() straight
    // after this, and a stale layout reports just the margins.
    gridLayout->activate();
}

int GrammarView::naturalWidth() const {
    return gridLayout->minimumSize().width();
}

void GrammarView::clearRows() {
    while (QLayoutItem* item = gridLayout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            // deleteLater only runs when the event loop gets to it, and an
            // old cell stays visible until then: on a rebuild it would paint
            // on top of the new ones. Take it out of sight right away.
            widget->hide();
            widget->setParent(nullptr);
            widget->deleteLater();
        }
        delete item;
    }
}

void GrammarView::setRows(const QVector<Row>& rows) {
    currentRows = rows;
    clearRows();

    const QFont monoFont = grammarFont();

    // SLR(1) numbers its rules, LL(1) leaves every index empty.
    const bool showIndex =
        std::any_of(currentRows.cbegin(), currentRows.cend(),
                    [](const Row& row) { return !row.index.isEmpty(); });

    QVector<QLabel*> created;
    created.reserve(currentRows.size() * 4);
    auto add = [&](QLabel* label, int row, int column) {
        gridLayout->addWidget(label, row, column,
                              Qt::AlignLeft | Qt::AlignTop);
        created.append(label);
    };

    for (int i = 0; i < currentRows.size(); ++i) {
        const Row& row = currentRows.at(i);

        int column = 0;
        if (showIndex) {
            add(makeCell(row.index, monoFont, this, "#F1F1F1"), i, column++);
        }
        add(makeCell(row.lhs, monoFont, this, "#F1F1F1"), i, column++);
        add(makeCell(row.marker, monoFont, this, "#F1F1F1"), i, column++);
        add(makeCell(row.rhs, monoFont, this, "#F1F1F1"), i, column);
    }

    // A child created under an already visible parent stays hidden until the
    // event loop runs, and the layout counts a hidden widget as empty. Show
    // them now so naturalWidth() is correct straight away, which is what the
    // tutors read to size the grammar panel.
    for (QLabel* label : created) {
        label->show();
    }

    const int lastColumn = showIndex ? 3 : 2;
    for (int column = 0; column < 4; ++column) {
        gridLayout->setColumnStretch(column, column == lastColumn ? 1 : 0);
    }

    gridLayout->setRowStretch(currentRows.size(), 1);
    adjustSize();
}
