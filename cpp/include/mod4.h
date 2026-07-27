#pragma once
#include "charRelationGraph.h"
#include <queue>
#include <stack>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <cmath>
using namespace std;

/*
=============================================================
  Module 4 — Graph Analytics (C++)
  Algorithms implemented:
    1.  Weighted Degree
    2.  PageRank
    3.  Cycle Detection
    4.  Connected Components
    5.  Community Detection  (Label Propagation)
    6.  Johnson's Algorithm  (All-Pairs Shortest Paths)
    7.  Bidirectional Search (Bidirectional BFS)
    8.  Viterbi Algorithm    (Max-Weight Path DP)
    9.  Gabow's Algorithm    (Strongly Connected Components)
   10.  Thorup Algorithm     (Dial / Bucket-Queue SSSP)
=============================================================
*/

// ─────────────────────────────────────────────────────────
// 1. WEIGHTED DEGREE
//    The weighted degree of a vertex is the sum of the
//    weights of all its incident edges — a measure of
//    total interaction strength rather than raw count.
// ─────────────────────────────────────────────────────────
void weightedDegree(const CharacterRelationGraph& graph)
{
    cout << "\n===== Weighted Degree =====\n";

    int maxWeightedDeg = -1;
    string mostInfluential;

    for (const auto& vertex : graph.vertices)
    {
        int wDeg = 0;
        for (const Edge& edge : vertex.neighbors)
        wDeg += edge.weight;

        cout << vertex.characterName
             << " : "
             << wDeg
             << endl;

        if (wDeg > maxWeightedDeg)
        {
            maxWeightedDeg    = wDeg;
            mostInfluential   = vertex.characterName;
        }
    }

    cout << "\nHighest Weighted Degree: "
         << mostInfluential
         << " (Total Interaction Strength = "
         << maxWeightedDeg
         << ")\n";
}


// ─────────────────────────────────────────────────────────
// 2. PAGERANK
//    Iterative random-walk score.
//    PR(v) = (1-d)/V + d * Σ PR(u)/out_degree(u)
//    where the sum is over every neighbor u of v.
//    Because the graph is undirected every neighbor is
//    both an in-neighbor and an out-neighbor.
//    Dampening factor d = 0.85, iterations = 100.
// ─────────────────────────────────────────────────────────
void pageRank(const CharacterRelationGraph& graph,
              double dampingFactor = 0.85,
              int    iterations    = 100)
{
    int V = graph.vertices.size();

    if (V == 0)
    {
        cout << "Graph is empty.\n";
        return;
    }

    vector<double> rank(V, 1.0 / V);
    vector<double> newRank(V, 0.0);

    for (int iter = 0; iter < iterations; iter++)
    {
        fill(newRank.begin(), newRank.end(), (1.0 - dampingFactor) / V);

        for (int u = 0; u < V; u++)
        {
            int deg = graph.vertices[u].neighbors.size();

            if (deg == 0)
                continue;

            double share = rank[u] / deg;

            for (const Edge& edge : graph.vertices[u].neighbors)
            {
                newRank[edge.destination] += dampingFactor * share;
            }
        }

        rank = newRank;
    }

    // Collect and sort for display
    vector<pair<double, string>> scores;

    for (int i = 0; i < V; i++)
    {
        scores.push_back({ rank[i], graph.vertices[i].characterName });
    }

    sort(scores.rbegin(), scores.rend());

    cout << "\n===== PageRank =====\n";
    cout << fixed << setprecision(6);

    for (const auto& s : scores)
    {
        cout << left  << setw(20) << s.second
             << right << s.first
             << endl;
    }

    cout << "\nMost Influential Character: "
         << scores[0].second
         << " (PR = "
         << scores[0].first
         << ")\n";
}


