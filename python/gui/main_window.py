"""
main_window.py — Module 6: LitNexus main application window.

Layout
──────
  ┌─ MenuBar ────────────────────────────────────────────────────────────┐
  │  File  |  View  |  Help                                              │
  ├─ ToolBar ────────────────────────────────────────────────────────────┤
  │  [Force Layout]  [Circular Layout]  [Reset View]  |  status label   │
  ├─ Left panel ──────────────┬─ Right panel (graph) ────────────────────┤
  │                           │                                          │
  │  ┌─ Tabs ──────────────┐  │   GraphView (Module 7)                   │
  │  │  Search | Stats     │  │                                          │
  │  └─────────────────────┘  │                                          │
  │                           │                                          │
  ├─ StatusBar ───────────────────────────────────────────────────────────┤
  └──────────────────────────────────────────────────────────────────────┘
"""

from __future__ import annotations
from pathlib import Path

from PyQt5.QtCore    import Qt, QThread, pyqtSignal, QObject
from PyQt5.QtGui     import QFont, QIcon, QKeySequence
from PyQt5.QtWidgets import (
    QMainWindow, QWidget, QSplitter, QTabWidget, QVBoxLayout,
    QHBoxLayout, QLabel, QPushButton, QToolBar, QAction,
    QFileDialog, QMessageBox, QApplication, QStatusBar,
    QSizePolicy,
)

from .graph_parser  import GraphData
from .search_engine import SearchEngine
from .graph_view    import GraphView
from .search_panel  import SearchPanel
from .stats_panel   import StatsPanel


WIN_BG  = "#0d1117"
ACCENT  = "#58a6ff"
TEXT    = "#c9d1d9"
MUTED   = "#8b949e"
GOLD    = "#f8d26b"


# ── Background worker for index building ─────────────────────────────────────

class _IndexWorker(QObject):
    finished = pyqtSignal(bool, str)     # success, message

    def __init__(self, engine: SearchEngine, folder: str) -> None:
        super().__init__()
        self._engine = engine
        self._folder = folder

    def run(self) -> None:
        try:
            self._engine.build_index(self._folder)
            self.finished.emit(True, f"{self._engine.vocab_size:,} terms indexed")
        except Exception as exc:
            self.finished.emit(False, str(exc))


# ── MainWindow ───────────────────────────────────────────────────────────────

