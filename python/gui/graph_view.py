"""
graph_view.py — Module 7: Interactive character graph visualization.

Rendering:
  • Node size   ∝ weighted degree
  • Edge width  ∝ co-occurrence weight (capped)
  • Node colour ∝ weighted degree (cool → warm gradient)

Interaction:
  • Mouse-wheel  — zoom in / out
  • Left-drag    — pan
  • Left-click   — select node, highlight its edges
  • Hover        — tooltip with character stats
  • Double-click — emit signal for stats panel focus
"""

from __future__ import annotations
import math
import random
from typing import Optional

from PyQt5.QtCore  import Qt, QPointF, QRectF, pyqtSignal, QTimer
from PyQt5.QtGui   import (
    QPainter, QColor, QPen, QBrush, QFont, QFontMetricsF,
    QLinearGradient, QRadialGradient, QPainterPath,
)
from PyQt5.QtWidgets import QWidget, QToolTip, QApplication

from .graph_parser import GraphData


# ── Palette ──────────────────────────────────────────────────────────────────

BG_COLOR        = QColor("#0d1117")
EDGE_BASE       = QColor(80, 100, 130, 80)
EDGE_HIGHLIGHT  = QColor(248, 210, 107, 220)
EDGE_NEIGHBOR   = QColor(139, 180, 250, 180)
NODE_SELECTED   = QColor("#f8d26b")
NODE_HOVER      = QColor("#ffffff")
LABEL_COLOR     = QColor("#e6edf3")
LABEL_SHADOW    = QColor(0, 0, 0, 180)


def _degree_color(heat: float) -> QColor:
    """Map heat ∈ [0, 1] → colour from cool-blue to warm-red via gold."""
    # blue #5b8de8 → gold #f8a832 → red #e84040
    if heat < 0.5:
        t = heat * 2.0
        r = int(91  + t * (248 - 91))
        g = int(141 + t * (168 - 141))
        b = int(232 + t * (50  - 232))
    else:
        t = (heat - 0.5) * 2.0
        r = int(248 + t * (232 - 248))
        g = int(168 + t * (64  - 168))
        b = int(50  + t * (64  - 50))
    return QColor(r, g, b)


# ── GraphView ────────────────────────────────────────────────────────────────