// ─────────────────────────────────────────────────────────
// 3. CYCLE DETECTION
//    For an undirected graph: DFS; if we reach a visited
//    vertex that is not the immediate parent, a cycle
//    exists.  Prints whether the graph contains a cycle
//    and, if so, one example cycle path.
// ─────────────────────────────────────────────────────────
namespace // anonymous — helpers private to this file
{
    bool cycleDetectDFS(const CharacterRelationGraph& graph,
                        int            current,
                        int            parent,
                        vector<bool>&  visited,
                        vector<int>&   path,
                        vector<int>&   cyclePath)
    {
        visited[current] = true;
        path.push_back(current);

        for (const Edge& edge : graph.vertices[current].neighbors)
        {
            int neighbor = edge.destination;

            if (!visited[neighbor])
            {
                if (cycleDetectDFS(graph, neighbor, current, visited, path, cyclePath))
                    return true;
            }
            else if (neighbor != parent)
            {
                // Back edge found — record the cycle
                cyclePath.clear();

                for (int i = path.size() - 1; i >= 0; i--)
                {
                    cyclePath.push_back(path[i]);

                    if (path[i] == neighbor)
                        break;
                }

                cyclePath.push_back(neighbor); // close the cycle
                return true;
            }
        }

        path.pop_back();
        return false;
    }
}

void detectCycles(const CharacterRelationGraph& graph)
{
    int V = graph.vertices.size();
    vector<bool> visited(V, false);
    vector<int>  path;
    vector<int>  cyclePath;

    cout << "\n===== Cycle Detection =====\n";

    bool found = false;

    for (int i = 0; i < V; i++)
    {
        if (!visited[i])
        {
            if (cycleDetectDFS(graph, i, -1, visited, path, cyclePath))
            {
                found = true;
                break;
            }
        }
    }

    if (found)
    {
        cout << "Cycle detected!\n";
        cout << "Example cycle: ";

        for (int i = cyclePath.size() - 1; i >= 0; i--)
        {
            cout << graph.vertices[cyclePath[i]].characterName;

            if (i > 0)
                cout << " -> ";
        }

        cout << endl;
    }
    else
    {
        cout << "No cycles detected. The graph is a forest (acyclic).\n";
    }
}


// ─────────────────────────────────────────────────────────
// 4. CONNECTED COMPONENTS
//    BFS flood-fill: label every vertex with its
//    component ID.  Prints each group of characters that
//    are reachable from one another.
// ─────────────────────────────────────────────────────────
void connectedComponents(const CharacterRelationGraph& graph)
{
    int V = graph.vertices.size();
    vector<int> component(V, -1);
    int         numComponents = 0;

    for (int i = 0; i < V; i++)
    {
        if (component[i] != -1)
            continue;

        // BFS from vertex i
        queue<int> q;
        q.push(i);
        component[i] = numComponents;

        while (!q.empty())
        {
            int current = q.front();
            q.pop();

            for (const Edge& edge : graph.vertices[current].neighbors)
            {
                if (component[edge.destination] == -1)
                {
                    component[edge.destination] = numComponents;
                    q.push(edge.destination);
                }
            }
        }

        numComponents++;
    }

    cout << "\n===== Connected Components =====\n";
    cout << "Total Components: " << numComponents << "\n\n";

    for (int c = 0; c < numComponents; c++)
    {
        cout << "Component " << c + 1 << ": ";

        bool first = true;

        for (int v = 0; v < V; v++)
        {
            if (component[v] == c)
            {
                if (!first)
                    cout << ", ";

                cout << graph.vertices[v].characterName;
                first = false;
            }
        }

        cout << "\n";
    }
}


