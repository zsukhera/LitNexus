#include "../include/mod4.h"
#include "../include/charRelationGraph.h"
#include "../include/graphMetrics.h"
#include "buildGraph.cpp"
#include "../include/mod6.h"
using namespace std;


//Here is how to use module 6
/*
SearchEngine engine;
engine.buildIndex("../../data/processed/chapters");

engine.search("ghost");
engine.searchMulti("love revenge Heathcliff");
engine.topWords(20);

auto hints = engine.suggest("hea");  // → {"heathcliff", "heard", "heart", ...}


*/

int main()
{
    CharacterRelationGraph graph  = returnGraph();
    weightedDegree(graph);
    
    SearchEngine engine;
    engine.buildIndex("../../data/processed/chapters");

    engine.search("ghost");
    engine.searchMulti("love revenge Heathcliff");
    engine.topWords(20);

    auto hints = engine.suggest("hea");  // → {"heathcliff", "heard", "heart", ...}

}