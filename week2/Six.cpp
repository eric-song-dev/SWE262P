/* ************************************************************************
> File Name:     Six.cpp
> Author:        Zhenyu Song
> Mail:          zhenyus4@uci.edu
> Created Time:  Mon Oct 13 13:58:21 2025
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

std::unordered_set<std::string> loadStopWords(const std::string& filename) {
    std::unordered_set<std::string> stopWords;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "failed to open " << filename << std::endl;
        return stopWords;
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
    return stopWords;
}

std::string readFile(const std::string& filename) {
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        std::cerr << "failed to open " << filename << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    return buffer.str();
}

std::vector<std::string> filter(const std::string& data, std::unordered_set<std::string> stopWords) {
    std::vector<std::string> words;
    if (data.empty()) return words;

    auto dummyLine = data + " ";
    std::string currentWord;
    for (char ch : dummyLine) {
        if (isalnum(ch)) {
            currentWord += std::tolower(ch);
        } else if (!currentWord.empty()) {
            if (currentWord.length() >= 2 && !stopWords.count(currentWord)) {
                words.push_back(currentWord);
            }
            currentWord.clear();
        }
    }
    return words;
}

std::unordered_map<std::string, int> count(const std::vector<std::string>& words) {
    std::unordered_map<std::string, int> wordCounts;
    for (const auto& word : words) {
        ++wordCounts[word];
    }
    return wordCounts;
}

std::vector<std::pair<std::string, int>> sort(const std::unordered_map<std::string, int>& wordCounts) {
    std::vector<std::pair<std::string, int>> wordVec(wordCounts.begin(), wordCounts.end());
    std::sort(wordVec.begin(), wordVec.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
        return a.second > b.second;
    });
    return wordVec;
}

void print(const std::vector<std::pair<std::string, int>>& sortedWords) {
    const int numToPrint = std::min(MAX_COUNT, static_cast<int>(sortedWords.size()));
    for (int i = 0; i < numToPrint; ++i) {
        std::cout << sortedWords[i].first << " - " << sortedWords[i].second << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "wrong usage, please enter: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    print(
        sort(
            count(
                filter(
                    readFile(argv[1]),
                    loadStopWords(STOP_WORDS_PATH)
                )
            )
        )
    );

    return 0;
}
