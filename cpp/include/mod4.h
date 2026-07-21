/*
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

### Community Detection

Cluster strongly connected characters into communities.

---
*/

#include "charRelationGraph.h"
#include <queue>
#include <stack>
#include <limits>
#include <iomanip>
/////////////////////////////////
/////Shortest Path Algorithms\\\\
// DFS
// receives a graph and the name of the starting character
// performs DFS using a stack
// targetCharacter is optional and can be used for searching
void dfs(const CharacterRelationGraph& graph,
         const string& startCharacter,
         const string& targetCharacter = "")
{
    // Check if the starting character exists
    if (graph.characterIndex.find(startCharacter) == graph.characterIndex.end())
    {
        cout << "Character \"" << startCharacter << "\" not found.\n";
        return;
    }

    vector<bool> visited(graph.vertices.size(), false);
    stack<int> st;

    int startIndex = graph.characterIndex.at(startCharacter);

    st.push(startIndex);

    cout << "DFS Traversal:\n";

    while (!st.empty())
    {
        int current = st.top();
        st.pop();

        if (visited[current])
            continue;

        visited[current] = true;

        cout << graph.vertices[current].characterName << endl;

        // Stop if searching for a specific character
        if (!targetCharacter.empty() &&
            graph.vertices[current].characterName == targetCharacter)
        {
            cout << "\nFound \"" << targetCharacter << "\".\n";
            return;
        }

        // Push adjacent vertices onto the stack
        // Reverse iteration gives traversal closer to recursive DFS
        for (int i = graph.vertices[current].neighbors.size() - 1; i >= 0; i--)
        {
            int neighbor = graph.vertices[current].neighbors[i].destination;

            if (!visited[neighbor])
            {
                st.push(neighbor);
            }
        }
    }

    if (!targetCharacter.empty())
    {
        cout << "\n\"" << targetCharacter << "\" was not found.\n";
    }
}

//bfs
//receives a graph and the name of the starting character
//does BFS
//the character to be found is optional (default parameter) and can be used for searching
void bfs(const CharacterRelationGraph& graph,const string& startCharacter,const string& targetCharacter = "")
{
    // Check if the starting character exists
    if (graph.characterIndex.find(startCharacter) == graph.characterIndex.end())
    {
        cout << "Character \"" << startCharacter << "\" not found.\n";
        return;
    }

    vector<bool> visited(graph.vertices.size(), false);
    queue<int> q;

    int startIndex = graph.characterIndex.at(startCharacter);

    visited[startIndex] = true;
    q.push(startIndex);

    cout << "BFS Traversal:\n";

    while (!q.empty())
    {
        int current = q.front();
        q.pop();

        cout << graph.vertices[current].characterName << endl;

        // Stop if searching for a specific character
        if (!targetCharacter.empty() &&
            graph.vertices[current].characterName == targetCharacter)
        {
            cout << "\nFound \"" << targetCharacter << "\".\n";
            return;
        }

        // Visit all adjacent vertices
        for (const Edge& edge : graph.vertices[current].neighbors)
        {
            if (!visited[edge.destination])
            {
                visited[edge.destination] = true;
                q.push(edge.destination);
            }
        }
    }

    if (!targetCharacter.empty())
    {
        cout << "\n\"" << targetCharacter << "\" was not found.\n";
    }
}


// Dijkstra
// Finds the shortest path from the starting character to every other character
void dijkstra(const CharacterRelationGraph& graph, const string& startCharacter)
{
    if (graph.characterIndex.find(startCharacter) == graph.characterIndex.end())
    {
        cout << "Character \"" << startCharacter << "\" not found.\n";
        return;
    }

    int V = graph.vertices.size();
    int start = graph.characterIndex.at(startCharacter);

    vector<int> distance(V, numeric_limits<int>::max());
    vector<bool> visited(V, false);

    // Stores the previous vertex in the shortest path
    vector<int> previous(V, -1);

    distance[start] = 0;

    for (int i = 0; i < V; i++)
    {
        int current = -1;
        int minDistance = numeric_limits<int>::max();

        // Find the unvisited vertex with the smallest distance
        for (int j = 0; j < V; j++)
        {
            if (!visited[j] && distance[j] < minDistance)
            {
                minDistance = distance[j];
                current = j;
            }
        }

        if (current == -1)
            break;

        visited[current] = true;

        // Relax all adjacent edges
        for (const Edge& edge : graph.vertices[current].neighbors)
        {
            int neighbor = edge.destination;
            int weight = edge.weight;

            if (!visited[neighbor] &&
                distance[current] != numeric_limits<int>::max() &&
                distance[current] + weight < distance[neighbor])
            {
                distance[neighbor] = distance[current] + weight;
                previous[neighbor] = current;
            }
        }
    }

    cout<<"Dijkstra's algorithm"<<endl;
    cout << "Shortest distances from " << startCharacter << ":\n\n";

    for (int i = 0; i < V; i++)
    {
        cout << graph.vertices[i].characterName << ": ";

        if (distance[i] == numeric_limits<int>::max())
        {
            cout << "Unreachable\n";
        }
        else
        {
            cout << distance[i] << endl;
        }
    }
}

