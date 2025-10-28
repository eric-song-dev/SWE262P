/* ************************************************************************
> File Name:     Fifteen.cpp
> Author:        Zhenyu Song
> Mail:          zhenyus4@uci.edu
> Created Time:  Mon Oct 27 22:52:46 2025
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

// Framework
#pragma mark - Framework

// callback types
using LoadEventHandler = std::function<void(const std::string&)>;
using DoWorkEventHandler = std::function<void()>;
using EndEventHandler = std::function<void()>;
using LineEventHandler = std::function<void(const std::string&)>;

class WordFrequencyFramework {
private:
    std::vector<LoadEventHandler> loadEventHandlers;
    std::vector<DoWorkEventHandler> doWorkEventHandlers;
    std::vector<EndEventHandler> endEventHandlers;

public:
    void registerForLoadEvent(LoadEventHandler handler) {
        loadEventHandlers.push_back(handler);
    }

    void registerForDoWorkEvent(DoWorkEventHandler handler) {
        doWorkEventHandlers.push_back(handler);
    }

    void registerForEndEvent(EndEventHandler handler) {
        endEventHandlers.push_back(handler);
    }

    void run(const std::string& path) {
        for (const auto& handler : loadEventHandlers) {
            handler(path);
        }
        for (const auto& handler : doWorkEventHandlers) {
            handler();
        }
        for (const auto& handler : endEventHandlers) {
            handler();
        }
    }
};

// Application
#pragma mark - Application

class StopWordManager {
public:
    StopWordManager(WordFrequencyFramework& framework) {
        framework.registerForLoadEvent([this](const std::string& path) { this->onLoad(); });
    }

    const std::unordered_set<std::string>& getStopWords() const {
        return stopWords;
    }

private:
    bool loadStopWords(const std::string& filename) {
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

    void onLoad() {
        if (!loadStopWords(STOP_WORDS_PATH)) {
            std::cerr << "failed to load stop words" << std::endl;
        }
    }

private:
    std::unordered_set<std::string> stopWords;
};

class FileDataProcessor {
public:
    FileDataProcessor(WordFrequencyFramework& framework) {
        framework.registerForLoadEvent([this](const std::string& path) { this->load(path); });
        framework.registerForDoWorkEvent([this]() { this->processFile(); });
    }

    void registerForLineEvent(LineEventHandler handler) {
        lineEventHandlers.push_back(handler);
    }

private:
    void load(const std::string& path) {
        this->path = path;
    }

    void processFile() {
        std::ifstream inputFile(path);
        if (!inputFile.is_open()) {
            std::cerr << "failed to open " << path << std::endl;
            return;
        }

        std::string line;
        while (std::getline(inputFile, line)) {
            for (const auto& handler : lineEventHandlers) {
                handler(line);
            }
        }
        inputFile.close();
    }

private:
    std::string path;
    std::vector<LineEventHandler> lineEventHandlers;
};

class WordCounter {
public:
    WordCounter(WordFrequencyFramework& framework, FileDataProcessor& processor, const StopWordManager& manager) : stopWordManager(manager) {
        processor.registerForLineEvent([this](const std::string& line) { this->processLine(line); });
        framework.registerForEndEvent([this]() { this->printResults(); });
    }

private:
    void toLower(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { 
            return std::tolower(c); 
        });
    }

    void processLine(const std::string& line) {
        const std::unordered_set<std::string>& stopWords = stopWordManager.getStopWords();

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

    void printResults() {
        std::vector<std::pair<std::string, int>> wordVec(wordCounts.begin(), wordCounts.end());
        
        std::sort(wordVec.begin(), wordVec.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
            return a.second > b.second;
        });
        
        const int numToPrint = std::min(MAX_COUNT, static_cast<int>(wordVec.size()));
        for (int i = 0; i < numToPrint; ++i) {
            std::cout << wordVec[i].first << " - " << wordVec[i].second << std::endl;
        }
    }

private:
    std::unordered_map<std::string, int> wordCounts;
    const StopWordManager& stopWordManager;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "wrong usage, please enter: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    // create the framework
    WordFrequencyFramework framework;

    // create the application entities, passing the framework
    StopWordManager stopWordManager(framework);
    FileDataProcessor fileProcessor(framework);
    WordCounter wordCounter(framework, fileProcessor, stopWordManager);

    // run the framework
    framework.run(argv[1]);

    return 0;
}