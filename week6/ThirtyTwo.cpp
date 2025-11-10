/* ************************************************************************
> File Name:     ThirtyTwo.cpp
> Author:        Zhenyu Song
> Mail:          zhenyus4@uci.edu
> Created Time:  Sun Nov  9 19:26:33 2025
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
#include <thread>
#include <future>
#include <numeric>
#include <functional>
#include <iterator>

const std::string STOP_WORDS_PATH = "../stop_words.txt";
const int MAX_COUNT = 25;
const int CHUNK_SIZE = 200;

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

std::unordered_map<std::string, int> mapProcessChunks(const std::vector<std::string>& chunk, const std::unordered_set<std::string>& stopWords) {
    std::unordered_map<std::string, int> localWordCounts;
    for (const std::string& line : chunk) {
        processLine(line, stopWords, localWordCounts);
    }
    return localWordCounts;
}

std::unordered_map<std::string, std::vector<int>> regroupCounts(const std::vector<std::unordered_map<std::string, int>>& allPartialCounts) {
    std::unordered_map<std::string, std::vector<int>> regroupedMap;
    for (const auto& partialMap : allPartialCounts) {
        for (const auto& pair : partialMap) {
            regroupedMap[pair.first].push_back(pair.second);
        }
    }
    return regroupedMap;
}

std::pair<std::string, int> reduceCounts(
    const std::pair<std::string, std::vector<int>>& wordGroup 
) {
    int total = std::accumulate(wordGroup.second.begin(), wordGroup.second.end(), 0);
    return {wordGroup.first, total};
}

std::vector<std::pair<std::string, int>> processReduceChunkByIndex(const std::vector<std::pair<std::string, std::vector<int>>>& regroupedVec, size_t start, size_t end
) {
    std::vector<std::pair<std::string, int>> localWordVec;
    localWordVec.reserve(end - start);

    size_t actualEnd = std::min(end, regroupedVec.size());
    for (size_t i = start; i < actualEnd; ++i) {
        localWordVec.push_back(reduceCounts(regroupedVec[i]));
    }
    return localWordVec;
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "wrong usage, please enter: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    
    std::unordered_set<std::string> stopWords;
    if (!loadStopWords(STOP_WORDS_PATH, stopWords)) {
        std::cerr << "failed to load stop words" << std::endl;
        return 1;
    }
    
    // 1. Partition
    std::vector<std::vector<std::string>> lineChunks;
    std::ifstream inputFile(argv[1]);
    if (!inputFile.is_open()) {
        std::cerr << "failed to open " << argv[1] << std::endl;
        return 1;
    }

    std::string line;
    std::vector<std::string> currentChunk;
    int lineCount = 0;
    while (std::getline(inputFile, line)) {
        currentChunk.push_back(line);
        lineCount++;
        if (lineCount >= CHUNK_SIZE) {
            lineChunks.push_back(currentChunk);
            currentChunk.clear();
            lineCount = 0;
        }
    }
    if (!currentChunk.empty()) {
        lineChunks.push_back(currentChunk);
    }
    inputFile.close();

    // 2. First Map
    std::vector<std::future<std::unordered_map<std::string, int>>> mapFutures;
    for (const auto& chunk : lineChunks) {
        mapFutures.push_back(std::async(
            std::launch::async, 
            mapProcessChunks, 
            std::cref(chunk), 
            std::cref(stopWords)
        ));
    }

    std::vector<std::unordered_map<std::string, int>> allPartialCounts;
    for (auto& fut : mapFutures) {
        allPartialCounts.push_back(fut.get());
    }

    // 3. Shuffle
    auto regroupedMap = regroupCounts(allPartialCounts);

    // 4. Second Map 
    const int numReducers = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::pair<std::string, std::vector<int>>> regroupedVector(
        regroupedMap.begin(), regroupedMap.end()
    );

    size_t totalUniqueWords = regroupedVector.size();
    size_t baseChunkSize = totalUniqueWords / numReducers;
    size_t remainder = totalUniqueWords % numReducers;

    std::vector<std::future<std::vector<std::pair<std::string, int>>>> reduceFutures;
    size_t currentStart = 0;

    for (int i = 0; i < numReducers; ++i) {
        size_t currentChunkSize = baseChunkSize + (i < remainder ? 1 : 0);
        if (currentChunkSize == 0) {
            continue;
        }
        size_t currentEnd = currentStart + currentChunkSize;
        
        reduceFutures.push_back(std::async(
            std::launch::async,
            processReduceChunkByIndex,
            std::cref(regroupedVector),
            currentStart,
            currentEnd
        ));
        currentStart = currentEnd;
    }

    // 5. Final Aggregation
    std::vector<std::pair<std::string, int>> wordVec; 
    for (auto& fut : reduceFutures) {
        auto partialVec = fut.get();
        wordVec.insert(
            wordVec.end(), 
            std::make_move_iterator(partialVec.begin()), 
            std::make_move_iterator(partialVec.end())
        );
    }
    
    // 6. Sort and Print
    std::sort(wordVec.begin(), wordVec.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
        return a.second > b.second;
    });
    
    const int numToPrint = std::min(MAX_COUNT, static_cast<int>(wordVec.size()));
    for (int i = 0; i < numToPrint; ++i) {
        std::cout << wordVec[i].first << " - " << wordVec[i].second << std::endl;
    }
    
    return 0;
}