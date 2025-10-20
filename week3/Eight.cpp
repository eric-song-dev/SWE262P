/* ************************************************************************
> File Name:     Eight.cpp
> Author:        Zhenyu Song
> Mail:          zhenyus4@uci.edu
> Created Time:  Sun Oct 19 16:16:24 2025
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
    std::string line;
    
    if (!file.is_open()) {
        std::cerr << "failed to open " << filename << std::endl;
        return false;
    }
    
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

std::vector<std::string>& parseRecursively(std::ifstream& file, std::vector<std::string>& wordList, const std::unordered_set<std::string>& stopWords) {
    char ch;
    std::string word;

    while (file.get(ch) && !isalnum(ch)) {
        // just continue all characters that are not alpha and number, no need to call parseRecursively
    }

    // base case
    if (file.eof()) {
        return wordList;
    }

    word += ch;
    
    while (file.peek() != EOF && isalnum(file.peek())) {
        file.get(ch);
        word += ch;
    }

    toLower(word);
    if (word.length() >= 2 && !stopWords.count(word)) {
        wordList.push_back(word);
    }

    parseRecursively(file, wordList, stopWords);

    return wordList;
}


int main(int argc, char* argv[]) {
    std::cout << "if a stack overflow occured, please execute 'ulimit -s 65532' in the terminal and then try again\n\n\n" << std::endl;

    if (argc != 2) {
        std::cerr << "wrong usage, please enter: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    
    std::unordered_set<std::string> stopWords;
    if (!loadStopWords(STOP_WORDS_PATH, stopWords)) {
        std::cerr << "failed to load stop words" << std::endl;
        return 1;
    }
    
    std::ifstream inputFile(argv[1]);
    if (!inputFile.is_open()) {
        std::cerr << "failed to open " << argv[1] << std::endl;
        return 1;
    }

    std::vector<std::string> wordList; 

    wordList = parseRecursively(inputFile, wordList, stopWords);
    
    inputFile.close();

    std::unordered_map<std::string, int> wordCounts;
    for (const std::string& word : wordList) {
        ++wordCounts[word];
    }
    
    std::vector<std::pair<std::string, int>> wordVec(wordCounts.begin(), wordCounts.end());
    
    std::sort(wordVec.begin(), wordVec.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
        return a.second > b.second;
    });

    const int numToPrint = std::min(MAX_COUNT, static_cast<int>(wordVec.size()));
    for (int i = 0; i < numToPrint; ++i) {
        std::cout << wordVec[i].first << " - " << wordVec[i].second << std::endl;
    }
    
    return 0;
}