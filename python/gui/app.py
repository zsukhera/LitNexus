"""
app.py — LitNexus GUI entry point.

Run from the project root:
    python python/gui/app.py

Or via the provided launcher:
    ./run_gui.sh
"""

from __future__ import annotations
import os
import sys
from pathlib import Path

# ── WSL / headless guard ─────────────────────────────────────────────────────
# Must happen before any Qt import.

def _configure_display() -> None:
    """
    Ensure a DISPLAY or WAYLAND_DISPLAY variable is set when running under
    WSL without WSLg.  Exits with a friendly message if no display can be
    found (e.g. a pure headless CI environment).
    """
    # WSLg sets WAYLAND_DISPLAY automatically — nothing to do.
    if os.environ.get("WAYLAND_DISPLAY"):
        return

    # Already set (X11 forwarding or native Linux desktop).
    if os.environ.get("DISPLAY"):
        return

    # Try to detect the Windows host IP from /etc/resolv.conf (WSL1/2).
    resolv = Path("/etc/resolv.conf")
    if resolv.exists():
        for line in resolv.read_text().splitlines():
            if line.startswith("nameserver"):
                host_ip = line.split()[1]
                os.environ["DISPLAY"] = f"{host_ip}:0.0"
                print(f"[LitNexus] WSL: set DISPLAY={os.environ['DISPLAY']}")
                return

    # Last resort: assume a local X server.
    os.environ.setdefault("DISPLAY", ":0")
    print("[LitNexus] Could not detect display; defaulting to DISPLAY=:0")


_configure_display()

# ── Qt imports ───────────────────────────────────────────────────────────────

try:
    from PyQt5.QtWidgets import QApplication, QSplashScreen, QMessageBox
    from PyQt5.QtCore    import Qt
    from PyQt5.QtGui     import QPixmap, QColor, QPainter, QFont
except ImportError:
    print(
        "\n[LitNexus] PyQt5 is not installed.\n"
        "  Activate your virtual environment and run:\n"
        "      pip install PyQt5>=5.15\n"
    )
    sys.exit(1)

# ── Path helpers ─────────────────────────────────────────────────────────────

# This file lives at  <project_root>/python/gui/app.py
# so the project root is two levels up.
_HERE        = Path(__file__).resolve().parent          # python/gui/
_PYTHON_DIR  = _HERE.parent                             # python/
_PROJECT_ROOT = _PYTHON_DIR.parent                      # project root

# Default data locations (matches the existing project structure)
_DEFAULT_GRAPH    = _PROJECT_ROOT / "data" / "processed" / "graph.txt"
_DEFAULT_CHAPTERS = _PROJECT_ROOT / "data" / "processed" / "chapters"

# Add python/ to sys.path so that existing modules (config, utils…) remain
# importable.
if str(_PYTHON_DIR) not in sys.path:
    sys.path.insert(0, str(_PYTHON_DIR))


# ── Splash screen ─────────────────────────────────────────────────────────────

def _make_splash() -> QSplashScreen:
    px = QPixmap(480, 200)
    px.fill(QColor("#0d1117"))

    p = QPainter(px)
    p.setRenderHint(QPainter.Antialiasing)

    p.setFont(QFont("sans-serif", 28, QFont.Bold))
    p.setPen(QColor("#58a6ff"))
    p.drawText(px.rect(), Qt.AlignHCenter | Qt.AlignTop + 30, "\n\nLitNexus")

    p.setFont(QFont("sans-serif", 12))
    p.setPen(QColor("#8b949e"))
    p.drawText(px.rect(), Qt.AlignHCenter | Qt.AlignBottom - 20,
               "Loading…\n")
    p.end()

    splash = QSplashScreen(px, Qt.WindowStaysOnTopHint)
    return splash


# ── Main ─────────────────────────────────────────────────────────────────────

def main() -> None:
    # High-DPI support
    QApplication.setAttribute(Qt.AA_EnableHighDpiScaling, True)
    QApplication.setAttribute(Qt.AA_UseHighDpiPixmaps,    True)

    app = QApplication(sys.argv)
    app.setApplicationName("LitNexus")
    app.setOrganizationName("LitNexus")

    # Splash
    splash = _make_splash()
    splash.show()
    app.processEvents()

    # Lazy import after QApplication exists.
    # Works whether launched as `python -m gui` or `python -m gui.app`.
    try:
        from .main_window import MainWindow
    except ImportError:
        from gui.main_window import MainWindow

    window = MainWindow()

    # ── Auto-load default data ────────────────────────────────────────────
    # Graph file
    graph_path = _DEFAULT_GRAPH

    # Accept an optional CLI argument: python app.py /path/to/graph.txt
    if len(sys.argv) >= 2 and Path(sys.argv[1]).exists():
        graph_path = Path(sys.argv[1])

    if graph_path.exists():
        splash.showMessage(
            f"  Loading graph from {graph_path.name}…",
            Qt.AlignLeft | Qt.AlignBottom,
            QColor("#8b949e"),
        )
        app.processEvents()
        window.load_graph(graph_path)
    else:
        print(
            f"[LitNexus] Graph file not found at {graph_path}.\n"
            "  Use  File → Open Graph…  to load one manually."
        )

    # Chapters folder (search index — built in background)
    if _DEFAULT_CHAPTERS.is_dir():
        splash.showMessage(
            "  Building search index…",
            Qt.AlignLeft | Qt.AlignBottom,
            QColor("#8b949e"),
        )
        app.processEvents()
        window.load_chapters(_DEFAULT_CHAPTERS)
    else:
        print(
            f"[LitNexus] Chapters folder not found at {_DEFAULT_CHAPTERS}.\n"
            "  Use  File → Load Chapters Folder…  to point to it manually."
        )

    window.show()
    splash.finish(window)

    sys.exit(app.exec_())


if __name__ == "__main__":
    main()