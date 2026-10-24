# LitNexus
### A Graph-Based Literary Analysis Engine
**Zain Sukhera · 2026**

LitNexus processes Emily Brontë's *Wuthering Heights* to build a character relationship graph and a full-text keyword search engine. The pipeline runs Python preprocessing first, then a C++ core that handles graph construction, analytics, and search.

---

## Features

- **HTML → clean text** — BeautifulSoup strips tags and preserves chapter/paragraph structure
- **NLP character extraction** — spaCy NER identifies characters; alias merging collapses variants (`Cathy` → `Catherine`)
- **Weighted co-occurrence graph** — undirected adjacency list; edge weight = paragraph co-occurrence count
- **10 graph algorithms** — see [Algorithms](#algorithms) below
- **Inverted-index search engine** — case-insensitive, ranked results with highlighted snippets
- **Trie autocomplete** — prefix suggestions in O(L) time
- **Qt GUI launcher** — `run_gui.sh` handles WSL display setup automatically

---

## Project Structure

```
LitNexus/
├── python/
│   ├── config.py              # path constants
│   ├── utils.py               # shared helpers
│   ├── html_parser.py         # Module 1 — HTML → clean text
│   ├── split_paragraphs.py    # split clean text into paragraph files
│   ├── split_characters.py    # chapter splitter
│   └── character_extractor.py # Module 2 — spaCy NER → characters.txt
│
├── cpp/
│   ├── include/
│   │   ├── charRelationGraph.h  # Module 3 — graph data structure
│   │   ├── graphMetrics.h       # density, average degree
│   │   ├── mod4.h               # Module 4 — all graph algorithms
│   │   └── mod6.h               # Module 6 — search engine + Trie
│   └── src/
│       ├── buildGraph.cpp       # loads characters.txt + paragraphs → graph
│       └── main.cpp             # entry point
│
├── data/
│   ├── raw/                   # original HTML (pg768-images.html)
│   ├── clean/                 # cleaned full text (pg768_clean.txt)
│   └── processed/
│       ├── chapters/          # chapter_01.txt … chapter_34.txt
│       ├── paragraphs/        # paragraph_00001.txt … (one file per paragraph)
│       └── characters.txt     # name<TAB>count, sorted by frequency
│
├── graph/
│   ├── graph.txt              # saved adjacency list
│   └── graphDescription.md   # Module 3 design notes
│
├── run_gui.sh                 # launch Qt GUI (handles WSL display)
└── description.md             # full project specification
```

---

## Setup

### Python dependencies

```bash
python -m venv venv
source venv/bin/activate
pip install beautifulsoup4 lxml spacy
python -m spacy download en_core_web_sm
```

### C++ requirements

- C++17 compiler (GCC 9+ or Clang 9+)
- Standard library only (no external C++ dependencies)

---

## Pipeline

Run steps in order:

```bash
# 1. Parse HTML → clean text
cd python
python html_parser.py

# 2. Split into paragraphs (used by graph builder)
python split_paragraphs.py

# 3. Split into chapters (used by search engine)
python split_characters.py

# 4. Extract characters via NER
python character_extractor.py
# → writes data/processed/characters.txt

# 5. Build and run C++ engine
cd ../cpp
g++ -std=c++17 -O2 -o litnexus src/main.cpp
./litnexus
```

---

## Algorithms

All implemented in `cpp/include/mod4.h` and `cpp/include/mod6.h`.

| # | Algorithm | Purpose |
|---|-----------|---------|
| 1 | **Weighted Degree** | Total interaction strength per character |
| 2 | **PageRank** | Most narratively influential character (d=0.85, 100 iter) |
| 3 | **Cycle Detection** (DFS) | Detect recurring social circles |
| 4 | **Connected Components** (BFS) | Identify isolated social groups |
| 5 | **Community Detection** (Label Propagation) | Cluster characters by weighted-vote consensus |
| 6 | **Johnson's Algorithm** | All-pairs shortest paths (handles negative weights) |
| 7 | **Bidirectional BFS** | Shortest hop-path between any two characters |
| 8 | **Viterbi** | Max-weight path of exactly K steps from a character |
| 9 | **Gabow's SCC** | Strongly connected components via two-stack DFS |
| 10 | **Dial's Algorithm (Bucket-Queue)** | SSSP with O(E + V·W) bucket-queue |

### Search engine (`mod6.h`)

| Feature | Detail |
|---------|--------|
| Index structure | `unordered_map<string, vector<Occurrence>>` |
| Single keyword search | Ranked by chapter hit count; up to 3 snippets per chapter |
| Multi-keyword search | OR logic; deduplicates by `(chapter, lineNum)` |
| Autocomplete | Trie prefix lookup, returns up to N suggestions |
| Top-N words | Most frequent terms with configurable minimum length |

---

## Usage (C++ API)

```cpp
// Build the character graph from processed data
CharacterRelationGraph graph = returnGraph();

// Graph analytics
weightedDegree(graph);
pageRank(graph);
detectCycles(graph);
connectedComponents(graph);
communityDetection(graph);
johnsonsAlgorithm(graph);
bidirectionalSearch(graph, "Heathcliff", "Edgar");
viterbi(graph, "Catherine", /*steps=*/3);
gabowSCC(graph);
dial(graph, "Heathcliff");

// Graph metrics
printGraphMetrics(graph);  // vertices, edges, density, average degree

// Search engine
SearchEngine engine;
engine.buildIndex("../../data/processed/chapters");

engine.search("ghost");
engine.searchMulti("love revenge Heathcliff");
engine.topWords(20);

auto hints = engine.suggest("hea");  // → {"heathcliff", "heard", "heart", ...}
```

---

## Graph Format

The adjacency list saved to `graph/graph.txt`:

```
Heathcliff -> (Catherine, 121) (Hareton, 39) (Joseph, 37)
Catherine  -> (Heathcliff, 121) (Edgar, 44) (Ellen, 38)
Edgar      -> (Catherine, 44) (Isabella, 29)
```

Each number is the co-occurrence count across all paragraphs.

---

## Technologies

| Layer | Technology |
|-------|-----------|
| HTML parsing | Python · BeautifulSoup · lxml |
| NLP / NER | Python · spaCy (`en_core_web_sm`) |
| Graph core | C++17 · STL only |
| GUI | Qt (launched via `run_gui.sh`) |

---

## Results

Running the pipeline on the Project Gutenberg edition of *Wuthering Heights* (pg768) yields the following.

### Graph structure

| Metric | Value |
|--------|-------|
| Characters (vertices) | 27 |
| Relationships (edges) | 165 |
| Graph density | 0.47 |
| Average degree | 12.22 |

The network is highly dense for a social graph — almost half of all possible character pairs share at least one paragraph together — which reflects Brontë's tight, enclosed cast.

### PageRank (top 10)

PageRank measures structural influence: a character scores higher by being connected to other well-connected characters, not just by appearing frequently.

| Rank | Character | PageRank |
|------|-----------|----------|
| 1 | Catherine | 0.0683 |
| 2 | Linton¹ | 0.0627 |
| 3 | Heathcliff | 0.0603 |
| 4 | Earnshaw | 0.0600 |
| 5 | Joseph | 0.0565 |
| 6 | Edgar | 0.0511 |
| 7 | Hareton | 0.0492 |
| 8 | Isabella | 0.0478 |
| 9 | Hindley | 0.0457 |
| 10 | Dean (Nelly) | 0.0416 |

**Catherine** ranks first despite Heathcliff having the highest raw interaction count. PageRank rewards her central brokering role — she connects the Earnshaw household, the Linton household, and Heathcliff's orbit simultaneously. Nelly Dean's position at #10 is consistent with her narrative role as the story's primary observer.

> ¹ *"Linton" is a spaCy NER conflation of Edgar Linton, Linton Heathcliff (Heathcliff's son), and the Linton family name — a known limitation of the co-occurrence approach on this text.*

### Weighted degree (top 5)

Weighted degree sums total co-occurrence counts, measuring raw interaction volume rather than structural position.

| Character | Weighted Degree |
|-----------|----------------|
| Heathcliff | 503 |
| Linton¹ | 473 |
| Catherine | 470 |
| Earnshaw | 226 |
| Hareton | 214 |

Heathcliff overtakes Catherine here: he shares scenes with nearly every character across both generations of the story, accumulating more total paragraph co-occurrences than anyone else.

### Community detection

With a graph density of 0.47, Label Propagation converges to **one community** — the entire cast. This is an honest structural result: Brontë's isolated Yorkshire setting and multi-generational entanglement mean every character eventually shares scenes with every other, leaving no distinct subgroup to separate out. Algorithms that enforce a minimum modularity threshold (e.g. Louvain) would be needed to surface finer-grained communities within this network.

---

## Future Enhancements

- TF-IDF ranking for search results
- Sentiment-aware edge weights
- Dialogue attribution
- GraphML / Gephi export
- Support for multiple novels
- Named entity disambiguation
