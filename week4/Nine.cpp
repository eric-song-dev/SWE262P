/* ************************************************************************
> File Name:     Nine.cpp
> Author:        Zhenyu Song
> Mail:          zhenyus4@uci.edu
> Created Time:  Mon Oct 27 21:43:41 2025
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
#include <functional>

const std::string STOP_WORDS_PATH = "../stop_words.txt";
const int MAX_COUNT = 25;

using StopWordsSet = std::unordered_set<std::string>;
using WordCounts = std::unordered_map<std::string, int>;
using SortedVec = std::vector<std::pair<std::string, int>>;

using VoidFunc = std::function<void()>;
using PrintFunc = std::function<void(SortedVec, VoidFunc)>;
using SortFunc = std::function<void(WordCounts, PrintFunc)>;
using ProcessFileFunc = std::function<void(const std::string&, StopWordsSet, SortFunc)>;
using LoadStopWordsFunc = std::function<void(const std::string&, ProcessFileFunc, const std::string&)>;

// Helper Functions
#pragma mark - Helper Functions

void toLower(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { 
        return std::tolower(c); 
    });
}

void processLine(const std::string& line, const StopWordsSet& stopWords, WordCounts& wordCounts) {
    if (line.empty()) return;
    
    auto dummyLine = line + " ";
    std::string word;
    for (char ch : dummyLine) {
        if (isalnum(ch)) {
            word += ch;
            continue;
        }

        toLower(word);
        
        if (word.length() < 2 || stopWords.count(word)) {
            word.clear();
            continue;
        }
        ++wordCounts[word];
        word.clear();
    }
}

// Kick Forward Functions
#pragma mark - Kick Forward Functions

void noOperation() {
    // does nothing
}

void printResults(SortedVec wordVec, VoidFunc finalFunc) {
    const int numToPrint = std::min(MAX_COUNT, static_cast<int>(wordVec.size()));
    for (int i = 0; i < numToPrint; ++i) {
        std::cout << wordVec[i].first << " - " << wordVec[i].second << std::endl;
    }
    finalFunc(); // kick to noOperation
}

void sortResults(WordCounts wordCounts, PrintFunc nextFunc) {
    SortedVec wordVec(wordCounts.begin(), wordCounts.end());
    
    std::sort(wordVec.begin(), wordVec.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
        return a.second > b.second;
    });
    
    nextFunc(wordVec, noOperation); // kick to printResults
}

void processFile(const std::string& inputFilePath, StopWordsSet stopWords, SortFunc nextFunc) {
    WordCounts wordCounts;
    
    std::ifstream inputFile(inputFilePath);
    if (!inputFile.is_open()) {
        std::cerr << "failed to open " << inputFilePath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        processLine(line, stopWords, wordCounts);
    }
    inputFile.close();
    
    nextFunc(wordCounts, printResults); // kick to sortResults
}

void loadStopWords(const std::string& filename, ProcessFileFunc nextFunc, const std::string& inputFilePath) {
    StopWordsSet stopWords;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "failed to open " << filename << std::endl;
        std::cerr << "failed to load stop words" << std::endl;
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string word;
        while (std::getline(ss, word, ',')) {
            stopWords.insert(word);
        }
    }
    file.close();
    
    nextFunc(inputFilePath, stopWords, sortResults); // kick to processFile
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "wrong usage, please enter: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    
    // start the kick forward chain
    loadStopWords(STOP_WORDS_PATH, processFile, argv[1]);
    
    return 0;
}