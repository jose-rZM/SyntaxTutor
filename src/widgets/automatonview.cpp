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

#include "appfonts.h"
#include "apptypography.h"

#include <QApplication>
#include <QFontDatabase>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QQueue>
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
const QColor kDetailsFill(0x1B, 0x1F, 0x20);
// The view's background, as set by app.qss for #slrAutomatonView.
const QColor kCanvas(0x21, 0x25, 0x26);

// An edge is a cubic Bezier between the two node centres; everything that
// has to meet the line - where it leaves and enters the circles, the arrow
// tip and its direction - is read off that same curve. Computing them from
// the straight line between the centres, as before, left curved edges with
// an arrowhead off the end of their line.
struct Cubic {
    QPointF p0, p1, p2, p3;

    QPointF at(qreal t) const {
        const qreal u = 1.0 - t;
        return u * u * u * p0 + 3 * u * u * t * p1 + 3 * u * t * t * p2 +
               t * t * t * p3;
    }

    QPointF tangent(qreal t) const {
        const qreal u = 1.0 - t;
        return 3 * (u * u * (p1 - p0) + 2 * u * t * (p2 - p1) +
                    t * t * (p3 - p2));
    }

    /// The part of the curve between parameters @p a and @p b.
    Cubic segment(qreal a, qreal b) const {
        const Cubic right = split(a).second;
        const qreal local = a < 1.0 ? (b - a) / (1.0 - a) : 1.0;
        return right.split(local).first;
    }

    std::pair<Cubic, Cubic> split(qreal t) const {
        const QPointF a = lerp(p0, p1, t), b = lerp(p1, p2, t),
                      c = lerp(p2, p3, t);
        const QPointF d = lerp(a, b, t), e = lerp(b, c, t);
        const QPointF f = lerp(d, e, t);
        return {Cubic{p0, a, d, f}, Cubic{f, e, c, p3}};
    }

    static QPointF lerp(const QPointF& x, const QPointF& y, qreal t) {
        return x + (y - x) * t;
    }
};

qreal distance(const QPointF& a, const QPointF& b) {
    return std::hypot(a.x() - b.x(), a.y() - b.y());
}

/// Parameter in [lo, hi] where the curve's distance to @p point crosses
/// @p radius; @p insideAtLo says on which side the curve starts.
qreal crossing(const Cubic& curve, const QPointF& point, qreal radius, qreal lo,
               qreal hi, bool insideAtLo) {
    for (int i = 0; i < 40; ++i) {
        const qreal mid    = (lo + hi) / 2.0;
        const bool  inside = distance(curve.at(mid), point) < radius;
        if (inside == insideAtLo) {
            lo = mid;
        } else {
            hi = mid;
        }
    }
    return (lo + hi) / 2.0;
}

