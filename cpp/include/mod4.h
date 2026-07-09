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
/////////////////////////////////
/////Shortest Path Algorithms\\\\

//dfs

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


//dijkstra

//bidirectional search 

//bellman ford 

//floyd warshall

//johnson's algorithm 

//viterbi algorithm 

//gabow's algorithm 

//thorup algorithm 


//strength of the community may be detected by all pair shortest path 
