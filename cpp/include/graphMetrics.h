#pragma once
#include <iostream>
#include <iomanip>
#include "charRelationGraph.h"
using namespace std;
/*
There must be ways to quantify the denisty or sparcity of a graph. 
There are some scores or metrics that aim to do this. 
- graph density 
density = (2E)/(V(V-1)) [for undirected graph]
density = (E)/(V(V-1))  [for directed graph]

- average degree
average degree = 2E/V
*/

//graph is at "../../graph/graph.txt"

int getVertexCount(const CharacterRelationGraph& graph)
{
    return graph.vertices.size();
}

// Counts the number of edges
// Since the graph is undirected, every edge is stored twice.
int getEdgeCount(const CharacterRelationGraph& graph)
{
    int edges = 0;

    for (const auto& vertex : graph.vertices)
    {
        edges += vertex.neighbors.size();
    }

    return edges / 2;
}

// Calculates graph density
// Density = 2E / (V(V-1)) for an undirected graph
double graphDensity(const CharacterRelationGraph& graph)
{
    int V = getVertexCount(graph);

    if (V < 2)
        return 0.0;

    int E = getEdgeCount(graph);

    return (2.0 * E) / (V * (V - 1));
}

// Calculates average degree
// Average Degree = 2E / V
double averageDegree(const CharacterRelationGraph& graph)
{
    int V = getVertexCount(graph);

    if (V == 0)
        return 0.0;

    int E = getEdgeCount(graph);

    return (2.0 * E) / V;
}

// Prints all graph metrics
void printGraphMetrics(const CharacterRelationGraph& graph)
{
    int V = getVertexCount(graph);
    int E = getEdgeCount(graph);

    cout << "Graph Metrics\n";
    cout << "-------------\n";
    cout << "Vertices       : " << V << endl;
    cout << "Edges          : " << E << endl;
    cout << "Density        : " << fixed << setprecision(4)
         << graphDensity(graph) << endl;
    cout << "Average Degree : " << fixed << setprecision(2)
         << averageDegree(graph) << endl;
}