/* ************************************************************************
> File Name:     Seven.cpp
> Author:        Zhenyu Song
> Mail:          zhenyus4@uci.edu
> Created Time:  Sun Oct 19 15:50:27 2025
> Description:   
 ************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <sstream>

int main(int argc, char* argv[]) {
    std::unordered_set<std::string> stopWords; std::string line, word; std::ifstream stopFile("../stop_words.txt");
    while (std::getline(stopFile, line)) { std::stringstream ss(line); while (std::getline(ss, word, ',')) stopWords.insert(word); }
    std::unordered_map<std::string, int> wordCounts; std::ifstream inputFile(argv[1]);
    while (std::getline(inputFile, line)) for (char ch : line + " ") if (std::isalnum(ch)) word += std::tolower(ch); else { if (word.length() > 1 && !stopWords.count(word)) wordCounts[word]++; word.clear(); }
    std::vector<std::pair<std::string, int>> wordVec(wordCounts.begin(), wordCounts.end()); std::sort(wordVec.begin(), wordVec.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) { return a.second > b.second; });
    for (int i = 0; i < std::min(25, (int)wordVec.size()); ++i) std::cout << wordVec[i].first << " - " << wordVec[i].second << std::endl;
    return 0;
}