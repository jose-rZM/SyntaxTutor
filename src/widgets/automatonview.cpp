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

#include "automatonview.h"

#include <QFontDatabase>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsTextItem>
#include <QMouseEvent>
#include <QQueue>
#include <QToolTip>
#include <QWheelEvent>
#include <QtMath>
#include <algorithm>
#include <map>

namespace {
constexpr qreal  kNodeRadius   = 24.0;
constexpr qreal  kLevelSpacing = 150.0;
constexpr qreal  kRowSpacing   = 96.0;
constexpr qreal  kArrowLength  = 11.0;
constexpr qreal  kArrowWidth   = 8.0;
constexpr double kMinZoom      = 0.4;
constexpr double kMaxZoom      = 2.5;

const QColor kNodeFill(0x2B, 0x31, 0x33);
const QColor kNodeBorder(0x3A, 0x41, 0x44);
const QColor kNodeText(0xF1, 0xF4, 0xF5);
const QColor kCurrentAccent(0x11, 0xB3, 0xBC);
const QColor kConflictAccent(0xD9, 0x53, 0x4F);
const QColor kReduceAccent(0xE5, 0xB5, 0x67);
const QColor kEdgeColor(0x5F, 0x6C, 0x70);
const QColor kEdgeLabelColor(0x9F, 0xCF, 0xD2);
const QColor kPlaceholderColor(0x6F, 0x7A, 0x7E);
} // namespace

AutomatonView::AutomatonView(QWidget* parent) : QGraphicsView(parent) {
    setObjectName("slrAutomatonView");
    scene_ = new QGraphicsScene(this);
    setScene(scene_);
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::TextAntialiasing, true);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    viewport()->setCursor(Qt::ArrowCursor);
}

QHash<unsigned, int> AutomatonView::computeLevels(
    const QVector<AutomatonTransitionInfo>& transitions) const {
    QHash<unsigned, QSet<unsigned>> adjacency;
    for (const AutomatonTransitionInfo& transition : transitions) {
        adjacency[transition.from].insert(transition.to);
    }

    QHash<unsigned, int> levels;
    if (nodes_.contains(0)) {
        levels.insert(0, 0);
        QQueue<unsigned> pending;
        pending.enqueue(0);
        while (!pending.isEmpty()) {
            const unsigned from = pending.dequeue();
            QList<unsigned> next = adjacency.value(from).values();
            std::sort(next.begin(), next.end());
            for (unsigned to : next) {
                if (!levels.contains(to)) {
                    levels.insert(to, levels.value(from) + 1);
                    pending.enqueue(to);
                }
            }
        }
    }

    // Unreachable states (should not happen) go to one extra column.
    int maxLevel = 0;
    for (int level : levels) {
        maxLevel = qMax(maxLevel, level);
    }
    for (auto it = nodes_.cbegin(); it != nodes_.cend(); ++it) {
        if (!levels.contains(it.key())) {
            levels.insert(it.key(), maxLevel + 1);
        }
    }
    return levels;
}

