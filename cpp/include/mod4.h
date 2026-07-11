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

//bellman ford 

//floyd warshall

//johnson's algorithm 

//bidirectional search 
//viterbi algorithm 

//gabow's algorithm 

//thorup algorithm 


//strength of the community may be detected by all pair shortest path 