// ─────────────────────────────────────────────────────────
// 5. COMMUNITY DETECTION — Label Propagation
//    Each vertex starts with its own unique label.
//    On every iteration each vertex adopts the label that
//    is most prevalent (by total edge weight) among its
//    neighbors.  Vertices with the same label form a
//    community.  Converges when no labels change.
// ─────────────────────────────────────────────────────────
void communityDetection(const CharacterRelationGraph& graph,
                        int maxIterations = 50)
{
    int V = graph.vertices.size();

    if (V == 0)
    {
        cout << "Graph is empty.\n";
        return;
    }

    // Every vertex begins as its own community
    vector<int> label(V);
    iota(label.begin(), label.end(), 0);

    for (int iter = 0; iter < maxIterations; iter++)
    {
        bool changed = false;
        vector<int> newLabel = label;

        // Shuffle order to avoid bias (deterministic shuffle by index parity)
        vector<int> order(V);
        iota(order.begin(), order.end(), 0);

        for (int idx = 0; idx < V; idx++)
        {
            int v = order[idx];

            if (graph.vertices[v].neighbors.empty())
                continue;

            // Tally weighted votes for each label
            unordered_map<int, int> votes;

            for (const Edge& edge : graph.vertices[v].neighbors)
            {
                votes[label[edge.destination]] += edge.weight;
            }

            // Pick label with the most votes (ties: lowest label wins)
            int bestLabel = label[v];
            int bestVotes = 0;

            for (const auto& kv : votes)
            {
                if (kv.second > bestVotes ||
                    (kv.second == bestVotes && kv.first < bestLabel))
                {
                    bestVotes = kv.second;
                    bestLabel = kv.first;
                }
            }

            if (bestLabel != label[v])
            {
                newLabel[v] = bestLabel;
                changed = true;
            }
        }

        label = newLabel;

        if (!changed)
            break;
    }

    // Collect communities
    unordered_map<int, vector<string>> communities;

    for (int v = 0; v < V; v++)
    {
        communities[label[v]].push_back(graph.vertices[v].characterName);
    }

    cout << "\n===== Community Detection (Label Propagation) =====\n";
    cout << "Communities found: " << communities.size() << "\n\n";

    int id = 1;

    for (const auto& kv : communities)
    {
        cout << "Community " << id++ << ": ";

        for (size_t i = 0; i < kv.second.size(); i++)
        {
            if (i > 0)
                cout << ", ";

            cout << kv.second[i];
        }

        cout << "\n";
    }
}


