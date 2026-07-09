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
CharacterRelationGraph::CharacterRelationGraph()
{
    vertices.clear();
    characterIndex.clear();
}

bool CharacterRelationGraph::hasCharacter(const string& name) const
{
    return characterIndex.find(name) != characterIndex.end();
}

void CharacterRelationGraph::addCharacter(const string& name)
{
    if (hasCharacter(name))
        return;

    characterIndex[name] = vertices.size();
    vertices.push_back(CharacterNode(name));
}

void CharacterRelationGraph::addRelationship(
    const string& character1,
    const string& character2,
    int weight)
{
    // Ignore self loops
    if (character1 == character2)
        return;

    // Add characters if they don't already exist
    if (!hasCharacter(character1))
        addCharacter(character1);

    if (!hasCharacter(character2))
        addCharacter(character2);

    int index1 = characterIndex[character1];
    int index2 = characterIndex[character2];

    //------------------------------------
    // Update edge from character1 -> character2
    //------------------------------------
    bool found = false;

    for (auto &edge : vertices[index1].neighbors)
    {
        if (edge.destination == index2)
        {
            edge.weight += weight;
            found = true;
            break;
        }
    }

    if (!found)
        vertices[index1].neighbors.push_back(Edge(index2, weight));

    //------------------------------------
    // Update edge from character2 -> character1
    //------------------------------------
    found = false;

    for (auto &edge : vertices[index2].neighbors)
    {
        if (edge.destination == index1)
        {
            edge.weight += weight;
            found = true;
            break;
        }
    }

    if (!found)
        vertices[index2].neighbors.push_back(Edge(index1, weight));
}

void CharacterRelationGraph::printGraph() const
{
    for (const auto &vertex : vertices)
    {
        cout << vertex.characterName << " -> ";

        for (const auto &edge : vertex.neighbors)
        {
            cout << "("
                 << vertices[edge.destination].characterName
                 << ", "
                 << edge.weight
                 << ") ";
        }

        cout << endl;
    }
}

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