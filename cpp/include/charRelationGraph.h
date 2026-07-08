#pragma once
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;

/*

Module 3 — Character Relationship Graph (C++)

Vertices: Each vertex represents one character.

Edges: Edges connect characters appearing together within:
the same chapter
the same paragraph
the same dialogue block (preferred)

Edge Weight: weighting strategies used:  Number of co-occurrences
*/
 
 
class Edge
{
    public:
    int destination;    //index of the connected character
    int weight; //number of co-occurences
    Edge(int dest, int w = 1)
    {
        destination = dest;
        weight = w;
    }
};

class CharacterNode
{
    public:
    string characterName;
    vector<Edge> neighbors;
    CharacterNode(const string &name)
    {
        characterName = name;
    }
};

class CharacterRelationGraph
{
private:
    vector<CharacterNode> vertices;
    unordered_map<string, int> characterIndex;

public:
    CharacterRelationGraph();

    void addCharacter(const string& name);

    void addRelationship(
        const string& character1,
        const string& character2,
        int weight = 1);

    bool hasCharacter(const string& name) const;

    void printGraph() const;
    void saveToFile(const string& filename) const;
};


void CharacterRelationGraph::saveToFile(const string& filename) const
{
    ofstream fout(filename);

    if (!fout)
    {
        cerr << "Could not open " << filename << endl;
        return;
    }

    for (const auto& vertex : vertices)
    {
        fout << vertex.characterName << " -> ";

        for (const auto& edge : vertex.neighbors)
        {
            fout << "("
                 << vertices[edge.destination].characterName
                 << ", "
                 << edge.weight
                 << ") ";
        }

        fout << '\n';
    }

    fout.close();
}