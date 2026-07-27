#include "../include/mod4.h"
#include "../include/charRelationGraph.h"
#include "../include/graphMetrics.h"
#include "buildGraph.cpp"
using namespace std;
int main()
{
    CharacterRelationGraph graph  = returnGraph();
    weightedDegree(graph);
}