// ─────────────────────────────────────────────────────────
// 6. JOHNSON'S ALGORITHM
//    All-pairs shortest paths that handles negative weights.
//    Steps:
//      a. Add virtual source q with 0-weight edges to all V.
//      b. Run Bellman-Ford from q to get potentials h[].
//      c. Reweight every edge: w'(u,v) = w(u,v)+h[u]-h[v]
//         (guaranteed non-negative after reweighting).
//      d. Run Dijkstra from each vertex with w' edges.
//      e. Recover true distance: dist(u,v) = d'(u,v)+h[v]-h[u]
// ─────────────────────────────────────────────────────────
void johnsonsAlgorithm(const CharacterRelationGraph& graph)
{
    int V = graph.vertices.size();

    if (V == 0)
    {
        cout << "Graph is empty.\n";
        return;
    }

    const int INF = numeric_limits<int>::max();

    // ── Step (a & b): Bellman-Ford with virtual source ──
    // The virtual source connects to every real vertex
    // with weight 0, so initial h[] = 0 everywhere.
    // We then relax all existing edges.
    vector<int> h(V, 0);

    bool negativeCycle = false;

    for (int iter = 0; iter < V - 1; iter++)
    {
        bool updated = false;

        for (int u = 0; u < V; u++)
        {
            for (const Edge& edge : graph.vertices[u].neighbors)
            {
                int v = edge.destination;

                if (h[u] != INF && h[u] + edge.weight < h[v])
                {
                    h[v]    = h[u] + edge.weight;
                    updated = true;
                }
            }
        }

        if (!updated)
            break;
    }

    // Check for negative-weight cycles
    for (int u = 0; u < V && !negativeCycle; u++)
    {
        for (const Edge& edge : graph.vertices[u].neighbors)
        {
            if (h[u] != INF && h[u] + edge.weight < h[edge.destination])
            {
                negativeCycle = true;
            }
        }
    }

    if (negativeCycle)
    {
        cout << "Johnson's Algorithm: graph contains a negative-weight cycle. Aborting.\n";
        return;
    }

    // ── Steps (c-e): Dijkstra per source with reweighted edges ──
    vector<vector<int>> dist(V, vector<int>(V, INF));

    for (int src = 0; src < V; src++)
    {
        vector<int>  d(V, INF);
        vector<bool> visited(V, false);
        d[src] = 0;

        for (int iter = 0; iter < V; iter++)
        {
            // Find unvisited vertex with minimum tentative distance
            int u = -1;

            for (int j = 0; j < V; j++)
            {
                if (!visited[j] && (u == -1 || d[j] < d[u]))
                    u = j;
            }

            if (u == -1 || d[u] == INF)
                break;

            visited[u] = true;

            for (const Edge& edge : graph.vertices[u].neighbors)
            {
                int v          = edge.destination;
                int reweighted = edge.weight + h[u] - h[v]; // always >= 0

                if (!visited[v] && d[u] + reweighted < d[v])
                {
                    d[v] = d[u] + reweighted;
                }
            }
        }

        // Recover true distances
        for (int v = 0; v < V; v++)
        {
            if (d[v] != INF)
                dist[src][v] = d[v] + h[v] - h[src];
        }
    }

    // ── Print result matrix ──
    cout << "\n===== Johnson's Algorithm — All-Pairs Shortest Paths =====\n\n";
    cout << setw(15) << "";

    for (int j = 0; j < V; j++)
        cout << setw(15) << graph.vertices[j].characterName;

    cout << "\n";

    for (int i = 0; i < V; i++)
    {
        cout << setw(15) << graph.vertices[i].characterName;

        for (int j = 0; j < V; j++)
        {
            if (dist[i][j] == INF)
                cout << setw(15) << "INF";
            else
                cout << setw(15) << dist[i][j];
        }

        cout << "\n";
    }
}


