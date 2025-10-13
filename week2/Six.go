/* ************************************************************************
> File Name:     Six.go
> Author:        Zhenyu Song
> Mail:          zhenyus4@uci.edu
> Created Time:  Mon Oct 13 14:15:27 2025
> Description:   
 ************************************************************************/

package main

import (
	"fmt"
	"log"
	"os"
	"regexp"
	"sort"
	"strings"
)

const (
	stopWordsPath = "../stop_words.txt"
	maxCount      = 25
)

type Pair struct {
	Word  string
	Count int
}

func loadStopWords(filename string) map[string]bool {
	stopWords := make(map[string]bool)
	content, err := os.ReadFile(filename)
	if err != nil {
		log.Fatalf("failed to open %s: %v", filename, err)
	}

	words := strings.Split(string(content), ",")
	for _, word := range words {
		if len(word) > 0 {
			stopWords[word] = true
		}
	}
	return stopWords
}

func readFile(filename string) string {
	content, err := os.ReadFile(filename)
	if err != nil {
		log.Fatalf("failed to open %s: %v", filename, err)
	}
	return string(content)
}

func filter(data string, stopWords map[string]bool) []string {
	if data == "" {
		return []string{}
	}

	reg := regexp.MustCompile("[a-zA-Z0-9]+")
	words := reg.FindAllString(data, -1)

	filteredWords := []string{}
	for _, word := range words {
		lowerWord := strings.ToLower(word)
		if len(lowerWord) >= 2 {
			if _, isStopWord := stopWords[lowerWord]; !isStopWord {
				filteredWords = append(filteredWords, lowerWord)
			}
		}
	}
	return filteredWords
}

func count(words []string) map[string]int {
	wordCounts := make(map[string]int)
	for _, word := range words {
		wordCounts[word]++
	}
	return wordCounts
}

func sortWords(wordCounts map[string]int) []Pair {
	pairs := make([]Pair, 0, len(wordCounts))
	for word, count := range wordCounts {
		pairs = append(pairs, Pair{word, count})
	}

	sort.Slice(pairs, func(i, j int) bool {
		return pairs[i].Count > pairs[j].Count
	})

	return pairs
}

func printWords(sortedWords []Pair) {
	numToPrint := maxCount
	if len(sortedWords) < maxCount {
		numToPrint = len(sortedWords)
	}

	for i := 0; i < numToPrint; i++ {
		fmt.Printf("%s - %d\n", sortedWords[i].Word, sortedWords[i].Count)
	}
}

func main() {
	if len(os.Args) != 2 {
		fmt.Fprintf(os.Stderr, "wrong usage, please enter: %s <filename>\n", os.Args[0])
		os.Exit(1)
	}

	printWords(
		sortWords(
			count(
				filter(
					readFile(os.Args[1]),
					loadStopWords(stopWordsPath),
				),
			),
		),
	)
}
