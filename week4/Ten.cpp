/* ************************************************************************
> File Name:     Ten.cpp
> Author:        Zhenyu Song
> Mail:          zhenyus4@uci.edu
> Created Time:  Mon Oct 27 22:31:08 2025
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
#include <type_traits>

const std::string STOP_WORDS_PATH = "../stop_words.txt";
const int MAX_COUNT = 25;

// Helper Functions
#pragma mark - Helper Functions

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
        
        if (word.length() < 2 || stopWords.count(word)) {
            word.clear();
            continue;
        }
        ++wordCounts[word];
        word.clear();
    }
}

// The One Functions
#pragma mark - The One Functions

std::unordered_map<std::string, int> processFile(const std::string& filename) {
    std::unordered_set<std::string> stopWords;
    if (!loadStopWords(STOP_WORDS_PATH, stopWords)) {
        std::cerr << "failed to load stop words" << std::endl;
        return {};
    }
    
    std::unordered_map<std::string, int> wordCounts;
    
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        std::cerr << "failed to open " << filename << std::endl;
        return {};
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        processLine(line, stopWords, wordCounts);
    }
    inputFile.close();
    
    return wordCounts;
}

std::vector<std::pair<std::string, int>> sortCounts(const std::unordered_map<std::string, int>& wordCounts) {
    std::vector<std::pair<std::string, int>> wordVec(wordCounts.begin(), wordCounts.end());
    
    std::sort(wordVec.begin(), wordVec.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
        return a.second > b.second;
    });
    
    return wordVec;
}

std::string outputResults(const std::vector<std::pair<std::string, int>>& wordVec) {
    std::stringstream output;
    const int numToPrint = std::min(MAX_COUNT, static_cast<int>(wordVec.size()));
    for (int i = 0; i < numToPrint; ++i) {
        output << wordVec[i].first << " - " << wordVec[i].second << std::endl;
    }
    return output.str();
}

template<typename T>
class TheOne {
private:
    T _value;

public:
    explicit TheOne(T v) : _value(v) {}

    // applies a function and returns a new TheOne wrapper
    template<typename Func>
    TheOne<typename std::result_of<Func(T)>::type> bind(Func func) {
       typedef typename std::result_of<Func(T)>::type U;
       return TheOne<U>(func(_value));
    }

    // print the final value
    void print() const {
        std::cout << *static_cast<const std::string*>(&_value);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "wrong usage, please enter: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    // chain the operations using TheOne class and bind
    TheOne<std::string>(argv[1])
        .bind(processFile)
        .bind(sortCounts)
        .bind(outputResults)
        .print();

    return 0;
}