class GraphView(QWidget):
    """
    Interactive force-directed graph widget.

    Signals
    -------
    node_selected(str)   — emitted when the user clicks a node
    node_focused(str)    — emitted on double-click (triggers stats focus)
    """

    node_selected = pyqtSignal(str)
    node_focused  = pyqtSignal(str)

    # ── construction ─────────────────────────────────────────────────────

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)

        self._data:          Optional[GraphData] = None
        self._positions:     dict[str, QPointF]  = {}   # graph space
        self._scale:         float = 1.0
        self._offset:        QPointF = QPointF(0, 0)
        self._selected:      Optional[str] = None
        self._hovered:       Optional[str] = None
        self._pan_origin:    Optional[QPointF] = None
        self._layout_mode:   str = "force"

        self.setMouseTracking(True)
        self.setMinimumSize(500, 400)
        self.setFocusPolicy(Qt.StrongFocus)
        self.setAttribute(Qt.WA_OpaquePaintEvent)

        # Defer layout so the widget has a real size first
        self._layout_timer = QTimer(self)
        self._layout_timer.setSingleShot(True)
        self._layout_timer.timeout.connect(self._run_layout)

    # ── public API ───────────────────────────────────────────────────────

    def load_graph(self, data: GraphData) -> None:
        self._data     = data
        self._selected = None
        self._hovered  = None
        self._offset   = QPointF(0, 0)
        self._scale    = 1.0
        self._layout_timer.start(50)

    def set_layout(self, mode: str) -> None:
        """'force' | 'circular'"""
        self._layout_mode = mode
        if self._data:
            self._run_layout()

    def select_node(self, name: str) -> None:
        if name in (self._data.nodes if self._data else {}):
            self._selected = name
            self.update()

    def reset_view(self) -> None:
        self._offset = QPointF(0, 0)
        self._scale  = 1.0
        self.update()

    # ── layout algorithms ────────────────────────────────────────────────

    def _run_layout(self) -> None:
        if not self._data or not self._data.nodes:
            return

        if self._layout_mode == "circular":
            self._circular_layout()
        else:
            self._force_layout()

        self.update()

    def _circular_layout(self) -> None:
        names  = list(self._data.nodes.keys())
        n      = len(names)
        radius = 220.0

        for i, name in enumerate(names):
            angle = 2 * math.pi * i / n - math.pi / 2
            self._positions[name] = QPointF(
                radius * math.cos(angle),
                radius * math.sin(angle),
            )

    def _force_layout(self, iterations: int = 180) -> None:
        """Fruchterman-Reingold force-directed placement."""
        names = list(self._data.nodes.keys())
        n     = len(names)
        if n == 0:
            return

        # Reproducible initial positions seeded by name
        rng = random.Random(42)
        pos = {
            name: [rng.uniform(-200, 200), rng.uniform(-200, 200)]
            for name in names
        }

        W = H = 450.0            # virtual canvas
        k = math.sqrt(W * H / n) # optimal distance
        max_w = self._data.max_weight() or 1

        for t_idx in range(iterations):
            temp = W / 10.0 * (1.0 - t_idx / iterations) + 0.5

            disp = {name: [0.0, 0.0] for name in names}

            # Repulsive forces between every pair
            for i in range(n):
                vi = names[i]
                for j in range(i + 1, n):
                    vj = names[j]
                    dx = pos[vi][0] - pos[vj][0]
                    dy = pos[vi][1] - pos[vj][1]
                    dist = max(math.hypot(dx, dy), 0.01)
                    force = k * k / dist
                    fx, fy = (dx / dist) * force, (dy / dist) * force
                    disp[vi][0] += fx;  disp[vi][1] += fy
                    disp[vj][0] -= fx;  disp[vj][1] -= fy

            # Attractive forces along edges (weighted)
            for a, b, w in self._data.edges:
                if a not in pos or b not in pos:
                    continue
                weight_factor = 1.0 + (w / max_w) * 1.5
                dx = pos[a][0] - pos[b][0]
                dy = pos[a][1] - pos[b][1]
                dist = max(math.hypot(dx, dy), 0.01)
                force = dist * dist / k * weight_factor
                fx, fy = (dx / dist) * force, (dy / dist) * force
                disp[a][0] -= fx;  disp[a][1] -= fy
                disp[b][0] += fx;  disp[b][1] += fy

            # Apply displacements, capped by temperature
            for name in names:
                d = disp[name]
                length = max(math.hypot(d[0], d[1]), 0.01)
                scale  = min(length, temp) / length
                pos[name][0] += d[0] * scale
                pos[name][1] += d[1] * scale
                # Soft clamping
                pos[name][0] = max(-W / 2, min(W / 2, pos[name][0]))
                pos[name][1] = max(-H / 2, min(H / 2, pos[name][1]))

        self._positions = {
            name: QPointF(p[0], p[1]) for name, p in pos.items()
        }

    # ── coordinate helpers ───────────────────────────────────────────────

    def _to_screen(self, gp: QPointF) -> QPointF:
        cx = self.width()  / 2 + self._offset.x()
        cy = self.height() / 2 + self._offset.y()
        return QPointF(cx + gp.x() * self._scale,
                       cy + gp.y() * self._scale)

    def _to_graph(self, sp: QPointF) -> QPointF:
        cx = self.width()  / 2 + self._offset.x()
        cy = self.height() / 2 + self._offset.y()
        return QPointF((sp.x() - cx) / self._scale,
                       (sp.y() - cy) / self._scale)

    def _node_radius(self, name: str) -> float:
        if not self._data:
            return 10.0
        wd     = self._data.nodes[name]["weighted_degree"]
        max_wd = self._data.max_weighted_degree() or 1
        return 7.0 + (wd / max_wd) * 20.0

    def _node_at(self, screen_pos: QPointF) -> Optional[str]:
        if not self._data:
            return None
        for name, gp in self._positions.items():
            sp = self._to_screen(gp)
            r  = self._node_radius(name) * self._scale
            dx = screen_pos.x() - sp.x()
            dy = screen_pos.y() - sp.y()
            if dx * dx + dy * dy <= (r + 4) ** 2:
                return name
        return None

    # ── painting ─────────────────────────────────────────────────────────

    def paintEvent(self, _event) -> None:
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        painter.fillRect(self.rect(), BG_COLOR)

        if not self._data or not self._positions:
            painter.setPen(QColor("#8b949e"))
            painter.setFont(QFont("sans-serif", 14))
            painter.drawText(self.rect(), Qt.AlignCenter,
                             "No graph loaded.\nUse  File → Open Graph…")
            return

        self._paint_edges(painter)
        self._paint_nodes(painter)
        self._paint_legend(painter)

    def _paint_edges(self, painter: QPainter) -> None:
        max_w  = self._data.max_weight() or 1
        sel_nb = self._data.neighbors_of(self._selected) if self._selected else set()

        for a, b, w in self._data.edges:
            if a not in self._positions or b not in self._positions:
                continue

            pa = self._to_screen(self._positions[a])
            pb = self._to_screen(self._positions[b])

            # Determine colour and opacity
            is_sel_edge = (
                self._selected and
                (a == self._selected or b == self._selected)
            )

            if self._selected:
                if is_sel_edge:
                    color = EDGE_HIGHLIGHT
                else:
                    color = QColor(60, 80, 110, 40)
            else:
                alpha = int(60 + (w / max_w) * 120)
                color = QColor(100, 140, 200, alpha)

            width = max(0.8, (w / max_w) * 4.0 * self._scale)
            pen   = QPen(color, width)
            pen.setCapStyle(Qt.RoundCap)
            painter.setPen(pen)
            painter.drawLine(pa, pb)

    def _paint_nodes(self, painter: QPainter) -> None:
        max_wd = self._data.max_weighted_degree() or 1
        sel_nb = self._data.neighbors_of(self._selected) if self._selected else set()

        font = QFont("sans-serif", max(7, int(9 * self._scale)))
        painter.setFont(font)
        fm = QFontMetricsF(font)

        # Draw all nodes (selected / hovered last so they appear on top)
        draw_order = sorted(
            self._positions.keys(),
            key=lambda n: (
                n == self._selected,
                n == self._hovered,
                n in sel_nb,
            ),
        )

        for name in draw_order:
            gp   = self._positions[name]
            sp   = self._to_screen(gp)
            r    = self._node_radius(name) * self._scale
            heat = self._data.nodes[name]["weighted_degree"] / max_wd

            # Determine fill colour
            if name == self._selected:
                fill = NODE_SELECTED
            elif name == self._hovered:
                fill = NODE_HOVER
            elif self._selected and name not in sel_nb:
                fill = QColor(50, 60, 80, 120)
            else:
                fill = _degree_color(heat)

            # Glow for selected / hovered
            if name in (self._selected, self._hovered):
                glow = QRadialGradient(sp, r * 2.2)
                glow.setColorAt(0.0, QColor(fill.red(), fill.green(),
                                             fill.blue(), 80))
                glow.setColorAt(1.0, QColor(0, 0, 0, 0))
                painter.setBrush(QBrush(glow))
                painter.setPen(Qt.NoPen)
                painter.drawEllipse(sp, r * 2.2, r * 2.2)

            # Node circle
            painter.setBrush(QBrush(fill))
            border_color = (
                NODE_SELECTED if name == self._selected else
                NODE_HOVER    if name == self._hovered   else
                fill.lighter(130)
            )
            painter.setPen(QPen(border_color, 1.5))
            painter.drawEllipse(sp, r, r)

            # Label — only show if scale is reasonable
            if self._scale >= 0.55 or name in (self._selected, self._hovered):
                label = name
                tw    = fm.horizontalAdvance(label)
                tx    = sp.x() - tw / 2
                ty    = sp.y() + r + fm.ascent() + 2 * self._scale

                # Shadow
                painter.setPen(LABEL_SHADOW)
                painter.drawText(QPointF(tx + 1, ty + 1), label)
                # Text
                lc = (NODE_SELECTED if name == self._selected else
                      NODE_HOVER    if name == self._hovered   else
                      LABEL_COLOR)
                painter.setPen(lc)
                painter.drawText(QPointF(tx, ty), label)

    def _paint_legend(self, painter: QPainter) -> None:
        if not self._data:
            return
        painter.setFont(QFont("sans-serif", 8))
        painter.setPen(QColor("#8b949e"))
        painter.drawText(
            QRectF(8, self.height() - 20, 300, 16),
            Qt.AlignLeft | Qt.AlignVCenter,
            "Scroll to zoom  •  Drag to pan  •  Click node to select",
        )

    # ── interaction ──────────────────────────────────────────────────────

    def mousePressEvent(self, event) -> None:
        if event.button() == Qt.LeftButton:
            hit = self._node_at(event.pos())
            if hit:
                self._selected = hit
                self.node_selected.emit(hit)
            else:
                self._pan_origin = QPointF(event.pos())
            self.update()

    def mouseDoubleClickEvent(self, event) -> None:
        if event.button() == Qt.LeftButton:
            hit = self._node_at(event.pos())
            if hit:
                self.node_focused.emit(hit)

    def mouseMoveEvent(self, event) -> None:
        pos = QPointF(event.pos())

        if self._pan_origin is not None:
            delta = pos - self._pan_origin
            self._offset += delta
            self._pan_origin = pos
            self.update()
        else:
            prev = self._hovered
            self._hovered = self._node_at(pos)
            if self._hovered != prev:
                self.update()
            if self._hovered and self._data:
                nd = self._data.nodes[self._hovered]
                QToolTip.showText(
                    event.globalPos(),
                    f"<b>{self._hovered}</b><br>"
                    f"Connections : {nd['degree']}<br>"
                    f"Interaction : {nd['weighted_degree']}<br>"
                    f"PageRank    : {nd['pagerank']:.4f}",
                    self,
                )
            else:
                QToolTip.hideText()

    def mouseReleaseEvent(self, event) -> None:
        if event.button() == Qt.LeftButton:
            self._pan_origin = None

    def wheelEvent(self, event) -> None:
        factor = 1.15 if event.angleDelta().y() > 0 else 1.0 / 1.15
        self._scale = max(0.1, min(12.0, self._scale * factor))
        self.update()

    def resizeEvent(self, event) -> None:
        super().resizeEvent(event)
        if self._data and not self._positions:
            self._run_layout()
