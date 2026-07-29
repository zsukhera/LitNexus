"""
search_panel.py — Search bar + results display widget (Module 6).
"""

from __future__ import annotations
from PyQt5.QtCore    import Qt, pyqtSignal
from PyQt5.QtGui     import QFont, QColor, QPalette
from PyQt5.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLineEdit, QPushButton,
    QLabel, QTextBrowser, QSizePolicy,
)

from .search_engine import SearchEngine, Hit


# ── Colours ──────────────────────────────────────────────────────────────────
PANEL_BG    = "#161b22"
RESULT_BG   = "#0d1117"
BORDER      = "#30363d"
ACCENT      = "#58a6ff"
GOLD        = "#f8d26b"
TEXT        = "#c9d1d9"
MUTED       = "#8b949e"
CHAPTER_HDR = "#388bfd"


class SearchPanel(QWidget):
    """
    Left-side panel: keyword search bar and ranked results.

    Signals
    -------
    search_done(str)  — emitted after a search with the query string
    """

    search_done = pyqtSignal(str)

    def __init__(self, engine: SearchEngine, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._engine = engine
        self._build_ui()

    # ── UI construction ──────────────────────────────────────────────────

    def _build_ui(self) -> None:
        self.setStyleSheet(f"background-color: {PANEL_BG}; color: {TEXT};")

        root = QVBoxLayout(self)
        root.setContentsMargins(10, 10, 10, 10)
        root.setSpacing(8)

        # ── Title ────────────────────────────────────────────────────────
        title = QLabel("🔍  Keyword Search")
        title.setFont(QFont("sans-serif", 12, QFont.Bold))
        title.setStyleSheet(f"color: {ACCENT};")
        root.addWidget(title)

        # ── Search bar ───────────────────────────────────────────────────
        bar_row = QHBoxLayout()
        bar_row.setSpacing(6)

        self._input = QLineEdit()
        self._input.setPlaceholderText("Enter keyword…  (e.g. ghost)")
        self._input.setFont(QFont("monospace", 11))
        self._input.setStyleSheet(
            f"background-color: {RESULT_BG}; color: {TEXT};"
            f"border: 1px solid {BORDER}; border-radius: 4px; padding: 4px 8px;"
        )
        self._input.returnPressed.connect(self._do_search)
        bar_row.addWidget(self._input)

        self._btn = QPushButton("Search")
        self._btn.setFont(QFont("sans-serif", 10, QFont.Bold))
        self._btn.setStyleSheet(
            f"background-color: {ACCENT}; color: #0d1117;"
            f"border-radius: 4px; padding: 4px 14px;"
            "QPushButton:hover { background-color: #79c0ff; }"
        )
        self._btn.clicked.connect(self._do_search)
        bar_row.addWidget(self._btn)

        root.addLayout(bar_row)

        # ── Status line ──────────────────────────────────────────────────
        self._status = QLabel("Index not loaded.")
        self._status.setFont(QFont("sans-serif", 9))
        self._status.setStyleSheet(f"color: {MUTED};")
        root.addWidget(self._status)

        # ── Results pane ─────────────────────────────────────────────────
        self._results = QTextBrowser()
        self._results.setOpenExternalLinks(False)
        self._results.setFont(QFont("monospace", 10))
        self._results.setStyleSheet(
            f"background-color: {RESULT_BG}; color: {TEXT};"
            f"border: 1px solid {BORDER}; border-radius: 4px;"
        )
        self._results.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        root.addWidget(self._results)

    # ── Public API ───────────────────────────────────────────────────────

    def set_index_ready(self, vocab_size: int) -> None:
        self._status.setText(
            f"Index ready — {vocab_size:,} unique terms indexed."
        )
        self._status.setStyleSheet(f"color: {GOLD};")

    def set_index_error(self, msg: str) -> None:
        self._status.setText(f"⚠  {msg}")
        self._status.setStyleSheet("color: #f85149;")

    # ── Search logic ─────────────────────────────────────────────────────

    def _do_search(self) -> None:
        query = self._input.text().strip()
        if not query:
            return

        if not self._engine.is_loaded:
            self._results.setHtml(
                f'<p style="color:#f85149;">Search index not loaded yet.</p>'
            )
            return

        by_chapter = self._engine.search(query)
        total      = self._engine.total_hits(query)
        html       = self._build_html(query, by_chapter, total)
        self._results.setHtml(html)
        self.search_done.emit(query)

    def _build_html(
        self,
        query: str,
        by_chapter: dict[int, list[Hit]],
        total: int,
    ) -> str:
        if not by_chapter:
            sugg = _suggest_close(query)
            html = (
                f'<p style="color:#f85149; font-size:12px;">'
                f'No results found for <b>"{query}"</b>.</p>'
            )
            if sugg:
                html += (
                    f'<p style="color:{MUTED}; font-size:11px;">'
                    f'Did you mean: <i>{sugg}</i>?</p>'
                )
            return html

        parts: list[str] = [
            f'<p style="color:{GOLD}; font-size:12px; margin-bottom:8px;">'
            f'<b>{total:,}</b> occurrence(s) across '
            f'<b>{len(by_chapter)}</b> chapter(s) — '
            f'query: <i>"{query}"</i></p>'
        ]

        SNIPPETS_PER_CHAPTER = 3

        for chap, hits in by_chapter.items():
            count = len(hits)
            parts.append(
                f'<p style="color:{CHAPTER_HDR}; font-size:11px; '
                f'margin-top:10px; margin-bottom:4px;">'
                f'<b>Chapter {chap}</b>'
                f'  <span style="color:{MUTED};">({count} hit'
                f'{"s" if count != 1 else ""})</span></p>'
                f'<hr style="border:none; border-top:1px solid {BORDER};">'
            )

            shown = 0
            for hit in hits:
                if shown >= SNIPPETS_PER_CHAPTER:
                    remaining = count - shown
                    parts.append(
                        f'<p style="color:{MUTED}; font-size:10px; '
                        f'margin-left:8px;">… and {remaining} more '
                        f'occurrence(s).</p>'
                    )
                    break

                snippet = SearchEngine.highlight(hit.line_text, query)
                # Style the «highlight» markers
                snippet_html = (
                    snippet
                    .replace("&", "&amp;")
                    .replace("<", "&lt;")
                    .replace(">", "&gt;")
                    .replace("«", f'<span style="color:{GOLD}; '
                                  f'font-weight:bold;">')
                    .replace("»", "</span>")
                )
                parts.append(
                    f'<p style="font-size:10px; margin:2px 0 2px 8px;">'
                    f'<span style="color:{MUTED};">line {hit.line_num:>5}:</span> '
                    f'{snippet_html}</p>'
                )
                shown += 1

        return "".join(parts)


def _suggest_close(query: str) -> str:
    """Trivial suggestions — just show common alternatives."""
    # Real prefix suggestions need access to the Trie; kept lightweight here.
    return ""