// ─────────────────────────────────────────────────────────
// 7. BIDIRECTIONAL SEARCH
//    Runs BFS simultaneously from the source forward
//    and from the target backward.  When a vertex is
//    settled by both frontiers the two partial paths
//    are joined to reconstruct the shortest (hop) path.
// ─────────────────────────────────────────────────────────
void bidirectionalSearch(const CharacterRelationGraph& graph,
                         const string& startCharacter,
                         const string& endCharacter)
{
    if (graph.characterIndex.find(startCharacter) == graph.characterIndex.end())
    {
        cout << "Character \"" << startCharacter << "\" not found.\n";
        return;
    }

    if (graph.characterIndex.find(endCharacter) == graph.characterIndex.end())
    {
        cout << "Character \"" << endCharacter << "\" not found.\n";
        return;
    }

    int src = graph.characterIndex.at(startCharacter);
    int tgt = graph.characterIndex.at(endCharacter);
    int V   = graph.vertices.size();

    cout << "\n===== Bidirectional Search =====\n";

    if (src == tgt)
    {
        cout << "Start and end are the same character: "
             << startCharacter << "\n";
        return;
    }

    // Visited arrays and parent arrays for each direction
    vector<int> parentFwd(V, -1), parentBwd(V, -1);
    vector<bool> visitedFwd(V, false), visitedBwd(V, false);

    queue<int> qFwd, qBwd;

    visitedFwd[src] = true;  qFwd.push(src);
    visitedBwd[tgt] = true;  qBwd.push(tgt);

    int meetingVertex = -1;

    while (!qFwd.empty() && !qBwd.empty() && meetingVertex == -1)
    {
        // ── Expand forward frontier ──
        int sizeFwd = qFwd.size();

        for (int i = 0; i < sizeFwd && meetingVertex == -1; i++)
        {
            int current = qFwd.front();
            qFwd.pop();

            for (const Edge& edge : graph.vertices[current].neighbors)
            {
                int nb = edge.destination;

                if (!visitedFwd[nb])
                {
                    visitedFwd[nb] = true;
                    parentFwd[nb]  = current;
                    qFwd.push(nb);
                }

                // Check if this neighbor has already been visited from the back
                if (visitedBwd[nb])
                {
                    meetingVertex = nb;
                    // Make sure parentFwd is set for the meeting vertex
                    if (parentFwd[nb] == -1 && nb != src)
                        parentFwd[nb] = current;
                    break;
                }
            }
        }

        if (meetingVertex != -1)
            break;

        // ── Expand backward frontier ──
        int sizeBwd = qBwd.size();

        for (int i = 0; i < sizeBwd && meetingVertex == -1; i++)
        {
            int current = qBwd.front();
            qBwd.pop();

            for (const Edge& edge : graph.vertices[current].neighbors)
            {
                int nb = edge.destination;

                if (!visitedBwd[nb])
                {
                    visitedBwd[nb] = true;
                    parentBwd[nb]  = current;
                    qBwd.push(nb);
                }

                if (visitedFwd[nb])
                {
                    meetingVertex = nb;
                    if (parentBwd[nb] == -1 && nb != tgt)
                        parentBwd[nb] = current;
                    break;
                }
            }
        }
    }

    if (meetingVertex == -1)
    {
        cout << "No path exists between \""
             << startCharacter << "\" and \""
             << endCharacter << "\".\n";
        return;
    }

    // ── Reconstruct path ──
    vector<string> path;

    // Walk from meeting vertex back to source
    for (int v = meetingVertex; v != -1; v = parentFwd[v])
    {
        path.push_back(graph.vertices[v].characterName);
    }

    reverse(path.begin(), path.end());

    // Walk from meeting vertex forward to target (skip meeting vertex itself)
    for (int v = parentBwd[meetingVertex]; v != -1; v = parentBwd[v])
    {
        path.push_back(graph.vertices[v].characterName);
    }

    cout << "Path from \"" << startCharacter
         << "\" to \"" << endCharacter
         << "\" (length = " << path.size() - 1 << " hops):\n";

    for (size_t i = 0; i < path.size(); i++)
    {
        if (i > 0)
            cout << " -> ";

        cout << path[i];
    }

    cout << "\n";
}


