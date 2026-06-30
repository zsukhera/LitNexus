# LitNexus
### A Graph-Based Literary Analysis Engine

## Overview

**LitNexus** is an interactive literary analysis platform that processes the HTML version of Emily Brontë's *Wuthering Heights* to construct a character relationship network and provide an intelligent keyword search engine.

The project combines **graph theory**, **data structures**, **algorithms**, **text processing**, **information retrieval**, and **GUI development** using **Python** and **C++**. The methods of this project are not limited to Brontë's work. 

---

## Objectives

The primary objectives of LitNexus are to:

- Parse and clean an HTML version of *Wuthering Heights*.
- Automatically identify chapters, paragraphs, and character occurrences.
- Construct a weighted character relationship graph based on co-occurrence.
- Analyze the graph using classical graph algorithms.
- Determine influential and highly connected characters using graph metrics.
- Implement efficient keyword searching using an inverted index.
- Provide an interactive desktop GUI for searching and graph exploration.
- Visualize the character social network.

---

## System Architecture

```text
                 HTML Novel
                      │
                      ▼
        HTML/Text Preprocessing (Python)
                      │
                      ▼
         Clean Structured Text Files
                      │
        ------------------------------
        │                            │
        ▼                            ▼
 Character Extraction        Search Index Builder
        │                            │
        ▼                            ▼
 Weighted Character Graph      Inverted Index
        │                            │
        ▼                            ▼
 Graph Algorithms         Keyword Search Engine
        │                            │
        └──────────── GUI ───────────┘
```

---

# Project Modules

## Module 1 — HTML Processing (Python)

### Goal

Convert the HTML novel into structured text.

### Responsibilities

- Remove HTML tags
- Preserve chapter headings
- Preserve paragraph boundaries
- Remove unnecessary formatting
- Export clean text files

**Output**

```
chapter_1.txt
chapter_2.txt
...
```

---

## Module 2 — Character Extraction (Python)

Detect all major character occurrences throughout the novel.

Each occurrence stores:

- Character name
- Chapter number
- Paragraph number
- Sentence number

Example:

```
Heathcliff

Chapter 2
Paragraph 15
Sentence 4
```

---

## Module 3 — Character Relationship Graph (C++)

The central component of LitNexus.

### Vertices

Each vertex represents one character.

### Edges

Edges connect characters appearing together within:

- the same chapter
- the same paragraph
- the same dialogue block (preferred)

### Edge Weight

Possible weighting strategies include:

- Number of co-occurrences
- Weighted scene importance
- Dialogue frequency
- Sentiment score (optional)

---

## Module 4 — Graph Analytics (C++)

Implement classical graph algorithms including:

### Degree Centrality

Determine which character interacts with the largest number of unique characters.

### Weighted Degree

Measure total interaction strength.

### PageRank

Estimate the most influential character within the story.

### Shortest Path

Find relationship chains between characters using:

- BFS
- Dijkstra (weighted)

### Cycle Detection

Detect recurring social circles among characters.

### Connected Components

Identify isolated social groups.

### Community Detection (Bonus)

Cluster strongly connected characters into communities.

---

## Module 5 — Keyword Search Engine (C++)

Implement a Google-style search engine.

Example query:

```
ghost
```

Returns:

```
Chapter 3

"He thought he saw a ghost..."

Chapter 9

...

Chapter 17

...
```

Features:

- Inverted index
- Fast keyword lookup
- Highlighted search terms
- Ranked search results

Possible data structures:

- `unordered_map`
- Trie

---

## Module 6 — Desktop GUI (Qt)

The graphical interface allows users to:

- Search the novel
- Browse search results
- View the character network
- Display graph statistics
- Explore relationships between characters

Main interface:

- Search bar
- Search results
- Character graph
- Statistics panel

---

## Module 7 — Graph Visualization (Qt)

Render the character network interactively.

Visual properties include:

- Node = Character
- Edge = Relationship
- Edge thickness = Relationship strength
- Node size = Degree/PageRank

Possible layouts:

- Force-directed
- Circular
- Spring layout

---

# Technologies

## Python

Responsibilities

- HTML parsing
- Text preprocessing
- Character extraction
- JSON generation
- Optional NLP

Libraries

- BeautifulSoup
- lxml
- regex
- NLTK (optional)
- spaCy (optional)

---

## C++

Responsibilities

- Graph construction
- Graph algorithms
- Search engine
- Data structures
- GUI backend

Algorithms

- BFS
- DFS
- Dijkstra
- PageRank
- Connected Components
- Cycle Detection

Data Structures

- Graph (Adjacency List)
- Hash Tables
- Trie
- Priority Queue

---

## Qt

Responsibilities

- Desktop GUI
- Graph visualization
- Search interface
- Statistics dashboard

---

# Project Structure

```
LitNexus/
│
├── data/
│   ├── wuthering.html
│   ├── cleaned/
│   ├── chapters/
│   └── characters.json
│
├── python/
│   ├── html_parser.py
│   ├── character_extractor.py
│   ├── export_graph_data.py
│   └── sentiment.py
│
├── cpp/
│   ├── Graph.h
│   ├── Graph.cpp
│   ├── SearchEngine.cpp
│   ├── PageRank.cpp
│   ├── Trie.cpp
│   ├── GUI.cpp
│   └── main.cpp
│
├── output/
│   ├── graph.json
│   ├── index.json
│   ├── statistics.txt
│   └── search_results/
│
└── README.md
```

---

# Expected Deliverables

By the completion of LitNexus, the system will be able to:

- Parse and preprocess an HTML copy of *Wuthering Heights*.
- Construct a weighted character relationship graph.
- Compute graph metrics including:
  - Degree Centrality
  - Weighted Degree
  - PageRank
  - Shortest Paths
  - Connected Components
  - Cycle Detection
- Visualize the character social network.
- Perform fast keyword searches using an inverted index.
- Highlight matching search results with contextual snippets.
- Explore relationships between characters through an interactive GUI.

---

# Future Enhancements

Possible future improvements include:

- Named Entity Recognition (NER)
- Sentiment-aware relationship graphs
- Dialogue attribution
- TF-IDF ranking
- Community detection algorithms
- Support for multiple novels
- Comparative literary analysis across different works
- Export graph data to GraphML or Gephi
- Interactive graph filtering and analytics

---

## Project Vision

LitNexus demonstrates the integration of **computational linguistics**, **graph theory**, **information retrieval**, and **software engineering** into a unified literary analysis platform. It is designed to showcase efficient algorithm implementation in C++, practical text processing in Python, and modern desktop application development with Qt, while remaining modular, extensible, and applicable to literary works beyond *Wuthering Heights*.