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

#ifndef AUTOMATONVIEW_H
#define AUTOMATONVIEW_H

#include <QGraphicsView>
#include <QHash>
#include <QSet>
#include <QVector>

class QGraphicsEllipseItem;
class QGraphicsPathItem;
class QGraphicsPolygonItem;
class QGraphicsSimpleTextItem;
class QGraphicsTextItem;

/**
 * @struct AutomatonStateInfo
 * @brief One LR(0) state of the automaton to display.
 */
struct AutomatonStateInfo {
    /// @brief State id (the N in "IN").
    unsigned id;
    /// @brief Items of the state, shown when hovering or clicking the node.
    QString itemsText;
};

/**
 * @struct AutomatonTransitionInfo
 * @brief One delta(I, X) transition of the automaton to display.
 */
struct AutomatonTransitionInfo {
    unsigned from;
    QString  symbol;
    unsigned to;
};

/**
 * @class AutomatonView
 * @brief Progressive LR(0) automaton visualization.
 *
 * QGraphicsView-based panel that renders the LR(0) collection as a graph of
 * state nodes (I0, I1, ...) and labelled transition edges. The full
 * automaton is loaded up front with a deterministic BFS-level layout, but
 * every node and edge starts hidden: the SLR tutor reveals them as the
 * student constructs them, so the panel never spoils the construction.
 * Once the collection is complete the tutor calls revealAll() and the view
 * becomes a stable consultation tool, with optional highlights for the
 * current, conflict and reduce states.
 *
 * Hovering or clicking a node shows the LR(0) items of that state. The view
 * supports Ctrl+wheel zooming and drag scrolling.
 */
class AutomatonView : public QGraphicsView {
    Q_OBJECT
  public:
    explicit AutomatonView(QWidget* parent = nullptr);

    /**
     * @brief Loads the automaton and computes the layout. Everything starts
     * hidden.
     *
     * @param states All LR(0) states.
     * @param transitions All delta transitions.
     */
    void setAutomaton(const QVector<AutomatonStateInfo>&      states,
                      const QVector<AutomatonTransitionInfo>& transitions);

    /// @brief Makes a state node visible.
    void revealState(unsigned id);

    /**
     * @brief Makes a transition edge visible, together with both endpoint
     * states.
     */
    void revealTransition(unsigned from, unsigned to);

    /// @brief Reveals every state and transition (consultation mode).
    void revealAll();

    /// @brief Highlights one state as the one being analyzed (-1 for none).
    void setCurrentState(int id);

    /// @brief Marks states that contain an LR(0) conflict.
    void setConflictStates(const QSet<unsigned>& ids);

    /// @brief Marks states that can perform a reduction.
    void setReduceStates(const QSet<unsigned>& ids);

    /// @brief Clears conflict and reduce marks.
    void clearStateMarks();

    bool isStateVisible(unsigned id) const;
    int  visibleStateCount() const;
    int  totalStateCount() const { return static_cast<int>(nodes_.size()); }
    int  visibleTransitionCount() const;
    bool isFullyRevealed() const;
    int  currentStateId() const { return currentId_; }
    QSet<unsigned> conflictStates() const { return conflictIds_; }
    QSet<unsigned> reduceStates() const { return reduceIds_; }

  protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

  private:
    struct Node {
        unsigned                 id     = 0;
        QPointF                  center;
        QGraphicsEllipseItem*    circle = nullptr;
        QGraphicsSimpleTextItem* label  = nullptr;
        bool                     revealed = false;
    };

    struct Edge {
        unsigned                 from = 0;
        unsigned                 to   = 0;
        QStringList              symbols;
        QGraphicsPathItem*       path     = nullptr;
        QGraphicsPolygonItem*    arrow    = nullptr;
        QGraphicsSimpleTextItem* label    = nullptr;
        bool                     revealed = false;
    };

    QHash<unsigned, int> computeLevels(
        const QVector<AutomatonTransitionInfo>& transitions) const;
    void buildEdgeGeometry(Edge& edge);
    void applyNodeStyle(Node& node);
    void setNodeVisible(Node& node, bool visible);
    void setEdgeVisible(Edge& edge, bool visible);
    void updatePlaceholder();

    QGraphicsScene*       scene_       = nullptr;
    QGraphicsTextItem*    placeholder_ = nullptr;
    QHash<unsigned, Node> nodes_;
    QList<Edge>           edges_;
    QSet<unsigned>        conflictIds_;
    QSet<unsigned>        reduceIds_;
    int                   currentId_     = -1;
    bool                  fullyRevealed_ = false;
    double                zoom_          = 1.0;
};

#endif // AUTOMATONVIEW_H
