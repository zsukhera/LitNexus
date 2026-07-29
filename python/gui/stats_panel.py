"""
stats_panel.py — Character statistics table widget (Module 6).

Displays all characters ranked by weighted degree, with degree and
PageRank columns.  Clicking a row selects the corresponding node in the
graph view via the character_selected signal.
"""

from __future__ import annotations
from PyQt5.QtCore    import Qt, pyqtSignal, QSortFilterProxyModel
from PyQt5.QtGui     import QFont, QColor, QStandardItemModel, QStandardItem
from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QTableView,
    QHeaderView, QAbstractItemView, QSizePolicy, QLineEdit,
)

from .graph_parser import GraphData


PANEL_BG  = "#161b22"
TABLE_BG  = "#0d1117"
BORDER    = "#30363d"
ACCENT    = "#58a6ff"
GOLD      = "#f8d26b"
TEXT      = "#c9d1d9"
MUTED     = "#8b949e"
SEL_BG    = "#1f6feb"
HDR_BG    = "#21262d"


class StatsPanel(QWidget):
    """
    Sortable character statistics table.

    Signals
    -------
    character_selected(str) — emitted when the user clicks a table row
    """

    character_selected = pyqtSignal(str)

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._build_ui()

    # ── UI ────────────────────────────────────────────────────────────────

    def _build_ui(self) -> None:
        self.setStyleSheet(f"background-color: {PANEL_BG}; color: {TEXT};")

        root = QVBoxLayout(self)
        root.setContentsMargins(10, 8, 10, 8)
        root.setSpacing(6)

        # Title row
        title_row = QHBoxLayout()

        title = QLabel("📊  Character Statistics")
        title.setFont(QFont("sans-serif", 11, QFont.Bold))
        title.setStyleSheet(f"color: {ACCENT};")
        title_row.addWidget(title)

        title_row.addStretch()

        # Filter box
        self._filter = QLineEdit()
        self._filter.setPlaceholderText("Filter…")
        self._filter.setMaximumWidth(160)
        self._filter.setStyleSheet(
            f"background-color: {TABLE_BG}; color: {TEXT};"
            f"border: 1px solid {BORDER}; border-radius: 4px; padding: 2px 6px;"
        )
        self._filter.textChanged.connect(self._apply_filter)
        title_row.addWidget(self._filter)

        root.addLayout(title_row)

        # Table model
        self._model = QStandardItemModel(0, 4)
        self._model.setHorizontalHeaderLabels(
            ["Character", "Degree", "Interaction", "PageRank"]
        )

        self._proxy = QSortFilterProxyModel()
        self._proxy.setSourceModel(self._model)
        self._proxy.setFilterCaseSensitivity(Qt.CaseInsensitive)
        self._proxy.setFilterKeyColumn(0)      # filter on name column

        self._table = QTableView()
        self._table.setModel(self._proxy)
        self._table.setSelectionBehavior(QAbstractItemView.SelectRows)
        self._table.setSelectionMode(QAbstractItemView.SingleSelection)
        self._table.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self._table.setSortingEnabled(True)
        self._table.sortByColumn(2, Qt.DescendingOrder)

        self._table.horizontalHeader().setSectionResizeMode(
            0, QHeaderView.Stretch
        )
        for col in (1, 2, 3):
            self._table.horizontalHeader().setSectionResizeMode(
                col, QHeaderView.ResizeToContents
            )

        self._table.verticalHeader().setVisible(False)
        self._table.setShowGrid(False)
        self._table.setAlternatingRowColors(True)

        self._table.setStyleSheet(
            f"QTableView {{"
            f"  background-color: {TABLE_BG}; color: {TEXT};"
            f"  alternate-background-color: #161b22;"
            f"  border: 1px solid {BORDER}; border-radius: 4px;"
            f"}}"
            f"QTableView::item:selected {{"
            f"  background-color: {SEL_BG}; color: white;"
            f"}}"
            f"QHeaderView::section {{"
            f"  background-color: {HDR_BG}; color: {MUTED};"
            f"  border: none; border-bottom: 1px solid {BORDER};"
            f"  padding: 4px;"
            f"}}"
        )

        self._table.clicked.connect(self._on_row_click)
        self._table.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        root.addWidget(self._table)

        # Summary label
        self._summary = QLabel()
        self._summary.setFont(QFont("sans-serif", 9))
        self._summary.setStyleSheet(f"color: {MUTED};")
        root.addWidget(self._summary)

    # ── Public API ────────────────────────────────────────────────────────

    def load_graph(self, data: GraphData) -> None:
        self._model.removeRows(0, self._model.rowCount())

        for name, info in data.sorted_by_weighted_degree():
            def _item(text: str, numeric: bool = False) -> QStandardItem:
                item = QStandardItem()
                if numeric:
                    item.setData(text, Qt.DisplayRole)
                else:
                    item.setText(text)
                item.setTextAlignment(Qt.AlignCenter if numeric else Qt.AlignVCenter | Qt.AlignLeft)
                item.setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled)
                return item

            name_item = _item(name)
            deg_item  = QStandardItem()
            wdeg_item = QStandardItem()
            pr_item   = QStandardItem()

            deg_item.setData(info["degree"],          Qt.DisplayRole)
            wdeg_item.setData(info["weighted_degree"], Qt.DisplayRole)
            pr_item.setData(round(info["pagerank"], 6), Qt.DisplayRole)

            for it in (deg_item, wdeg_item, pr_item):
                it.setTextAlignment(Qt.AlignCenter)
                it.setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled)

            # Colour-code by weighted degree percentile
            max_wd = data.max_weighted_degree() or 1
            heat   = info["weighted_degree"] / max_wd
            r = int(91  + heat * (248 - 91))
            g = int(141 + heat * (168 - 141))
            b = int(232 + heat * (50  - 232))
            name_item.setForeground(QColor(r, g, b))

            self._model.appendRow([name_item, deg_item, wdeg_item, pr_item])

        n = self._model.rowCount()
        e = len(data.edges)
        self._summary.setText(
            f"{n} character(s)  •  {e} relationship(s)"
        )

    def highlight_character(self, name: str) -> None:
        """Select the row for `name` (called from graph view)."""
        for row in range(self._model.rowCount()):
            if self._model.item(row, 0).text() == name:
                proxy_idx = self._proxy.mapFromSource(
                    self._model.index(row, 0)
                )
                self._table.setCurrentIndex(proxy_idx)
                self._table.scrollTo(proxy_idx)
                return

    # ── Slots ─────────────────────────────────────────────────────────────

    def _on_row_click(self, index) -> None:
        source_idx = self._proxy.mapToSource(index)
        name = self._model.item(source_idx.row(), 0).text()
        self.character_selected.emit(name)

    def _apply_filter(self, text: str) -> None:
        self._proxy.setFilterFixedString(text)
