#include "grammarview.h"

#include "appfonts.h"
#include <QGridLayout>
#include <QLabel>
#include <QSizePolicy>
#include <algorithm>

namespace {

QFont grammarFont() {
    QFont font = appMonospaceFont();
    font.setPixelSize(15);
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

void GrammarView::clearRows() {
    while (QLayoutItem* item = gridLayout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
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

    for (int i = 0; i < currentRows.size(); ++i) {
        const Row& row = currentRows.at(i);

        int column = 0;
        if (showIndex) {
            gridLayout->addWidget(makeCell(row.index, monoFont, this, "#F1F1F1"),
                                  i, column++, Qt::AlignLeft | Qt::AlignTop);
        }
        gridLayout->addWidget(makeCell(row.lhs, monoFont, this, "#F1F1F1"), i,
                              column++, Qt::AlignLeft | Qt::AlignTop);
        gridLayout->addWidget(makeCell(row.marker, monoFont, this, "#F1F1F1"), i,
                              column++, Qt::AlignLeft | Qt::AlignTop);
        gridLayout->addWidget(makeCell(row.rhs, monoFont, this, "#F1F1F1"), i,
                              column, Qt::AlignLeft | Qt::AlignTop);
    }

    const int lastColumn = showIndex ? 3 : 2;
    for (int column = 0; column < 4; ++column) {
        gridLayout->setColumnStretch(column, column == lastColumn ? 1 : 0);
    }

    gridLayout->setRowStretch(currentRows.size(), 1);
    adjustSize();
}
