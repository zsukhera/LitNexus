#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <filesystem>
#include <map>
using namespace std;
namespace fs = std::filesystem;

/*
=============================================================
  Module 6 — Keyword Search Engine (C++)

  Features:
    - Inverted index built from chapter files
    - Case-insensitive keyword lookup
    - Context snippets with the matched keyword highlighted
    - Results ranked by hit frequency (most hits first)
    - Trie for prefix-based autocomplete / suggestions
=============================================================
*/


// ─────────────────────────────────────────────────────────
// TRIE
//   Supports O(L) insert and prefix lookup where L is the
//   length of the word.  Used for autocomplete suggestions.
// ─────────────────────────────────────────────────────────

struct TrieNode
{
    unordered_map<char, TrieNode*> children;
    bool isEndOfWord = false;

    ~TrieNode()
    {
        for (auto& kv : children)
            delete kv.second;
    }
};

class Trie
{
public:
    Trie()  { root = new TrieNode(); }
    ~Trie() { delete root; }

    // Insert a word (already lowercased)
    void insert(const string& word)
    {
        TrieNode* node = root;

        for (char c : word)
        {
            if (node->children.find(c) == node->children.end())
                node->children[c] = new TrieNode();

            node = node->children[c];
        }

        node->isEndOfWord = true;
    }

    // Returns true if the exact word exists
    bool search(const string& word) const
    {
        TrieNode* node = find(word);
        return node && node->isEndOfWord;
    }

    // Returns true if any word starts with prefix
    bool startsWith(const string& prefix) const
    {
        return find(prefix) != nullptr;
    }

    // Collect up to maxSuggestions words that start with prefix
    vector<string> suggestions(const string& prefix, int maxSuggestions = 10) const
    {
        vector<string> results;
        TrieNode* node = find(prefix);

        if (!node)
            return results;

        collectWords(node, prefix, results, maxSuggestions);
        return results;
    }

private:
    TrieNode* root;

    // Walk the trie along the given prefix and return the end node
    TrieNode* find(const string& prefix) const
    {
        TrieNode* node = root;

        for (char c : prefix)
        {
            auto it = node->children.find(c);

            if (it == node->children.end())
                return nullptr;

            node = it->second;
        }

        return node;
    }

    // DFS from node, accumulating words into results
    void collectWords(TrieNode* node,
                      const string& current,
                      vector<string>& results,
                      int maxSuggestions) const
    {
        if ((int)results.size() >= maxSuggestions)
            return;

        if (node->isEndOfWord)
            results.push_back(current);

        for (const auto& kv : node->children)
        {
            collectWords(kv.second,
                         current + kv.first,
                         results,
                         maxSuggestions);
        }
    }
};


// ─────────────────────────────────────────────────────────
// OCCURRENCE
//   One hit in the inverted index.
// ─────────────────────────────────────────────────────────

struct Occurrence
{
    int    chapter;     // 1-based chapter number
    int    lineNum;     // 1-based line within the chapter file
    string lineText;    // raw line from the file (for the snippet)
};


// ─────────────────────────────────────────────────────────
// HELPERS
// ─────────────────────────────────────────────────────────

// Convert a string to lowercase
static string toLower(const string& s)
{
    string result = s;
    transform(result.begin(), result.end(), result.begin(),
              [](unsigned char c){ return tolower(c); });
    return result;
}

// Strip leading/trailing punctuation from a token so that
// "ghost," and "ghost" both map to "ghost" in the index.
static string stripPunctuation(const string& token)
{
    size_t start = 0;
    size_t end   = token.size();

    while (start < end && ispunct((unsigned char)token[start]))
        start++;

    while (end > start && ispunct((unsigned char)token[end - 1]))
        end--;

    return token.substr(start, end - start);
}

// Return a display snippet: the raw line trimmed to maxWidth characters.
// The matched keyword is wrapped in [ ] for visibility in plain terminals.
static string makeSnippet(const string& lineText,
                           const string& keyword,
                           int   maxWidth = 100)
{
    string result = lineText;

    // Trim leading/trailing whitespace
    size_t a = result.find_first_not_of(" \t\r\n");
    size_t b = result.find_last_not_of(" \t\r\n");

    if (a == string::npos)
        return "";

    result = result.substr(a, b - a + 1);

    // Highlight every case-insensitive occurrence of the keyword
    string lower      = toLower(result);
    string lowerKey   = toLower(keyword);
    string highlighted;
    size_t pos        = 0;

    while (pos < result.size())
    {
        size_t found = lower.find(lowerKey, pos);

        if (found == string::npos)
        {
            highlighted += result.substr(pos);
            break;
        }

        highlighted += result.substr(pos, found - pos);
        highlighted += "[" + result.substr(found, lowerKey.size()) + "]";
        pos = found + lowerKey.size();
    }

    // Truncate if too long
    if ((int)highlighted.size() > maxWidth)
        highlighted = highlighted.substr(0, maxWidth - 3) + "...";

    return highlighted;
}


