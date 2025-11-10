/* ************************************************************************
> File Name:     TwentyNine.cpp
> Author:        Zhenyu Song
> Mail:          zhenyus4@uci.edu
> Created Time:  Sun Nov  9 19:26:09 2025
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
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <atomic>
#include <iomanip>
#include <future>

const std::string STOP_WORDS_PATH = "../stop_words.txt";
const int MAX_COUNT = 25;

enum class MessageType {
    FilePath = 0,
    Line,
    StopWords,
    Word,
    FileProcessDone,
    WordCountDone,
    WordCounts,
    SortedWords,
    PrintDone
};

class Message {
public:
    virtual ~Message() = default;
    virtual MessageType getType() const = 0;
};

class FilePathMsg : public Message {
public:
    FilePathMsg(const std::string& p) : path(p) {}
    MessageType getType() const override { return MessageType::FilePath; }

public:
    std::string path;
};

class LineMsg : public Message {
public:
    LineMsg(const std::string& l) : line(l) {}
    MessageType getType() const override { return MessageType::Line; }

public:
    std::string line;
};

class StopWordsMsg : public Message {
public:
    StopWordsMsg(const std::unordered_set<std::string>& sw) : stopWords(sw) {}
    MessageType getType() const override { return MessageType::StopWords; }

public:
    std::unordered_set<std::string> stopWords;
};

class WordMsg : public Message {
public:
    WordMsg(const std::string& w) : word(w) {}
    MessageType getType() const override { return MessageType::Word; }

public:
    std::string word;
};

class FileProcessDoneMsg : public Message {
public:
    MessageType getType() const override { return MessageType::FileProcessDone; }
};

class WordCountDoneMsg : public Message {
public:
    MessageType getType() const override { return MessageType::WordCountDone; }
};

class WordCountsMsg : public Message {
public:
    WordCountsMsg(const std::unordered_map<std::string, int>& c) : counts(c) {}
    MessageType getType() const override { return MessageType::WordCounts; }

public:
    std::unordered_map<std::string, int> counts;
};

class SortedWordsMsg : public Message {
public:
    SortedWordsMsg(std::vector<std::pair<std::string, int>>&& vec) : topWords(std::move(vec)) {}
    MessageType getType() const override { return MessageType::SortedWords; }

public:
    std::vector<std::pair<std::string, int>> topWords;
};

class PrintDoneMsg : public Message {
public:
    MessageType getType() const override { return MessageType::PrintDone; }
};

class Actor {
public:
    Actor() = default;

    Actor(const Actor&) = delete;
    Actor& operator=(const Actor&) = delete;

    virtual ~Actor() {
        stop();
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    void start() {
        running = true;
        thread = std::thread(&Actor::run, this);
    }

    void stop() {
        running = false;
        cv.notify_one();
    }

    void send(std::unique_ptr<Message> msg) {
        if (!running) return;
        std::lock_guard<std::mutex> lock(mtx);
        msgQueue.push(std::move(msg));
        cv.notify_one();
    }

    void run() {
        while (running) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return !msgQueue.empty() || !running; });
            
            if (!running) break;
            
            auto msg = std::move(msgQueue.front());
            msgQueue.pop();
            lock.unlock();
            
            processMsg(std::move(msg));
        }
    }

protected:
    virtual void processMsg(std::unique_ptr<Message> msg) = 0;

protected:
    std::queue<std::unique_ptr<Message>> msgQueue;
    std::mutex mtx;
    std::condition_variable cv;
    std::thread thread;
    std::atomic<bool> running{false};
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

class StopWordsLoaderActor : public Actor {
public:
    StopWordsLoaderActor(std::shared_ptr<Actor> lp) : lineProcessorActor(lp) {}

protected:
    void processMsg(std::unique_ptr<Message> msg) override {
        switch(msg->getType()) {
            case MessageType::FilePath: {
                auto filePathMsg = static_cast<FilePathMsg*>(msg.get());
                std::unordered_set<std::string> stopWords;
                if (loadStopWords(filePathMsg->path, stopWords)) {
                    lineProcessorActor->send(std::make_unique<StopWordsMsg>(stopWords));
                } else {
                    std::cerr << "could not load stop words" << std::endl;
                    lineProcessorActor->send(std::make_unique<StopWordsMsg>(stopWords));
                }
                break;
            }
            default:
                break;
        }
    }

private:
    std::shared_ptr<Actor> lineProcessorActor;
};

class LineProcessorActor : public Actor {
public:
    LineProcessorActor(std::shared_ptr<Actor> c) : counterActor(c) {}

protected:
    void processMsg(std::unique_ptr<Message> msg) override {
        switch (msg->getType()) {
            case MessageType::StopWords: {
                auto stopWordsMsg = static_cast<StopWordsMsg*>(msg.get());
                stopWords = stopWordsMsg->stopWords;
                stopWordsLoaded = true;
                processBuffer();
                break;
            }
            case MessageType::Line: {
                if (stopWordsLoaded) {
                    processLine(static_cast<LineMsg*>(msg.get())->line);
                } else {
                    lineBuffer.push(std::unique_ptr<LineMsg>(static_cast<LineMsg*>(msg.release())));
                }
                break;
            }
            case MessageType::FileProcessDone: {
                counterActor->send(std::make_unique<WordCountDoneMsg>());
                break;
            }
            default:
                break;
        }
    }

private:
    void processLine(const std::string& line) {
        if (line.empty()) return;
        
        auto dummyLine = line + " ";
        std::string word;
        for (char ch : dummyLine) {
            if (isalnum(ch)) {
                word += ch;
                continue;
            }
            if (word.empty()) continue;

            toLower(word);
            
            if (word.length() < 2 || stopWords.count(word)) {
                word.clear();
                continue;
            }
            counterActor->send(std::make_unique<WordMsg>(word));
            word.clear();
        }
    }

    void processBuffer() {
        while(!lineBuffer.empty()) {
            processLine(lineBuffer.front()->line);
            lineBuffer.pop();
        }
    }

private:
    std::shared_ptr<Actor> counterActor;
    std::unordered_set<std::string> stopWords;
    bool stopWordsLoaded = false;
    std::queue<std::unique_ptr<LineMsg>> lineBuffer;
};

class CounterActor : public Actor {
public:
    CounterActor(std::shared_ptr<Actor> s) : sortActor(s) {}

protected:
    void processMsg(std::unique_ptr<Message> msg) override {
        switch (msg->getType()) {
            case MessageType::Word: {
                ++wordCounts[static_cast<WordMsg*>(msg.get())->word];
                break;
            }
            case MessageType::WordCountDone: {
                sortActor->send(std::make_unique<WordCountsMsg>(wordCounts));
                break;
            }
            default:
                break;
        }
    }

private:
    std::shared_ptr<Actor> sortActor;
    std::unordered_map<std::string, int> wordCounts;
};

class SortActor : public Actor {
public:
    SortActor(std::shared_ptr<Actor> p) : printerActor(p) {}

protected:
    void processMsg(std::unique_ptr<Message> msg) override {
        switch (msg->getType()) {
            case MessageType::WordCounts: {
                auto countsMsg = static_cast<WordCountsMsg*>(msg.get());
                std::vector<std::pair<std::string, int>> wordVec(
                    countsMsg->counts.begin(), countsMsg->counts.end()
                );
                
                std::sort(wordVec.begin(), wordVec.end(), 
                    [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
                        return a.second > b.second;
                    }
                );
                
                int numToTake = std::min(MAX_COUNT, static_cast<int>(wordVec.size()));
                std::vector<std::pair<std::string, int>> topWords;
                topWords.reserve(numToTake);
                std::move(wordVec.begin(), wordVec.begin() + numToTake, std::back_inserter(topWords));

                printerActor->send(std::make_unique<SortedWordsMsg>(std::move(topWords)));
                break;
            }
            default:
                break;
        }
    }

private:
    std::shared_ptr<Actor> printerActor;
};

class ShutdownActor : public Actor {
public:
    ShutdownActor(std::promise<void>&& p) : promise(std::move(p)) {}

    void setActors(std::vector<std::shared_ptr<Actor>>&& actorList) {
        actors = std::move(actorList);
    }

protected:
    void processMsg(std::unique_ptr<Message> msg) override {
        switch (msg->getType()) {
            case MessageType::PrintDone: {
                for (auto& actor : actors) {
                    actor->stop();
                }
                promise.set_value();
                break;
            }
            default:
                break;
        }
    }

private:
    std::promise<void> promise;
    std::vector<std::shared_ptr<Actor>> actors;
};


class PrinterActor : public Actor {
public:
    PrinterActor(std::shared_ptr<Actor> sa) : shutdownActor(sa) {}

protected:
    void processMsg(std::unique_ptr<Message> msg) override {
        switch (msg->getType()) {
            case MessageType::SortedWords: {
                auto sortedMsg = static_cast<SortedWordsMsg*>(msg.get());
                for (const auto& pair : sortedMsg->topWords) {
                    std::cout << pair.first << " - " << pair.second << std::endl;
                }
                
                shutdownActor->send(std::make_unique<PrintDoneMsg>());
                break;
            }
            default:
                break;
        }
    }

private:
    std::shared_ptr<Actor> shutdownActor;
};

class FileReaderActor : public Actor {
public:
    FileReaderActor(std::shared_ptr<Actor> lp) : lineProcessorActor(lp) {}

protected:
    void processMsg(std::unique_ptr<Message> msg) override {
        switch (msg->getType()) {
            case MessageType::FilePath: {
                auto filePathMsg = static_cast<FilePathMsg*>(msg.get());
                std::ifstream inputFile(filePathMsg->path);
                if (!inputFile.is_open()) {
                    std::cerr << "failed to open " << filePathMsg->path << std::endl;
                    lineProcessorActor->send(std::make_unique<FileProcessDoneMsg>());
                    return;
                }

                std::string line;
                while (std::getline(inputFile, line)) {
                    lineProcessorActor->send(std::make_unique<LineMsg>(line));
                }
                inputFile.close();
                
                lineProcessorActor->send(std::make_unique<FileProcessDoneMsg>());
                break;
            }
            default:
                break;
        }
    }

private:
    std::shared_ptr<Actor> lineProcessorActor;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "wrong usage, please enter: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    // Set up promise/future for graceful shutdown
    std::promise<void> shutdownPromise;
    std::future<void> shutdownFuture = shutdownPromise.get_future();

    auto shutdownActor = std::make_shared<ShutdownActor>(std::move(shutdownPromise));
    auto printerActor = std::make_shared<PrinterActor>(shutdownActor);
    auto sortActor = std::make_shared<SortActor>(printerActor);
    auto counterActor = std::make_shared<CounterActor>(sortActor);
    auto lineProcessorActor = std::make_shared<LineProcessorActor>(counterActor);
    auto stopWordsLoaderActor = std::make_shared<StopWordsLoaderActor>(lineProcessorActor);
    auto fileReaderActor = std::make_shared<FileReaderActor>(lineProcessorActor);

    std::vector<std::shared_ptr<Actor>> allActors = {
        shutdownActor,
        printerActor, 
        sortActor, 
        counterActor, 
        lineProcessorActor, 
        stopWordsLoaderActor, 
        fileReaderActor, 
    };

    for (const auto& actor : allActors) {
        actor->start();
    }

    shutdownActor->setActors(std::move(allActors));

    // Start the pipeline
    // 1. Load stop words
    stopWordsLoaderActor->send(std::make_unique<FilePathMsg>(STOP_WORDS_PATH));
    
    // 2. Read file
    fileReaderActor->send(std::make_unique<FilePathMsg>(argv[1]));

    // Wait for the signal from ShutdownActor
    shutdownFuture.wait();
    
    return 0;
}