// ─────────────────────────────────────────────────────────
// 8. VITERBI ALGORITHM
//    Adapted for character graphs: finds the path of
//    exactly K steps from a start character that
//    accumulates the maximum total edge weight.
//    This mirrors the classic Viterbi DP on an HMM where
//    each "state" is a character and each "transition
//    probability" is the normalised edge weight.
//
//    viterbi[step][v] = max total weight arriving at v
//                       in exactly `step` transitions.
//    backtrack[step][v] = vertex we came from.
// ─────────────────────────────────────────────────────────
void viterbi(const CharacterRelationGraph& graph,
             const string& startCharacter,
             int steps = 3)
{
    if (graph.characterIndex.find(startCharacter) == graph.characterIndex.end())
    {
        cout << "Character \"" << startCharacter << "\" not found.\n";
        return;
    }

    if (steps < 1)
    {
        cout << "Steps must be at least 1.\n";
        return;
    }

    int V   = graph.vertices.size();
    int src = graph.characterIndex.at(startCharacter);

    const int NEG_INF = numeric_limits<int>::min();

    // DP tables: (steps+1) x V
    vector<vector<int>> dp(steps + 1, vector<int>(V, NEG_INF));
    vector<vector<int>> back(steps + 1, vector<int>(V, -1));

    dp[0][src] = 0; // start at source with cost 0

    for (int step = 1; step <= steps; step++)
    {
        for (int v = 0; v < V; v++)
        {
            // Try every edge u->v
            for (int u = 0; u < V; u++)
            {
                if (dp[step - 1][u] == NEG_INF)
                    continue;

                // Check if edge u->v exists
                for (const Edge& edge : graph.vertices[u].neighbors)
                {
                    if (edge.destination == v)
                    {
                        int candidate = dp[step - 1][u] + edge.weight;

                        if (candidate > dp[step][v])
                        {
                            dp[step][v] = candidate;
                            back[step][v] = u;
                        }
                    }
                }
            }
        }
    }

    // Find the vertex with the best score after `steps` transitions
    int bestVertex = -1;
    int bestScore  = NEG_INF;

    for (int v = 0; v < V; v++)
    {
        if (dp[steps][v] > bestScore)
        {
            bestScore  = dp[steps][v];
            bestVertex = v;
        }
    }

    cout << "\n===== Viterbi Algorithm =====\n";
    cout << "Start      : " << startCharacter << "\n";
    cout << "Steps      : " << steps << "\n";

    if (bestVertex == -1 || bestScore == NEG_INF)
    {
        cout << "No path of " << steps << " steps exists from \""
             << startCharacter << "\".\n";
        return;
    }

    // Backtrack to reconstruct the optimal sequence
    vector<string> sequence;

    for (int v = bestVertex, step = steps; step >= 0; step--)
    {
        sequence.push_back(graph.vertices[v].characterName);

        if (step > 0)
            v = back[step][v];
    }

    reverse(sequence.begin(), sequence.end());

    cout << "Max-weight path (total weight = " << bestScore << "):\n";

    for (size_t i = 0; i < sequence.size(); i++)
    {
        if (i > 0)
            cout << " -> ";

        cout << sequence[i];
    }

    cout << "\n";
}


// ─────────────────────────────────────────────────────────
// 9. GABOW'S ALGORITHM
//    Finds all Strongly Connected Components (SCCs)
//    using two stacks (Gabow / Cheriyan-Mehlhorn variant).
//
//    Stack S holds vertices in DFS discovery order.
//    Stack B marks the boundaries between SCCs.
//    When DFS finishes a root (top of B equals top of S),
//    all vertices above it in S form one SCC.
//
//    Note: for undirected graphs every SCC equals its
//    connected component, so the algorithm is most
//    meaningful when the graph is interpreted as directed
//    (each stored edge is taken as directed u->v).
// ─────────────────────────────────────────────────────────
void gabowSCC(const CharacterRelationGraph& graph)
{
    int V = graph.vertices.size();

    vector<int>  index(V, -1);   // DFS discovery time
    vector<bool> onStack(V, false);

    stack<int> S; // vertex stack (DFS order)
    stack<int> B; // boundary stack (SCC roots)

    int  counter = 0;
    vector<vector<string>> sccs;

    // Iterative DFS using explicit state
    // State: (vertex, neighbor iterator index)
    using Frame = pair<int, int>;
    stack<Frame> callStack;

    auto visit = [&](int start)
    {
        if (index[start] != -1)
            return;

        callStack.push({ start, 0 });

        while (!callStack.empty())
        {
            auto& [v, ni] = callStack.top();

            if (index[v] == -1)
            {
                // First visit
                index[v]   = counter++;
                onStack[v] = true;
                S.push(v);
                B.push(v);
            }

            bool pushed = false;

            while (ni < (int)graph.vertices[v].neighbors.size())
            {
                int w = graph.vertices[v].neighbors[ni].destination;
                ni++;

                if (index[w] == -1)
                {
                    // Tree edge — recurse
                    callStack.push({ w, 0 });
                    pushed = true;
                    break;
                }
                else if (onStack[w])
                {
                    // Back/cross edge — pop B until B.top() has index <= w's index
                    while (index[B.top()] > index[w])
                        B.pop();
                }
            }

            if (!pushed)
            {
                // Finished processing v
                callStack.pop();

                if (!B.empty() && B.top() == v)
                {
                    // v is the root of an SCC
                    B.pop();

                    vector<string> scc;

                    while (true)
                    {
                        int u = S.top();
                        S.pop();
                        onStack[u] = false;
                        scc.push_back(graph.vertices[u].characterName);

                        if (u == v)
                            break;
                    }

                    sccs.push_back(scc);
                }
            }
        }
    };

    for (int i = 0; i < V; i++)
    {
        visit(i);
    }

    cout << "\n===== Gabow's Algorithm — Strongly Connected Components =====\n";
    cout << "SCCs found: " << sccs.size() << "\n\n";

    for (size_t i = 0; i < sccs.size(); i++)
    {
        cout << "SCC " << i + 1 << ": ";

        for (size_t j = 0; j < sccs[i].size(); j++)
        {
            if (j > 0)
                cout << ", ";

            cout << sccs[i][j];
        }

        cout << "\n";
    }
}