// ─────────────────────────────────────────────────────────
// SEARCH ENGINE
// ─────────────────────────────────────────────────────────

class SearchEngine
{
public:

    // Build the inverted index from all chapter_XX.txt files
    // in the given folder (e.g. "../../data/processed/chapters").
    void buildIndex(const string& chaptersFolder)
    {
        invertedIndex.clear();

        if (!fs::exists(chaptersFolder) || !fs::is_directory(chaptersFolder))
        {
            cerr << "SearchEngine: folder not found: " << chaptersFolder << "\n";
            return;
        }

        // Collect chapter files and sort them so ch 1 < ch 2 < ...
        vector<fs::path> files;

        for (const auto& entry : fs::directory_iterator(chaptersFolder))
        {
            if (entry.is_regular_file())
                files.push_back(entry.path());
        }

        sort(files.begin(), files.end());

        for (const auto& filePath : files)
        {
            // Extract chapter number from filename: "chapter_03.txt" → 3
            string stem    = filePath.stem().string();    // "chapter_03"
            int    chapter = 0;

            size_t underscore = stem.rfind('_');

            if (underscore != string::npos)
            {
                try { chapter = stoi(stem.substr(underscore + 1)); }
                catch (...) { chapter = 0; }
            }

            if (chapter == 0)
                continue;

            ifstream fin(filePath);

            if (!fin)
                continue;

            string line;
            int    lineNum = 0;

            while (getline(fin, line))
            {
                lineNum++;

                if (line.empty())
                    continue;

                // Tokenise
                istringstream iss(line);
                string        token;

                while (iss >> token)
                {
                    string word = toLower(stripPunctuation(token));

                    if (word.empty())
                        continue;

                    // Add to inverted index
                    invertedIndex[word].push_back({ chapter, lineNum, line });

                    // Add to Trie (only once per unique word)
                    if (!trie.search(word))
                        trie.insert(word);
                }
            }
        }

        cout << "SearchEngine: index built from \""
             << chaptersFolder << "\"\n";
        cout << "             Unique terms indexed: "
             << invertedIndex.size() << "\n\n";
    }


    // Search for a single keyword.
    // Prints grouped results ordered by chapter, with hit counts and snippets.
    void search(const string& query) const
    {
        string key = toLower(stripPunctuation(query));

        cout << "\n===== Search: \"" << query << "\" =====\n";

        if (key.empty())
        {
            cout << "Empty query.\n";
            return;
        }

        auto it = invertedIndex.find(key);

        if (it == invertedIndex.end())
        {
            cout << "No results found for \"" << query << "\".\n";

            // Offer suggestions if any prefix matches
            vector<string> sug = trie.suggestions(key, 5);

            if (!sug.empty())
            {
                cout << "Did you mean: ";

                for (size_t i = 0; i < sug.size(); i++)
                {
                    if (i > 0) cout << ", ";
                    cout << sug[i];
                }

                cout << "?\n";
            }

            return;
        }

        const vector<Occurrence>& hits = it->second;

        // Group by chapter: chapter → list of Occurrences
        unordered_map<int, vector<const Occurrence*>> byChapter;

        for (const Occurrence& occ : hits)
            byChapter[occ.chapter].push_back(&occ);

        // Sort chapters in descending order of hit count (most hits first)
        vector<pair<int, int>> chapterHits;  // (chapter, count)

        for (const auto& kv : byChapter)
            chapterHits.push_back({ kv.first, (int)kv.second.size() });

        sort(chapterHits.begin(), chapterHits.end(),
             [](const pair<int,int>& a, const pair<int,int>& b)
             {
                 // Primary: most hits first; secondary: lower chapter first
                 if (a.second != b.second)
                     return a.second > b.second;
                 return a.first < b.first;
             });

        cout << "Found " << hits.size()
             << " occurrence(s) across "
             << chapterHits.size()
             << " chapter(s).\n\n";

        for (const auto& [chNum, count] : chapterHits)
        {
            cout << "Chapter " << setw(2) << chNum
                 << "  (" << count << " hit"
                 << (count == 1 ? "" : "s") << ")\n";

            // Print up to 3 representative snippets per chapter
            const vector<const Occurrence*>& occList = byChapter.at(chNum);

            int shown = 0;

            for (const Occurrence* occ : occList)
            {
                if (shown >= 3)
                {
                    int remaining = (int)occList.size() - shown;
                    cout << "     ... and " << remaining
                         << " more occurrence(s).\n";
                    break;
                }

                string snippet = makeSnippet(occ->lineText, query);

                if (!snippet.empty())
                {
                    cout << "  Line " << setw(4) << occ->lineNum
                         << ": \"" << snippet << "\"\n";
                }

                shown++;
            }

            cout << "\n";
        }
    }


