#include "../include/mod4.h"
#include "../include/charRelationGraph.h"
#include "buildGraph.cpp"
using namespace std;
int main()
{
    CharacterRelationGraph graph  = returnGraph();
    // bfs(graph,"Lockwood" );
    dfs(graph, "Lockwood");
    dijkstra(graph, "Lockwood" );
    // floydWarshall(graph);    //output is currently a little messy
    bellmanFord(graph, "Heathcliff");
}