void AutomatonView::setAutomaton(
    const QVector<AutomatonStateInfo>&      states,
    const QVector<AutomatonTransitionInfo>& transitions) {
    scene_->clear();
    nodes_.clear();
    edges_.clear();
    conflictIds_.clear();
    reduceIds_.clear();
    currentId_     = -1;
    fullyRevealed_ = false;

    for (const AutomatonStateInfo& info : states) {
        Node node;
        node.id = info.id;
        nodes_.insert(info.id, node);
    }

    // Deterministic BFS-level layout: columns are BFS levels from I0 and
    // rows follow ascending state ids, so the picture is stable no matter
    // in which order states get revealed.
    const QHash<unsigned, int>    levels = computeLevels(transitions);
    std::map<int, QList<unsigned>> byLevel;
    for (auto it = levels.cbegin(); it != levels.cend(); ++it) {
        byLevel[it.value()].append(it.key());
    }

    QFont labelFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    labelFont.setPointSize(12);
    labelFont.setBold(true);

    for (auto& [level, ids] : byLevel) {
        std::sort(ids.begin(), ids.end());
        const qreal columnHeight = (ids.size() - 1) * kRowSpacing;
        for (int row = 0; row < ids.size(); ++row) {
            Node& node  = nodes_[ids.at(row)];
            node.center = QPointF(level * kLevelSpacing,
                                  row * kRowSpacing - columnHeight / 2.0);
        }
    }

    for (const AutomatonStateInfo& info : states) {
        Node& node = nodes_[info.id];

        node.circle = scene_->addEllipse(
            QRectF(node.center.x() - kNodeRadius, node.center.y() - kNodeRadius,
                   kNodeRadius * 2, kNodeRadius * 2));
        node.circle->setZValue(1);
        node.circle->setToolTip(info.itemsText);

        node.label = scene_->addSimpleText(QString("I%1").arg(info.id));
        node.label->setFont(labelFont);
        node.label->setBrush(kNodeText);
        const QRectF labelBounds = node.label->boundingRect();
        node.label->setPos(node.center.x() - labelBounds.width() / 2.0,
                           node.center.y() - labelBounds.height() / 2.0);
        node.label->setZValue(2);
        node.label->setToolTip(info.itemsText);

        applyNodeStyle(node);
        setNodeVisible(node, false);
    }

    // Combine transitions that share (from, to) into one labelled edge.
    std::map<std::pair<unsigned, unsigned>, QStringList> combined;
    for (const AutomatonTransitionInfo& transition : transitions) {
        combined[{transition.from, transition.to}].append(transition.symbol);
    }
    QSet<QPair<unsigned, unsigned>> pairs;
    for (const auto& [key, symbols] : combined) {
        pairs.insert({key.first, key.second});
    }

    for (const auto& [key, symbols] : combined) {
        if (!nodes_.contains(key.first) || !nodes_.contains(key.second)) {
            continue;
        }
        Edge edge;
        edge.from    = key.first;
        edge.to      = key.second;
        edge.symbols = symbols;
        edge.symbols.sort();
        buildEdgeGeometry(edge);
        setEdgeVisible(edge, false);
        edges_.append(edge);
    }

    const QRectF bounds = scene_->itemsBoundingRect();
    scene_->setSceneRect(bounds.adjusted(-50, -50, 50, 50));

    placeholder_ = scene_->addText(
        tr("El autómata aparecerá cuando\nconstruyas el estado inicial I0."));
    placeholder_->setDefaultTextColor(kPlaceholderColor);
    QFont placeholderFont = placeholder_->font();
    placeholderFont.setPointSize(12);
    placeholder_->setFont(placeholderFont);
    const QRectF placeholderBounds = placeholder_->boundingRect();
    placeholder_->setPos(-placeholderBounds.width() / 2.0,
                         -placeholderBounds.height() / 2.0);
    placeholder_->setZValue(3);

    resetTransform();
    zoom_ = 1.0;
    centerOn(placeholder_);
}