QPointF normalized(const QPointF& v, const QPointF& fallback) {
    const qreal length = std::hypot(v.x(), v.y());
    return length > 0 ? v / length : fallback;
}
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
            const unsigned  from = pending.dequeue();
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
    hideStateDetails();
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
    const QHash<unsigned, int>     levels = computeLevels(transitions);
    std::map<int, QList<unsigned>> byLevel;
    for (auto it = levels.cbegin(); it != levels.cend(); ++it) {
        byLevel[it.value()].append(it.key());
    }

    QFont labelFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    labelFont.setPointSizeF(AppTypography::points(AppTypography::Role::Label));
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
        node.itemsText = info.itemsText;

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
    placeholderFont.setPointSizeF(
        AppTypography::points(AppTypography::Role::Label));
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

    Cubic   curve;
    QPointF labelPos;
    // Parameter ranges where the curve leaves the source circle and enters
    // the target one. A loop starts and ends at the same centre, with its
    // apex at t = 0.5.
    qreal leaveHi = 1.0, enterLo = 0.0;

    if (edge.from == edge.to) {
        const QPointF c = fromNode.center;
        curve           = Cubic{c, c + QPointF(-44.0, -kNodeRadius - 52.0),
                      c + QPointF(44.0, -kNodeRadius - 52.0), c};
        leaveHi         = 0.5;
        enterLo         = 0.5;
        labelPos        = curve.at(0.5) - QPointF(0.0, 10.0);
    } else {
        const QPointF delta = toNode.center - fromNode.center;
        const QPointF unit  = normalized(delta, QPointF(1, 0));
        const QPointF normal(-unit.y(), unit.x());
        const qreal   length = std::hypot(delta.x(), delta.y());

        // The quadratic curve through a control point pushed `bend` along
        // the normal, raised to a cubic.
        const auto bentCurve = [&](qreal bend) {
            const QPointF control = (fromNode.center + toNode.center) / 2.0 +
                                    normal * (bend * 2.0);
            return Cubic{
                fromNode.center,
                fromNode.center + (control - fromNode.center) * 2.0 / 3.0,
                toNode.center + (control - toNode.center) * 2.0 / 3.0,
                toNode.center};
        };
        const auto clearsOtherNodes = [&](const Cubic& candidate) {
            for (int i = 0; i <= 50; ++i) {
                const QPointF point = candidate.at(i / 50.0);
                for (const Node& other : std::as_const(nodes_)) {
                    if (other.id != edge.from && other.id != edge.to &&
                        distance(point, other.center) < kNodeRadius + 6.0) {
                        return false;
                    }
                }
            }
            return true;
        };

        // Straight when the way is clear; otherwise the gentlest bend that
        // goes around every state in between, trying one side and then the
        // other. Backward edges try the opposite side first, so an edge and
        // its reverse never overlap, and long or backward edges always bend
        // a little to keep apart from the straight ones.
        const bool  backward  = edge.to <= edge.from;
        const qreal preferred = backward ? -1.0 : 1.0;
        const qreal minimum =
            (backward || length > kLevelSpacing * 1.4) ? 34.0 : 0.0;
        qreal bend = minimum * preferred;
        curve      = bentCurve(bend);
        if (!clearsOtherNodes(curve)) {
            const auto findClearBend = [&]() {
                for (const qreal magnitude : {34.0, 60.0, 90.0, 120.0, 160.0}) {
                    for (const qreal side : {preferred, -preferred}) {
                        const Cubic candidate = bentCurve(magnitude * side);
                        if (clearsOtherNodes(candidate)) {
                            bend  = magnitude * side;
                            curve = candidate;
                            return;
                        }
                    }
                }
            };
            // If no bend clears, the edge keeps its default shape.
            findClearBend();
        }
        labelPos = curve.at(0.5) + normal * (bend >= 0 ? 12.0 : -16.0);
    }

    const qreal tLeave =
        crossing(curve, fromNode.center, kNodeRadius, 0.0, leaveHi, true);
    const qreal tTip =
        crossing(curve, toNode.center, kNodeRadius, enterLo, 1.0, false);
    const QPointF arrowTip = curve.at(tTip);
    const QPointF unitDir  = normalized(curve.tangent(tTip),
                                        arrowTip - curve.at(tLeave));

    // The line stops inside the arrowhead rather than at its tip, so the two
    // join without a gap and the line never pokes out past the point.
    const qreal tBase = crossing(curve, arrowTip, kArrowLength * 0.7, tLeave,
                                 tTip, false);
    const Cubic shown = curve.segment(tLeave, tBase);
    QPainterPath path(shown.p0);
    path.cubicTo(shown.p1, shown.p2, shown.p3);

    QPen edgePen(kEdgeColor, 1.6);
    edgePen.setCapStyle(Qt::RoundCap);
    edge.path = scene_->addPath(path, edgePen);
    edge.path->setZValue(0);

    const QPointF normalDir(-unitDir.y(), unitDir.x());
    QPolygonF     arrowHead;
    arrowHead << arrowTip
              << arrowTip - unitDir * kArrowLength +
                     normalDir * (kArrowWidth / 2.0)
              << arrowTip - unitDir * kArrowLength -
                     normalDir * (kArrowWidth / 2.0);
    edge.arrow = scene_->addPolygon(arrowHead, Qt::NoPen, kEdgeColor);
    edge.arrow->setZValue(0.5);

    QFont labelFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    labelFont.setPointSizeF(AppTypography::points(AppTypography::Role::Micro));
    edge.label = scene_->addSimpleText(edge.symbols.join(", "));
    edge.label->setFont(labelFont);
    edge.label->setBrush(kEdgeLabelColor);
    const QRectF labelBounds = edge.label->boundingRect();
    edge.label->setPos(labelPos.x() - labelBounds.width() / 2.0,
                       labelPos.y() - labelBounds.height() / 2.0);
    edge.label->setZValue(2);

    // A patch of canvas behind the label, so a line crossing it does not
    // strike the symbol through. It is a child of the label and so shows
    // and hides with it.
    QPainterPath halo;
    halo.addRoundedRect(labelBounds.adjusted(-3, -1, 3, 1), 3, 3);
    auto* haloItem = new QGraphicsPathItem(halo, edge.label);
    haloItem->setPen(Qt::NoPen);
    haloItem->setBrush(kCanvas);
    haloItem->setFlag(QGraphicsItem::ItemStacksBehindParent);
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
    fitToView();
}

void AutomatonView::zoomIn() {
    const double target = qBound(kMinZoom, zoom_ * 1.15, kMaxZoom);
    if (!qFuzzyCompare(target, zoom_)) {
        scale(target / zoom_, target / zoom_);
        zoom_ = target;
    }
}

void AutomatonView::zoomOut() {
    const double target = qBound(kMinZoom, zoom_ / 1.15, kMaxZoom);
    if (!qFuzzyCompare(target, zoom_)) {
        scale(target / zoom_, target / zoom_);
        zoom_ = target;
    }
}

