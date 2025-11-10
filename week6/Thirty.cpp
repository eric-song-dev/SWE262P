/* ************************************************************************
> File Name:     Thirty.cpp
> Author:        Zhenyu Song
> Mail:          zhenyus4@uci.edu
> Created Time:  Sun Nov  9 19:26:19 2025
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
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

const std::string STOP_WORDS_PATH = "../stop_words.txt";
const int MAX_COUNT = 25;
const int NUM_WORKERS = 5;

template <typename T>
class ThreadSafeQueue {
public:
    void push(T item) {
        std::unique_lock<std::mutex> lock(mtx);
        queue.push(item);
        cv.notify_one();
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !queue.empty() || closed; });

        if (queue.empty() && closed) {
            return false;
        }

        item = queue.front();
        queue.pop();
        return true;
    }

    void close() {
        std::unique_lock<std::mutex> lock(mtx);
        closed = true;
        cv.notify_all();
    }

private:
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cv;
    bool closed = false;
};

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

void processLinesWorker(ThreadSafeQueue<std::string>& lineSpace, ThreadSafeQueue<std::unordered_map<std::string, int>>& freqSpace, const std::unordered_set<std::string>& stopWords
) {
    std::unordered_map<std::string, int> localWordCounts;
    std::string line;

    while (lineSpace.pop(line)) {
        processLine(line, stopWords, localWordCounts);
    }

    freqSpace.push(localWordCounts);
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
    
    ThreadSafeQueue<std::string> lineSpace;
    ThreadSafeQueue<std::unordered_map<std::string, int>> freqSpace;

    std::ifstream inputFile(argv[1]);
    if (!inputFile.is_open()) {
        std::cerr << "failed to open " << argv[1] << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        lineSpace.push(line);
    }
    inputFile.close();
    lineSpace.close();

    std::vector<std::thread> workers;
    for (int i = 0; i < NUM_WORKERS; ++i) {
        workers.emplace_back(
            processLinesWorker, 
            std::ref(lineSpace), 
            std::ref(freqSpace), 
            std::cref(stopWords)
        );
    }

    for (auto& t : workers) {
        t.join();
    }
    
    freqSpace.close();
    
    std::unordered_map<std::string, int> wordCounts;
    std::unordered_map<std::string, int> partialCounts;

    while (freqSpace.pop(partialCounts)) {
        for (const auto& pair : partialCounts) {
            wordCounts[pair.first] += pair.second;
        }
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