    // Search for multiple space-separated keywords (OR logic).
    // Each keyword is searched independently and results are merged.
    void searchMulti(const string& queryString) const
    {
        istringstream iss(queryString);
        string        word;
        vector<string> keywords;

        while (iss >> word)
            keywords.push_back(word);

        if (keywords.empty())
        {
            cout << "Empty query.\n";
            return;
        }

        if (keywords.size() == 1)
        {
            search(keywords[0]);
            return;
        }

        cout << "\n===== Multi-keyword Search: \"" << queryString << "\" =====\n";

        // Accumulate all hits, deduplicating by (chapter, lineNum)
        // Key: (chapter, lineNum) → pair<lineText, set of matched keywords>
        map<pair<int,int>, pair<string, vector<string>>> merged;

        for (const string& kw : keywords)
        {
            string key = toLower(stripPunctuation(kw));
            auto   it  = invertedIndex.find(key);

            if (it == invertedIndex.end())
                continue;

            for (const Occurrence& occ : it->second)
            {
                auto loc = make_pair(occ.chapter, occ.lineNum);
                merged[loc].first = occ.lineText;
                merged[loc].second.push_back(kw);
            }
        }

        if (merged.empty())
        {
            cout << "No results found for any of the keywords.\n";
            return;
        }

        // Group by chapter
        map<int, vector<pair<int, pair<string, vector<string>>>>> byChapter;

        for (const auto& kv : merged)
            byChapter[kv.first.first].push_back({ kv.first.second, kv.second });

        cout << "Found matches in " << byChapter.size() << " chapter(s).\n\n";

        for (const auto& [chNum, lines] : byChapter)
        {
            cout << "Chapter " << setw(2) << chNum
                 << "  (" << lines.size() << " matching line"
                 << (lines.size() == 1 ? "" : "s") << ")\n";

            int shown = 0;

            for (const auto& [lineNum, textAndKeys] : lines)
            {
                if (shown >= 3) break;

                // Highlight all matched keywords in the snippet
                string snippet = textAndKeys.first;

                for (const string& kw : textAndKeys.second)
                    snippet = makeSnippet(snippet, kw, 120);

                cout << "  Line " << setw(4) << lineNum
                     << ": \"" << snippet << "\"\n";

                shown++;
            }

            if ((int)lines.size() > 3)
            {
                cout << "     ... and " << lines.size() - 3
                     << " more line(s).\n";
            }

            cout << "\n";
        }
    }


    // Return autocomplete suggestions for a prefix
    vector<string> suggest(const string& prefix, int maxSuggestions = 10) const
    {
        return trie.suggestions(toLower(prefix), maxSuggestions);
    }


    // Print the N most frequent words in the index (excluding very short words)
    void topWords(int N = 20, int minLength = 4) const
    {
        vector<pair<int, string>> freq;

        for (const auto& kv : invertedIndex)
        {
            if ((int)kv.first.size() >= minLength)
                freq.push_back({ (int)kv.second.size(), kv.first });
        }

        sort(freq.rbegin(), freq.rend());

        cout << "\n===== Top " << N << " Words (min length " << minLength << ") =====\n";

        int count = 0;

        for (const auto& kv : freq)
        {
            if (count >= N) break;

            cout << left  << setw(25) << kv.second
                 << right << kv.first << " occurrence(s)\n";

            count++;
        }
    }


    // Print a brief summary of the index
    void printIndexStats() const
    {
        int totalOccurrences = 0;

        for (const auto& kv : invertedIndex)
            totalOccurrences += kv.second.size();

        cout << "\n===== Search Index Statistics =====\n";
        cout << "Unique terms      : " << invertedIndex.size()       << "\n";
        cout << "Total occurrences : " << totalOccurrences            << "\n";
    }


private:

    // invertedIndex[lowercased_word] = list of all occurrences
    unordered_map<string, vector<Occurrence>> invertedIndex;

    // Trie for prefix-based suggestions
    Trie trie;

    // map used in searchMulti (ordered by key for deterministic output)
    template<typename K, typename V>
    using map = std::map<K, V>;//error here
};
