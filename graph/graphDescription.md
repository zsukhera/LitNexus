# Module 3 – Character Relationship Graph

## Overview

This module constructs a **weighted, undirected graph** representing relationships between characters in *Wuthering Heights*. The graph is generated using the processed text produced in Module 2 and serves as the foundation for graph analytics such as centrality measures, PageRank, community detection, and network visualization.

Each **vertex** represents a character, while each **edge** represents a relationship between two characters based on their co-occurrence within the same textual unit.

---

## Graph Representation

The graph is implemented as an **Adjacency List**.

```text
Character
    ├── Neighbor 1 (Weight)
    ├── Neighbor 2 (Weight)
    └── Neighbor 3 (Weight)
```

An adjacency list was selected because:

- The novel contains relatively few significant relationships compared to all possible character pairs.
- It is memory efficient for sparse graphs.
- It allows efficient traversal for graph algorithms.
- It is well suited for weighted graph analysis.

---

## Graph Construction

The graph is built by processing the preprocessed novel text.

For each paragraph (or chosen textual unit):

1. Identify every character appearing in the paragraph.
2. Create an edge between every pair of characters.
3. If the edge already exists, increment its weight.
4. Otherwise, create a new edge with an initial weight of 1.

The resulting graph captures the frequency with which characters appear together throughout the novel.

---

## Edge Weights

Edge weights represent the **number of co-occurrences** between two characters.

For example:

```text
Catherine -> (Heathcliff, 121)
```

means that Catherine and Heathcliff appeared together in **121 textual units**.

A larger weight indicates a stronger narrative relationship.

---

## Graph Properties

- Graph Type: Undirected
- Representation: Weighted Adjacency List
- Vertices: Characters
- Edges: Character co-occurrences
- Edge Weight: Number of co-occurrences

Since the graph is undirected, every relationship is stored in both directions.

Example:

```text
Catherine -> (Heathcliff,121)

Heathcliff -> (Catherine,121)
```

These represent the same relationship.

---

## Output Format

The graph is exported as a text file.

Example:

```text
Heathcliff -> (Catherine,121) (Hareton,39) (Joseph,37)

Catherine -> (Heathcliff,121) (Linton,110) (Edgar,44)

Joseph -> (Heathcliff,37) (Hareton,32)
```

Each line contains:

- Character name
- List of adjacent characters
- Weight of each relationship

---

## Classes

### Edge

Represents a weighted connection between two characters.

**Members**

- `destination`
- `weight`

---

### CharacterNode

Represents a single character in the graph.

**Members**

- `characterName`
- `neighbors`

---
## Example Interpretation

```text
Catherine -> (Heathcliff,121) (Edgar,44)
```

This indicates that:

- Catherine has a strong relationship with Heathcliff.
- Catherine also has a significant relationship with Edgar.
- The relationship with Heathcliff is approximately three times stronger than the relationship with Edgar based on co-occurrence frequency.

---

The generated graph enables quantitative analysis of character interactions and provides the basis for identifying major characters, influential relationships, and narrative communities within *Wuthering Heights*.