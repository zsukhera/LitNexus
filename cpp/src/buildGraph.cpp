#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>

#include "../include/charRelationGraph.h"

using namespace std;
namespace fs = std::filesystem;

int main()
{
    CharacterRelationGraph graph;

    //----------------------------------------------------------
    // Load all characters
    //----------------------------------------------------------

    vector<string> characters;

    ifstream fin("../../data/processed/characters.txt");

    if (!fin)
    {
        cerr << "Could not open characters.txt\n";
        return 1;
    }

    string name;

    while (getline(fin, name))
    {
        size_t pos = name.find('\t');

        if (pos != string::npos)
            name = name.substr(0, pos);

        if (!name.empty())
        {
            characters.push_back(name);
            graph.addCharacter(name);
        }
    }

    fin.close();

    //----------------------------------------------------------
    // Read every paragraph
    //----------------------------------------------------------

    string paragraphFolder = "../../data/processed/paragraphs";

    for (const auto &entry : fs::directory_iterator(paragraphFolder))
    {
        if (!entry.is_regular_file())
            continue;

        ifstream para(entry.path());

        if (!para)
            continue;

        string text(
            (istreambuf_iterator<char>(para)),
            istreambuf_iterator<char>());

        para.close();

        //------------------------------------------------------
        // Find characters present in this paragraph
        //------------------------------------------------------

        vector<string> present;

        for (const string &character : characters)
        {
            if (text.find(character) != string::npos)
            {
                present.push_back(character);
            }
        }

        //------------------------------------------------------
        // Connect every pair
        //------------------------------------------------------

        for (int i = 0; i < present.size(); i++)
        {
            for (int j = i + 1; j < present.size(); j++)
            {
                graph.addRelationship(
                    present[i],
                    present[j],
                    1
                );
            }
        }
    }

    //----------------------------------------------------------
    // Display graph
    //----------------------------------------------------------

    graph.printGraph();

    //save graph 
    graph.saveToFile("graph.txt");
    return 0;
}