void AutomatonView::fitToView() {
    // Fit the revealed content, never zooming in past 1:1.
    resetTransform();
    zoom_ = 1.0;

    QRectF bounds;
    for (const Node& node : nodes_) {
        if (node.revealed && node.circle != nullptr) {
            bounds = bounds.united(node.circle->sceneBoundingRect());
        }
    }
    if (bounds.isEmpty()) {
        bounds = scene_->sceneRect();
    } else {
        bounds.adjust(-40, -40, 40, 40);
    }

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

void AutomatonView::centerOnCurrentState() {
    if (currentId_ < 0) {
        return;
    }
    auto it = nodes_.find(static_cast<unsigned>(currentId_));
    if (it != nodes_.end() && it->circle != nullptr) {
        if (!it->revealed) {
            revealState(static_cast<unsigned>(currentId_));
        }
        centerOn(it->circle);
    }
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
    pressPos_ = event->pos();
    QGraphicsView::mousePressEvent(event);
}

void AutomatonView::mouseReleaseEvent(QMouseEvent* event) {
    QGraphicsView::mouseReleaseEvent(event);
    // Dragging pans the view; only a press and release in place is a click.
    if (event->button() != Qt::LeftButton ||
        (event->pos() - pressPos_).manhattanLength() >=
            QApplication::startDragDistance()) {
        return;
    }
    const int id = nodeAt(event->pos());
    if (id < 0 || id == shownId_) {
        hideStateDetails();
    } else {
        showStateDetails(static_cast<unsigned>(id));
    }
}

void AutomatonView::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape && shownId_ >= 0) {
        hideStateDetails();
        event->accept();
        return;
    }
    QGraphicsView::keyPressEvent(event);
}

int AutomatonView::nodeAt(const QPoint& viewPos) const {
    const QList<QGraphicsItem*> hits = items(viewPos);
    for (const Node& node : nodes_) {
        if (node.revealed && (hits.contains(node.circle) ||
                              hits.contains(node.label))) {
            return static_cast<int>(node.id);
        }
    }
    return -1;
}

void AutomatonView::showStateDetails(unsigned id) {
    auto it = nodes_.constFind(id);
    if (it == nodes_.constEnd() || !it->revealed) {
        return;
    }
    hideStateDetails();

    // One item per line, sorted: the items come from an unordered set, and
    // the same state should read the same every time it is opened.
    QStringList lines;
    for (QString line : it->itemsText.split(u'\n', Qt::SkipEmptyParts)) {
        line = line.trimmed();
        if (line.startsWith(QStringLiteral("- "))) {
            line = line.mid(2);
        }
        lines << line;
    }
    lines.sort();

    constexpr qreal kPadding = 10.0;
    auto*           text     = new QGraphicsSimpleTextItem;
    text->setText(QStringLiteral("I%1\n%2").arg(id).arg(lines.join(u'\n')));
    text->setFont(
        AppTypography::font(AppTypography::Role::Label, appMonospaceFont()));
    text->setBrush(kNodeText);

    const QRectF  textBounds = text->boundingRect();
    const QSizeF  size(textBounds.width() + 2 * kPadding,
                       textBounds.height() + 2 * kPadding);
    QPainterPath  frame;
    frame.addRoundedRect(QRectF(QPointF(0, 0), size), 8, 8);
    details_ = scene_->addPath(frame, QPen(kCurrentAccent, 1.2),
                               kDetailsFill);
    details_->setZValue(10);
    text->setParentItem(details_);
    text->setPos(kPadding, kPadding);

    // Beside the node, on the side that covers fewer states; the right one
    // when they tie. The scene grows to fit it and the view scrolls there.
    const QPointF center = it->center;
    const qreal   top    = center.y() - size.height() / 2.0;
    const QRectF  right(QPointF(center.x() + kNodeRadius + 12.0, top), size);
    const QRectF  left(
        QPointF(center.x() - kNodeRadius - 12.0 - size.width(), top), size);
    const auto covered = [this](const QRectF& area) {
        int count = 0;
        for (const Node& node : std::as_const(nodes_)) {
            if (node.revealed &&
                area.adjusted(-kNodeRadius, -kNodeRadius, kNodeRadius,
                              kNodeRadius)
                    .contains(node.center)) {
                ++count;
            }
        }
        return count;
    };
    details_->setPos(covered(left) < covered(right) ? left.topLeft()
                                                    : right.topLeft());
    scene_->setSceneRect(scene_->sceneRect().united(
        details_->sceneBoundingRect().adjusted(-20, -20, 20, 20)));

    shownId_ = static_cast<int>(id);
    ensureVisible(details_, 20, 20);
}

void AutomatonView::hideStateDetails() {
    if (details_ != nullptr) {
        scene_->removeItem(details_);
        delete details_;
        details_ = nullptr;
    }
    shownId_ = -1;
}