class MainWindow(QMainWindow):
    """LitNexus main application window."""

    def __init__(self) -> None:
        super().__init__()

        self._graph_data = GraphData()
        self._engine     = SearchEngine()
        self._index_thread: QThread | None = None

        self._setup_window()
        self._build_menus()
        self._build_toolbar()
        self._build_central()
        self._build_statusbar()
        self._connect_signals()

        self.setStyleSheet(
            f"QMainWindow {{ background-color: {WIN_BG}; }}"
            f"QMenuBar {{ background-color: #161b22; color: {TEXT}; "
            f"           border-bottom: 1px solid #30363d; }}"
            f"QMenuBar::item:selected {{ background-color: #21262d; }}"
            f"QMenu {{ background-color: #161b22; color: {TEXT}; "
            f"         border: 1px solid #30363d; }}"
            f"QMenu::item:selected {{ background-color: #1f6feb; }}"
            f"QToolBar {{ background-color: #161b22; "
            f"            border-bottom: 1px solid #30363d; spacing: 4px; }}"
            f"QTabWidget::pane {{ border: 1px solid #30363d; "
            f"                    background-color: #161b22; }}"
            f"QTabBar::tab {{ background-color: #21262d; color: {MUTED}; "
            f"               padding: 6px 14px; border: none; }}"
            f"QTabBar::tab:selected {{ background-color: #161b22; "
            f"                        color: {ACCENT}; "
            f"                        border-top: 2px solid {ACCENT}; }}"
            f"QSplitter::handle {{ background-color: #30363d; width: 2px; }}"
            f"QStatusBar {{ background-color: #161b22; color: {MUTED}; "
            f"             font-size: 10px; }}"
            f"QToolTip {{ background-color: #1c2128; color: {TEXT}; "
            f"           border: 1px solid #30363d; padding: 4px; }}"
        )

    # ── Window setup ─────────────────────────────────────────────────────

    def _setup_window(self) -> None:
        self.setWindowTitle("LitNexus — Literary Analysis Engine")
        self.resize(1300, 820)
        self.setMinimumSize(900, 600)

    # ── Menu bar ─────────────────────────────────────────────────────────

    def _build_menus(self) -> None:
        mb = self.menuBar()

        # File
        file_menu = mb.addMenu("&File")

        open_graph_act = QAction("&Open Graph…", self)
        open_graph_act.setShortcut(QKeySequence.Open)
        open_graph_act.setStatusTip("Load a graph.txt file")
        open_graph_act.triggered.connect(self._open_graph_dialog)
        file_menu.addAction(open_graph_act)

        open_chapters_act = QAction("Load &Chapters Folder…", self)
        open_chapters_act.setStatusTip(
            "Select the folder containing chapter_XX.txt files for search"
        )
        open_chapters_act.triggered.connect(self._open_chapters_dialog)
        file_menu.addAction(open_chapters_act)

        file_menu.addSeparator()

        quit_act = QAction("&Quit", self)
        quit_act.setShortcut(QKeySequence.Quit)
        quit_act.triggered.connect(QApplication.quit)
        file_menu.addAction(quit_act)

        # View
        view_menu = mb.addMenu("&View")

        force_act = QAction("&Force-Directed Layout", self)
        force_act.triggered.connect(lambda: self._set_layout("force"))
        view_menu.addAction(force_act)

        circ_act = QAction("&Circular Layout", self)
        circ_act.triggered.connect(lambda: self._set_layout("circular"))
        view_menu.addAction(circ_act)

        view_menu.addSeparator()

        reset_act = QAction("&Reset View", self)
        reset_act.setShortcut("Ctrl+0")
        reset_act.triggered.connect(self._graph_view.reset_view
                                    if hasattr(self, "_graph_view") else
                                    lambda: None)
        view_menu.addAction(reset_act)

        # Help
        help_menu = mb.addMenu("&Help")
        about_act = QAction("&About LitNexus", self)
        about_act.triggered.connect(self._show_about)
        help_menu.addAction(about_act)

    # ── Toolbar ──────────────────────────────────────────────────────────

    def _build_toolbar(self) -> None:
        tb = QToolBar("Main Toolbar", self)
        tb.setMovable(False)
        tb.setToolButtonStyle(Qt.ToolButtonTextBesideIcon)
        self.addToolBar(tb)

        def _btn(label: str, tip: str, slot) -> QPushButton:
            b = QPushButton(label)
            b.setStatusTip(tip)
            b.setFont(QFont("sans-serif", 9))
            b.setStyleSheet(
                f"QPushButton {{ background-color: #21262d; color: {TEXT};"
                f"  border: 1px solid #30363d; border-radius: 4px;"
                f"  padding: 3px 10px; }}"
                f"QPushButton:hover {{ background-color: #30363d; }}"
            )
            b.clicked.connect(slot)
            return b

        tb.addWidget(_btn("⛶  Force Layout",    "Switch to force-directed layout",
                           lambda: self._set_layout("force")))
        tb.addWidget(_btn("◎  Circular Layout", "Switch to circular layout",
                           lambda: self._set_layout("circular")))
        tb.addWidget(_btn("⟳  Rerun Layout",    "Re-randomise and rerun layout",
                           self._rerun_layout))

        sep = QWidget()
        sep.setFixedWidth(8)
        tb.addWidget(sep)

        tb.addWidget(_btn("⊡  Reset View",      "Reset zoom and pan",
                           lambda: self._graph_view.reset_view()))

        # Spacer
        spacer = QWidget()
        spacer.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        tb.addWidget(spacer)

        self._toolbar_status = QLabel("No graph loaded")
        self._toolbar_status.setStyleSheet(f"color: {MUTED}; font-size: 10px;")
        tb.addWidget(self._toolbar_status)

    # ── Central widget ───────────────────────────────────────────────────

    def _build_central(self) -> None:
        splitter = QSplitter(Qt.Horizontal)
        splitter.setHandleWidth(2)

        # ── Left panel ───────────────────────────────────────────────────
        left = QWidget()
        left.setMinimumWidth(340)
        left.setMaximumWidth(500)
        left_layout = QVBoxLayout(left)
        left_layout.setContentsMargins(0, 0, 0, 0)

        tabs = QTabWidget()
        tabs.setFont(QFont("sans-serif", 10))

        self._search_panel = SearchPanel(self._engine)
        tabs.addTab(self._search_panel, "🔍  Search")

        self._stats_panel = StatsPanel()
        tabs.addTab(self._stats_panel, "📊  Statistics")

        left_layout.addWidget(tabs)
        splitter.addWidget(left)

        # ── Right panel (graph) ──────────────────────────────────────────
        self._graph_view = GraphView()
        splitter.addWidget(self._graph_view)

        splitter.setStretchFactor(0, 0)
        splitter.setStretchFactor(1, 1)
        splitter.setSizes([380, 900])

        self.setCentralWidget(splitter)

    # ── Status bar ───────────────────────────────────────────────────────

    def _build_statusbar(self) -> None:
        sb = QStatusBar()
        self.setStatusBar(sb)
        self._status_left  = QLabel("Ready")
        self._status_right = QLabel()
        self._status_right.setAlignment(Qt.AlignRight)
        sb.addWidget(self._status_left,  1)
        sb.addPermanentWidget(self._status_right)

    # ── Signal wiring ────────────────────────────────────────────────────

    def _connect_signals(self) -> None:
        # Graph view ↔ stats panel (bidirectional highlight)
        self._graph_view.node_selected.connect(self._on_node_selected)
        self._graph_view.node_focused.connect(self._on_node_focused)
        self._stats_panel.character_selected.connect(
            self._graph_view.select_node
        )

        # Search done → status update
        self._search_panel.search_done.connect(
            lambda q: self._status_left.setText(f"Search: '{q}'")
        )

    # ── Public loaders ───────────────────────────────────────────────────

    def load_graph(self, filepath: str | Path) -> None:
        try:
            self._graph_data.load_from_file(filepath)
        except Exception as exc:
            QMessageBox.critical(self, "Graph load error", str(exc))
            return

        n = len(self._graph_data.nodes)
        e = len(self._graph_data.edges)
        self._graph_view.load_graph(self._graph_data)
        self._stats_panel.load_graph(self._graph_data)
        self._toolbar_status.setText(
            f"{n} characters  •  {e} relationships"
        )
        self._toolbar_status.setStyleSheet(f"color: {GOLD}; font-size: 10px;")
        self._status_left.setText(f"Graph loaded from {Path(filepath).name}")

    def load_chapters(self, folder: str | Path) -> None:
        """Build the search index in a background thread."""
        if self._index_thread and self._index_thread.isRunning():
            return

        self._status_left.setText("Building search index…")
        self._search_panel.set_index_error("Indexing…")

        self._index_thread  = QThread()
        worker              = _IndexWorker(self._engine, str(folder))
        worker.moveToThread(self._index_thread)

        self._index_thread.started.connect(worker.run)
        worker.finished.connect(self._on_index_done)
        worker.finished.connect(self._index_thread.quit)

        self._index_thread.start()
        self._worker_ref = worker          # prevent GC

    # ── Slots ────────────────────────────────────────────────────────────

    def _on_index_done(self, success: bool, msg: str) -> None:
        if success:
            self._search_panel.set_index_ready(self._engine.vocab_size)
            self._status_left.setText(f"Search index ready — {msg}")
        else:
            self._search_panel.set_index_error(msg)
            self._status_left.setText(f"Index error: {msg}")

    def _on_node_selected(self, name: str) -> None:
        self._stats_panel.highlight_character(name)
        nd = self._graph_data.nodes.get(name, {})
        self._status_right.setText(
            f"{name}  |  connections: {nd.get('degree','?')}  "
            f"|  interaction: {nd.get('weighted_degree','?')}  "
            f"|  PageRank: {nd.get('pagerank', 0):.4f}"
        )

    def _on_node_focused(self, name: str) -> None:
        """Double-click — switch to stats tab and highlight the row."""
        self._stats_panel.highlight_character(name)
        # Find the stats tab index (tab 1)
        tab_widget = self._stats_panel.parentWidget()
        if isinstance(tab_widget, QTabWidget):
            tab_widget.setCurrentIndex(1)

    def _set_layout(self, mode: str) -> None:
        self._graph_view.set_layout(mode)
        self._status_left.setText(
            f"Layout: {'force-directed' if mode == 'force' else 'circular'}"
        )

    def _rerun_layout(self) -> None:
        import random as _r
        _r.seed()                           # re-randomise seed
        self._graph_view._positions.clear()
        self._graph_view._run_layout()

    def _open_graph_dialog(self) -> None:
        path, _ = QFileDialog.getOpenFileName(
            self, "Open Graph File", "",
            "Text Files (*.txt);;All Files (*)"
        )
        if path:
            self.load_graph(path)

    def _open_chapters_dialog(self) -> None:
        folder = QFileDialog.getExistingDirectory(
            self, "Select Chapters Folder"
        )
        if folder:
            self.load_chapters(folder)

    def _show_about(self) -> None:
        QMessageBox.about(
            self, "About LitNexus",
            "<b>LitNexus</b><br>"
            "A Graph-Based Literary Analysis Engine<br><br>"
            "Modules 6 &amp; 7 — Desktop GUI &amp; Graph Visualization<br>"
            "<br>"
            "Analyses <i>Wuthering Heights</i> character relationships<br>"
            "using weighted graph theory and PageRank.<br><br>"
            "<small>Built with Python + PyQt5</small>",
        )
