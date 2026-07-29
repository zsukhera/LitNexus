"""
graph_parser.py — load graph.txt and compute derived metrics.

Expected file format (one character per line):
    Catherine -> (Linton, 110) (Heathcliff, 121) ...
"""

from __future__ import annotations
from pathlib import Path
import re


class GraphData:
    """Parsed, in-memory representation of the character relationship graph."""

    def __init__(self) -> None:
        # name -> {degree, weighted_degree, pagerank}
        self.nodes: dict[str, dict] = {}
        # list of (char_a, char_b, weight)  — each edge stored once
        self.edges: list[tuple[str, str, int]] = []
        # name -> [(neighbor_name, weight), ...]
        self.adjacency: dict[str, list[tuple[str, int]]] = {}

    # ------------------------------------------------------------------ #
    #  Loading                                                             #
    # ------------------------------------------------------------------ #

    def load_from_file(self, filepath: str | Path) -> None:
        path = Path(filepath)
        if not path.exists():
            raise FileNotFoundError(f"Graph file not found: {path}")

        self.nodes.clear()
        self.edges.clear()
        self.adjacency.clear()

        edge_set: set[tuple[str, str]] = set()
        pattern = re.compile(r"\(([^,]+),\s*(\d+)\)")

        with open(path, encoding="utf-8") as fh:
            for line in fh:
                line = line.strip()
                if not line or " -> " not in line:
                    continue

                name, neighbors_str = line.split(" -> ", 1)
                name = name.strip()

                if name not in self.nodes:
                    self.nodes[name] = {
                        "degree": 0,
                        "weighted_degree": 0,
                        "pagerank": 0.0,
                    }

                self.adjacency.setdefault(name, [])

                for m in pattern.finditer(neighbors_str):
                    neighbor = m.group(1).strip()
                    weight   = int(m.group(2))

                    self.adjacency[name].append((neighbor, weight))
                    self.nodes[name]["degree"]          += 1
                    self.nodes[name]["weighted_degree"] += weight

                    # Store each undirected edge only once
                    key = tuple(sorted([name, neighbor]))
                    if key not in edge_set:
                        edge_set.add(key)
                        self.edges.append((name, neighbor, weight))

        self._compute_pagerank()

    # ------------------------------------------------------------------ #
    #  PageRank (iterative)                                                #
    # ------------------------------------------------------------------ #

    def _compute_pagerank(
        self, damping: float = 0.85, iterations: int = 100
    ) -> None:
        n = len(self.nodes)
        if n == 0:
            return

        names = list(self.nodes.keys())
        rank  = {name: 1.0 / n for name in names}

        for _ in range(iterations):
            new_rank = {name: (1.0 - damping) / n for name in names}

            for name in names:
                neighbors = self.adjacency.get(name, [])
                deg = len(neighbors)
                if deg == 0:
                    continue
                share = rank[name] / deg
                for nb, _ in neighbors:
                    if nb in new_rank:
                        new_rank[nb] += damping * share

            rank = new_rank

        for name in names:
            self.nodes[name]["pagerank"] = rank[name]

    # ------------------------------------------------------------------ #
    #  Convenience helpers                                                 #
    # ------------------------------------------------------------------ #

    def max_weight(self) -> int:
        return max((w for _, _, w in self.edges), default=1)

    def max_weighted_degree(self) -> int:
        return max(
            (v["weighted_degree"] for v in self.nodes.values()), default=1
        )

    def neighbors_of(self, name: str) -> set[str]:
        return {nb for nb, _ in self.adjacency.get(name, [])}

    def edge_weight(self, a: str, b: str) -> int:
        for nb, w in self.adjacency.get(a, []):
            if nb == b:
                return w
        return 0

    def sorted_by_weighted_degree(self) -> list[tuple[str, dict]]:
        return sorted(
            self.nodes.items(),
            key=lambda kv: kv[1]["weighted_degree"],
            reverse=True,
        )

    def sorted_by_pagerank(self) -> list[tuple[str, dict]]:
        return sorted(
            self.nodes.items(),
            key=lambda kv: kv[1]["pagerank"],
            reverse=True,
        )
