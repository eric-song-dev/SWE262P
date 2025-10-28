/* ************************************************************************
> File Name:     Four.cpp
> Author:        Zhenyu Song
> Mail:          zhenyus4@uci.edu
> Created Time:  Mon Oct 13 13:40:37 2025
> Description:   
 ************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cctype>
#include <sstream>
#include <algorithm>

const std::string STOP_WORDS_PATH = "../stop_words.txt";
const int MAX_COUNT = 25;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "wrong usage, please enter: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    
    std::unordered_set<std::string> stopWords;
    std::ifstream stopWordsFile(STOP_WORDS_PATH);
    if (!stopWordsFile.is_open()) {
        std::cerr << "failed to open " << STOP_WORDS_PATH << std::endl;
        return 1;
    }
    std::string stopWordsLine;
    while (std::getline(stopWordsFile, stopWordsLine)) {
        std::stringstream ss(stopWordsLine);
        std::string word;
        while (std::getline(ss, word, ',')) {
            stopWords.insert(word);
        }
    }
    stopWordsFile.close();
    
    std::unordered_map<std::string, int> wordCounts;
    std::ifstream inputFile(argv[1]);
    if (!inputFile.is_open()) {
        std::cerr << "failed to open " << argv[1] << std::endl;
        return 1;
    }
    std::string line;
    while (std::getline(inputFile, line)) {
        if (line.empty()) continue;
    
        auto dummyLine = line + " ";
        std::string word;
        for (char ch : dummyLine) {
            if (isalnum(ch)) {
                word += ch;
                continue;
            }

            std::transform(word.begin(), word.end(), word.begin(), [](unsigned char c) { 
                return std::tolower(c); 
            });
            
            if (word.length() < 2 || stopWords.count(word)) {
                word.clear();
                continue;
            }
            ++wordCounts[word];
            word.clear();
        }
    }
    inputFile.close();
    
    std::vector<std::pair<std::string, int>> wordVec(wordCounts.begin(), wordCounts.end());
    
    auto length = wordVec.size();
    bool swapped = false;
    for (int i = 0; i < length - 1; ++i) {
        swapped = false;
        for (int j = 0; j < length - i - 1; ++j) {
            if (wordVec[j].second < wordVec[j + 1].second) {
                auto tmp = wordVec[j + 1];
                wordVec[j + 1] = wordVec[j];
                wordVec[j] = tmp;
                swapped = true;
            }
        }
        if (!swapped) {
            break;
        }
    }
    
    const int numToPrint = std::min(MAX_COUNT, static_cast<int>(wordVec.size()));
    for (int i = 0; i < numToPrint; ++i) {
        std::cout << wordVec[i].first << " - " << wordVec[i].second << std::endl;
    }
    
    return 0;
}