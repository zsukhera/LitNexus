"""
search_engine.py — Python inverted-index search engine.

Reads all chapter_XX.txt files from the chapters folder and builds a
word-level inverted index.  Mirrors the C++ mod6.h implementation.
"""

from __future__ import annotations
from pathlib import Path
import re
from dataclasses import dataclass, field


@dataclass
class Hit:
    chapter:   int
    line_num:  int
    line_text: str


class SearchEngine:
    """Inverted-index keyword search over the novel chapters."""

    def __init__(self) -> None:
        # lowercased word → list of Hit
        self._index: dict[str, list[Hit]] = {}
        self._loaded = False

    # ------------------------------------------------------------------ #
    #  Building the index                                                  #
    # ------------------------------------------------------------------ #

    def build_index(self, chapters_folder: str | Path) -> None:
        folder = Path(chapters_folder)
        if not folder.is_dir():
            raise FileNotFoundError(f"Chapters folder not found: {folder}")

        self._index.clear()
        strip_re = re.compile(r"^[^\w]+|[^\w]+$")

        files = sorted(folder.glob("chapter_*.txt"))

        for path in files:
            # Extract chapter number: "chapter_03.txt" → 3
            try:
                chapter_num = int(re.search(r"(\d+)", path.stem).group(1))
            except (AttributeError, ValueError):
                continue

            with open(path, encoding="utf-8", errors="ignore") as fh:
                for line_num, raw_line in enumerate(fh, start=1):
                    line = raw_line.rstrip()
                    if not line:
                        continue

                    for token in line.split():
                        word = strip_re.sub("", token).lower()
                        if not word:
                            continue

                        self._index.setdefault(word, []).append(
                            Hit(chapter_num, line_num, line)
                        )

        self._loaded = True

    # ------------------------------------------------------------------ #
    #  Searching                                                           #
    # ------------------------------------------------------------------ #

    def search(self, query: str) -> dict[int, list[Hit]]:
        """
        Return a dict mapping chapter_number → list[Hit] for `query`.
        Chapters are ordered by descending hit count (most hits first).
        Returns an empty dict if not found.
        """
        if not self._loaded:
            return {}

        key  = query.strip().lower()
        hits = self._index.get(key, [])

        # Group by chapter
        by_chapter: dict[int, list[Hit]] = {}
        for hit in hits:
            by_chapter.setdefault(hit.chapter, []).append(hit)

        # Sort chapters: most hits first, tie-break by chapter number
        return dict(
            sorted(
                by_chapter.items(),
                key=lambda kv: (-len(kv[1]), kv[0]),
            )
        )

    def total_hits(self, query: str) -> int:
        return len(self._index.get(query.strip().lower(), []))

    @property
    def is_loaded(self) -> bool:
        return self._loaded

    @property
    def vocab_size(self) -> int:
        return len(self._index)

    # ------------------------------------------------------------------ #
    #  Snippet helper                                                      #
    # ------------------------------------------------------------------ #

    @staticmethod
    def highlight(line: str, keyword: str, max_len: int = 120) -> str:
        """
        Return the line with occurrences of keyword wrapped in «…».
        Truncated to max_len characters with trailing "…" if needed.
        """
        result  = line.strip()
        pattern = re.compile(re.escape(keyword), re.IGNORECASE)
        result  = pattern.sub(lambda m: f"«{m.group(0)}»", result)

        if len(result) > max_len:
            result = result[:max_len - 1] + "…"

        return result