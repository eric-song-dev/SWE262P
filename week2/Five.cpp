/* ************************************************************************
> File Name:     Five.cpp
> Author:        Zhenyu Song
> Mail:          zhenyus4@uci.edu
> Created Time:  Mon Oct 13 13:52:14 2025
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

const std::string STOP_WORDS_PATH = "../stop_words.txt";
const int MAX_COUNT = 25;

bool loadStopWords(const std::string& filename, std::unordered_set<std::string>& stopWords) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "failed to open " << filename << std::endl;
        return false;
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
    return true;
}

void toLower(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
}

void processLine(const std::string& line, const std::unordered_set<std::string>& stopWords, std::unordered_map<std::string, int>& wordCounts) {
    if (line.empty()) return;

    auto dummyLine = line + " ";
    std::string word;
    for (char ch : dummyLine) {
        if (isalnum(ch)) {
            word += ch;
            continue;
        }

        toLower(word);

        if (word.length() >= 2 && !stopWords.count(word)) {
            ++wordCounts[word];
        }
        word.clear();
    }
}

bool readFileAndCountWords(const std::string& filename, const std::unordered_set<std::string>& stopWords, std::unordered_map<std::string, int>& wordCounts) {
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        std::cerr << "failed to open " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        processLine(line, stopWords, wordCounts);
    }
    inputFile.close();
    return true;
}

void sortAndPrintResults(const std::unordered_map<std::string, int>& wordCounts) {
    std::vector<std::pair<std::string, int>> wordVec(wordCounts.begin(), wordCounts.end());

    std::sort(wordVec.begin(), wordVec.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
        return a.second > b.second;
    });

    const int numToPrint = std::min(MAX_COUNT, static_cast<int>(wordVec.size()));
    for (int i = 0; i < numToPrint; ++i) {
        std::cout << wordVec[i].first << " - " << wordVec[i].second << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "wrong usage, please enter: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::unordered_set<std::string> stopWords;
    loadStopWords(STOP_WORDS_PATH, stopWords);
    std::unordered_map<std::string, int> wordCounts;
    readFileAndCountWords(argv[1], stopWords, wordCounts);
    sortAndPrintResults(wordCounts);

    return 0;
}