void AutomatonView::buildEdgeGeometry(Edge& edge) {
    const Node& fromNode = nodes_[edge.from];
    const Node& toNode   = nodes_[edge.to];

    QPainterPath path;
    QPointF      arrowTip;
    QPointF      arrowDirection;
    QPointF      labelPos;

    if (edge.from == edge.to) {
        // Self loop drawn above the node.
        const QPointF c     = fromNode.center;
        const QPointF start = c + QPointF(-12.0, -kNodeRadius + 6.0);
        const QPointF end   = c + QPointF(14.0, -kNodeRadius + 2.0);
        path.moveTo(start);
        path.cubicTo(c + QPointF(-34.0, -kNodeRadius - 44.0),
                     c + QPointF(34.0, -kNodeRadius - 44.0), end);
        arrowTip       = end;
        arrowDirection = QPointF(0.25, 1.0);
        labelPos       = c + QPointF(0.0, -kNodeRadius - 48.0);
    } else {
        const QPointF delta  = toNode.center - fromNode.center;
        const qreal   length = std::hypot(delta.x(), delta.y());
        const QPointF unit   = length > 0 ? delta / length : QPointF(1, 0);
        const QPointF normal(-unit.y(), unit.x());

        // Bend long or backward edges so they do not cross nodes placed in
        // between; a reverse edge bends to the other side automatically.
        const bool backward = edge.to <= edge.from;
        qreal      bend     = 0.0;
        if (backward || length > kLevelSpacing * 1.4) {
            bend = backward ? -34.0 : 34.0;
        }

        const QPointF start = fromNode.center + unit * kNodeRadius;
        const QPointF end =
            toNode.center - unit * (kNodeRadius + kArrowLength * 0.6);
        const QPointF control =
            (start + end) / 2.0 + normal * (bend * 2.0);

        path.moveTo(start);
        path.quadTo(control, end);

        arrowTip       = end + unit * (kArrowLength * 0.6);
        arrowDirection = arrowTip - control;
        labelPos = path.pointAtPercent(0.5) + normal * (bend >= 0 ? 12.0 : -16.0);
    }

    QPen edgePen(kEdgeColor, 1.6);
    edgePen.setCapStyle(Qt::RoundCap);
    edge.path = scene_->addPath(path, edgePen);
    edge.path->setZValue(0);

    const qreal dirLength =
        std::hypot(arrowDirection.x(), arrowDirection.y());
    const QPointF unitDir =
        dirLength > 0 ? arrowDirection / dirLength : QPointF(1, 0);
    const QPointF normalDir(-unitDir.y(), unitDir.x());
    QPolygonF arrowHead;
    arrowHead << arrowTip
              << arrowTip - unitDir * kArrowLength +
                     normalDir * (kArrowWidth / 2.0)
              << arrowTip - unitDir * kArrowLength -
                     normalDir * (kArrowWidth / 2.0);
    edge.arrow = scene_->addPolygon(arrowHead, Qt::NoPen, kEdgeColor);
    edge.arrow->setZValue(0);

    QFont labelFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    labelFont.setPointSize(10);
    edge.label = scene_->addSimpleText(edge.symbols.join(", "));
    edge.label->setFont(labelFont);
    edge.label->setBrush(kEdgeLabelColor);
    const QRectF labelBounds = edge.label->boundingRect();
    edge.label->setPos(labelPos.x() - labelBounds.width() / 2.0,
                       labelPos.y() - labelBounds.height() / 2.0);
    edge.label->setZValue(2);
}

void AutomatonView::applyNodeStyle(Node& node) {
    QBrush brush(kNodeFill);
    QPen   pen(kNodeBorder, 1.6);

    if (conflictIds_.contains(node.id)) {
        QColor tint = kConflictAccent;
        tint.setAlpha(46);
        brush = QBrush(tint);
        pen   = QPen(kConflictAccent, 2.0);
    } else if (reduceIds_.contains(node.id)) {
        QColor tint = kReduceAccent;
        tint.setAlpha(38);
        brush = QBrush(tint);
        pen   = QPen(kReduceAccent, 2.0);
    }
    if (currentId_ >= 0 && node.id == static_cast<unsigned>(currentId_)) {
        pen = QPen(kCurrentAccent, 3.0);
    }

    if (node.circle != nullptr) {
        node.circle->setBrush(brush);
        node.circle->setPen(pen);
    }
}

void AutomatonView::setNodeVisible(Node& node, bool visible) {
    node.revealed = visible;
    if (node.circle != nullptr) {
        node.circle->setVisible(visible);
    }
    if (node.label != nullptr) {
        node.label->setVisible(visible);
    }
}

void AutomatonView::setEdgeVisible(Edge& edge, bool visible) {
    edge.revealed = visible;
    if (edge.path != nullptr) {
        edge.path->setVisible(visible);
    }
    if (edge.arrow != nullptr) {
        edge.arrow->setVisible(visible);
    }
    if (edge.label != nullptr) {
        edge.label->setVisible(visible);
    }
}

void AutomatonView::updatePlaceholder() {
    if (placeholder_ != nullptr) {
        placeholder_->setVisible(visibleStateCount() == 0);
    }
}