// ─────────────────────────────────────────────────────────
// 10. THORUP ALGORITHM (Bucket-Queue / Dial's Variant)
//     Single-source shortest paths for non-negative
//     integer edge weights.
//     Uses a bucket array of size (max_weight * V + 1)
//     instead of a binary heap, giving O(E + V * W)
//     time where W is the maximum edge weight.
//     This is the practical foundation of Thorup's
//     linear-time framework for undirected graphs.
// ─────────────────────────────────────────────────────────
void thorup(const CharacterRelationGraph& graph,
            const string& startCharacter)
{
    if (graph.characterIndex.find(startCharacter) == graph.characterIndex.end())
    {
        cout << "Character \"" << startCharacter << "\" not found.\n";
        return;
    }

    int V   = graph.vertices.size();
    int src = graph.characterIndex.at(startCharacter);

    const int INF = numeric_limits<int>::max();

    // Find maximum edge weight to size the bucket array
    int maxWeight = 0;

    for (const auto& vertex : graph.vertices)
    {
        for (const Edge& edge : vertex.neighbors)
        {
            maxWeight = max(maxWeight, edge.weight);
        }
    }

    if (maxWeight == 0)
    {
        // No edges — every vertex is unreachable except source
        cout << "\n===== Thorup Algorithm (Bucket-Queue SSSP) =====\n";
        cout << "Shortest distances from " << startCharacter << ":\n\n";
        cout << startCharacter << ": 0\n";

        for (int i = 0; i < V; i++)
        {
            if (i != src)
                cout << graph.vertices[i].characterName << ": Unreachable\n";
        }

        return;
    }

    int bucketCount = maxWeight * V + 1;

    // Bucket array: bucket[d] holds all vertices whose current
    // tentative distance equals d.
    vector<vector<int>> bucket(bucketCount);

    vector<int> dist(V, INF);
    dist[src] = 0;
    bucket[0].push_back(src);

    for (int d = 0; d < bucketCount; d++)
    {
        // Process all vertices in bucket d
        // (new vertices may be added to this same bucket during processing)
        size_t idx = 0;

        while (idx < bucket[d].size())
        {
            int u = bucket[d][idx++];

            // Skip stale entries (vertex was already settled at a lower dist)
            if (dist[u] < d)
                continue;

            for (const Edge& edge : graph.vertices[u].neighbors)
            {
                int v        = edge.destination;
                int newDist  = dist[u] + edge.weight;

                if (newDist < dist[v])
                {
                    dist[v] = newDist;

                    if (newDist < bucketCount)
                        bucket[newDist].push_back(v);
                }
            }
        }
    }

    cout << "\n===== Thorup Algorithm (Bucket-Queue SSSP) =====\n";
    cout << "Shortest distances from " << startCharacter << ":\n\n";

    for (int i = 0; i < V; i++)
    {
        cout << graph.vertices[i].characterName << ": ";

        if (dist[i] == INF)
            cout << "Unreachable\n";
        else
            cout << dist[i] << "\n";
    }
}