// Bellman-Ford Algorithm
// Finds the shortest path from the starting character to every other character
// Works with negative edge weights and detects negative weight cycles
void bellmanFord(const CharacterRelationGraph& graph, const string& startCharacter)
{
    if (graph.characterIndex.find(startCharacter) == graph.characterIndex.end())
    {
        cout << "Character \"" << startCharacter << "\" not found.\n";
        return;
    }

    int V = graph.vertices.size();
    int start = graph.characterIndex.at(startCharacter);

    vector<int> distance(V, numeric_limits<int>::max());
    vector<int> previous(V, -1);

    distance[start] = 0;

    // Relax all edges V-1 times
    for (int i = 0; i < V - 1; i++)
    {
        bool updated = false;

        for (int u = 0; u < V; u++)
        {
            if (distance[u] == numeric_limits<int>::max())
                continue;

            for (const Edge& edge : graph.vertices[u].neighbors)
            {
                int v = edge.destination;
                int weight = edge.weight;

                if (distance[u] + weight < distance[v])
                {
                    distance[v] = distance[u] + weight;
                    previous[v] = u;
                    updated = true;
                }
            }
        }

        // Stop early if no updates were made
        if (!updated)
            break;
    }

    // Check for negative weight cycles
    for (int u = 0; u < V; u++)
    {
        if (distance[u] == numeric_limits<int>::max())
            continue;

        for (const Edge& edge : graph.vertices[u].neighbors)
        {
            int v = edge.destination;
            int weight = edge.weight;

            if (distance[u] + weight < distance[v])
            {
                cout << "Graph contains a negative weight cycle.\n";
                return;
            }
        }
    }

    cout << "Bellman-Ford Algorithm" << endl;
    cout << "Shortest distances from " << startCharacter << ":\n\n";

    for (int i = 0; i < V; i++)
    {
        cout << graph.vertices[i].characterName << ": ";

        if (distance[i] == numeric_limits<int>::max())
        {
            cout << "Unreachable\n";
        }
        else
        {
            cout << distance[i] << endl;
        }
    }
}


// Floyd-Warshall Algorithm
// Finds the shortest paths between every pair of characters
// Works with positive and negative edge weights (but not negative cycles)
void floydWarshall(const CharacterRelationGraph& graph)
{
    int V = graph.vertices.size();
    const int INF = numeric_limits<int>::max();

    // Distance matrix
    vector<vector<int>> distance(V, vector<int>(V, INF));

    // Initialize distances
    for (int i = 0; i < V; i++)
    {
        distance[i][i] = 0;

        for (const Edge& edge : graph.vertices[i].neighbors)
        {
            distance[i][edge.destination] = edge.weight;
        }
    }

    // Floyd-Warshall algorithm
    for (int k = 0; k < V; k++)
    {
        for (int i = 0; i < V; i++)
        {
            if (distance[i][k] == INF)
                continue;

            for (int j = 0; j < V; j++)
            {
                if (distance[k][j] == INF)
                    continue;

                if (distance[i][k] + distance[k][j] < distance[i][j])
                {
                    distance[i][j] = distance[i][k] + distance[k][j];
                }
            }
        }
    }

    // Check for negative weight cycles
    for (int i = 0; i < V; i++)
    {
        if (distance[i][i] < 0)
        {
            cout << "Graph contains a negative weight cycle.\n";
            return;
        }
    }

    cout << "Floyd-Warshall Algorithm" << endl;
    cout << "Shortest distances between all pairs of characters:\n\n";

    // Print header
    cout << setw(15) << "";
    for (int j = 0; j < V; j++)
    {
        cout << setw(15) << graph.vertices[j].characterName;
    }
    cout << endl;

    // Print distance matrix
    for (int i = 0; i < V; i++)
    {
        cout << setw(15) << graph.vertices[i].characterName;

        for (int j = 0; j < V; j++)
        {
            if (distance[i][j] == INF)
                cout << setw(15) << "INF";
            else
                cout << setw(15) << distance[i][j];
        }

        cout << endl;
    }
}


//johnson's algorithm 

//bidirectional search 
//viterbi algorithm 

//gabow's algorithm 

//thorup algorithm 


//Dgree centrality 
void degreeCentrality(const CharacterRelationGraph& graph)
{
    int maxDegree = -1;
    string mostCentralCharacter;

    cout << "\n===== Degree Centrality =====\n";

    for (const auto& vertex : graph.vertices)
    {
        int degree = vertex.neighbors.size();

        cout << vertex.characterName
             << " : "
             << degree
             << endl;

        if (degree > maxDegree)
        {
            maxDegree = degree;
            mostCentralCharacter = vertex.characterName;
        }
    }

    cout << "\nMost Connected Character: "
         << mostCentralCharacter
         << " (Degree = "
         << maxDegree
         << ")\n";
}