void AutomatonView::revealState(unsigned id) {
    auto it = nodes_.find(id);
    if (it == nodes_.end() || it->revealed) {
        return;
    }
    setNodeVisible(*it, true);
    updatePlaceholder();
    if (it->circle != nullptr) {
        ensureVisible(it->circle, 60, 60);
    }
}

void AutomatonView::revealTransition(unsigned from, unsigned to) {
    revealState(from);
    revealState(to);
    for (Edge& edge : edges_) {
        if (edge.from == from && edge.to == to && !edge.revealed) {
            setEdgeVisible(edge, true);
        }
    }
}

void AutomatonView::revealAll() {
    if (fullyRevealed_) {
        return;
    }
    fullyRevealed_ = true;
    for (Node& node : nodes_) {
        setNodeVisible(node, true);
    }
    for (Edge& edge : edges_) {
        setEdgeVisible(edge, true);
    }
    updatePlaceholder();

    // Fit the whole automaton, never zooming in past 1:1.
    resetTransform();
    zoom_ = 1.0;
    const QRectF bounds = scene_->sceneRect();
    if (!bounds.isEmpty() && viewport() != nullptr) {
        const qreal scaleX = viewport()->width() / bounds.width();
        const qreal scaleY = viewport()->height() / bounds.height();
        const qreal factor = qMin(1.0, qMin(scaleX, scaleY));
        if (factor < 1.0 && factor > 0.0) {
            scale(factor, factor);
            zoom_ = factor;
        }
    }
    centerOn(bounds.center());
}

void AutomatonView::setCurrentState(int id) {
    if (currentId_ == id) {
        return;
    }
    currentId_ = id;
    for (Node& node : nodes_) {
        applyNodeStyle(node);
    }
    if (id >= 0) {
        auto it = nodes_.find(static_cast<unsigned>(id));
        if (it != nodes_.end() && it->revealed && it->circle != nullptr) {
            ensureVisible(it->circle, 60, 60);
        }
    }
}

void AutomatonView::setConflictStates(const QSet<unsigned>& ids) {
    conflictIds_ = ids;
    for (Node& node : nodes_) {
        applyNodeStyle(node);
    }
}

void AutomatonView::setReduceStates(const QSet<unsigned>& ids) {
    reduceIds_ = ids;
    for (Node& node : nodes_) {
        applyNodeStyle(node);
    }
}

void AutomatonView::clearStateMarks() {
    if (conflictIds_.isEmpty() && reduceIds_.isEmpty()) {
        return;
    }
    conflictIds_.clear();
    reduceIds_.clear();
    for (Node& node : nodes_) {
        applyNodeStyle(node);
    }
}

bool AutomatonView::isStateVisible(unsigned id) const {
    auto it = nodes_.constFind(id);
    return it != nodes_.constEnd() && it->revealed;
}

int AutomatonView::visibleStateCount() const {
    int count = 0;
    for (const Node& node : nodes_) {
        if (node.revealed) {
            ++count;
        }
    }
    return count;
}

int AutomatonView::visibleTransitionCount() const {
    int count = 0;
    for (const Edge& edge : edges_) {
        if (edge.revealed) {
            ++count;
        }
    }
    return count;
}

bool AutomatonView::isFullyRevealed() const {
    if (fullyRevealed_) {
        return true;
    }
    return visibleStateCount() == totalStateCount() &&
           visibleTransitionCount() == static_cast<int>(edges_.size());
}

void AutomatonView::wheelEvent(QWheelEvent* event) {
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        const double factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
        const double target = qBound(kMinZoom, zoom_ * factor, kMaxZoom);
        if (!qFuzzyCompare(target, zoom_)) {
            scale(target / zoom_, target / zoom_);
            zoom_ = target;
        }
        event->accept();
        return;
    }
    QGraphicsView::wheelEvent(event);
}

void AutomatonView::mousePressEvent(QMouseEvent* event) {
    if (QGraphicsItem* item = itemAt(event->pos());
        item != nullptr && !item->toolTip().isEmpty()) {
        QToolTip::showText(event->globalPosition().toPoint(),
                           item->toolTip(), this);
    }
    QGraphicsView::mousePressEvent(event);
}
