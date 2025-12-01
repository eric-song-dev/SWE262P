/* ************************************************************************
> File Name:     Iterators.java
> Author:        Zhenyu Song
> Mail:          zhenyus4@uci.edu
> Created Time:  Sat Nov 29 02:51:26 2025
> Description:   
 ************************************************************************/

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.*;

public class Iterators {

    private static final String STOP_WORDS_PATH = "../stop_words.txt";

    private static final int MAX_COUNT = 25;

    static class Characters implements Iterator<Character> {
        private final BufferedReader reader;
        private int nextChar;

        public Characters(String filename) throws IOException {
            this.reader = new BufferedReader(new FileReader(filename));
            this.nextChar = reader.read();
        }

        @Override
        public boolean hasNext() {
            return nextChar != -1;
        }

        @Override
        public Character next() {
            if (nextChar == -1) throw new NoSuchElementException();
            char current = (char) nextChar;
            try {
                nextChar = reader.read();
                if (nextChar == -1) reader.close();
            } catch (IOException e) {
                nextChar = -1;
            }
            return current;
        }
    }

    /**
     * A stream of words derived from the stream of characters.
     * Mimics: def all_words(filename)
     */
    static class AllWords implements Iterator<String> {
        private final Iterator<Character> charStream;
        private String nextWord;

        public AllWords(String filename) throws IOException {
            this.charStream = new Characters(filename);
            this.nextWord = computeNextWord();
        }

        private String computeNextWord() {
            StringBuilder sb = new StringBuilder();
            boolean startChar = true;

            while (charStream.hasNext()) {
                char c = charStream.next();
                if (startChar) {
                    if (Character.isLetterOrDigit(c)) {
                        sb.append(Character.toLowerCase(c));
                        startChar = false;
                    }
                } else {
                    if (Character.isLetterOrDigit(c)) {
                        sb.append(Character.toLowerCase(c));
                    } else {
                        // End of word found
                        return sb.toString();
                    }
                }
            }
            // Return the last word if file ends
            if (sb.length() > 0) return sb.toString();
            return null;
        }

        @Override
        public boolean hasNext() {
            return nextWord != null;
        }

        @Override
        public String next() {
            if (nextWord == null) throw new NoSuchElementException();
            String result = nextWord;
            nextWord = computeNextWord();
            return result;
        }
    }

    /**
     * A stream of words filtering out stop words.
     * Mimics: def non_stop_words(filename)
     */
    static class NonStopWords implements Iterator<String> {
        private final Iterator<String> wordStream;
        private final Set<String> stopWords = new HashSet<>();
        private String nextValidWord;

        public NonStopWords(String filename) throws IOException {
            this.wordStream = new AllWords(filename);
            loadStopWords();
            this.nextValidWord = computeNextValidWord();
        }

        private void loadStopWords() throws IOException {
            // Assumes stop_words.txt is in the parent directory relative to execution
            String content = new String(Files.readAllBytes(Paths.get(STOP_WORDS_PATH)));
            Collections.addAll(stopWords, content.split(","));
            // Add single-letter words
            for (char c = 'a'; c <= 'z'; c++) {
                stopWords.add(String.valueOf(c));
            }
        }

        private String computeNextValidWord() {
            while (wordStream.hasNext()) {
                String word = wordStream.next();
                if (!stopWords.contains(word)) {
                    return word;
                }
            }
            return null;
        }

        @Override
        public boolean hasNext() {
            return nextValidWord != null;
        }

        @Override
        public String next() {
            if (nextValidWord == null) throw new NoSuchElementException();
            String result = nextValidWord;
            nextValidWord = computeNextValidWord();
            return result;
        }
    }

    /**
     * A stream of sorted frequency lists, yielded periodically.
     * Mimics: def count_and_sort(filename)
     */
    static class CountAndSort implements Iterator<List<Map.Entry<String, Integer>>> {
        private final Iterator<String> wordStream;
        private final Map<String, Integer> freqs = new HashMap<>();
        private int count = 1;
        private boolean finished = false;

        public CountAndSort(String filename) throws IOException {
            this.wordStream = new NonStopWords(filename);
        }

        @Override
        public boolean hasNext() {
            // If the upstream is empty but we haven't emitted the final state, we have one last item
            return wordStream.hasNext() || !finished;
        }

        @Override
        public List<Map.Entry<String, Integer>> next() {
            if (finished && !wordStream.hasNext()) throw new NoSuchElementException();

            while (wordStream.hasNext()) {
                String word = wordStream.next();
                freqs.put(word, freqs.getOrDefault(word, 0) + 1);

                if (count % 5000 == 0) {
                    count++;
                    return getSortedList();
                }
                count++;
            }

            // Processing the final batch
            finished = true;
            return getSortedList();
        }

        private List<Map.Entry<String, Integer>> getSortedList() {
            List<Map.Entry<String, Integer>> list = new ArrayList<>(freqs.entrySet());
            list.sort((e1, e2) -> e2.getValue().compareTo(e1.getValue())); // Sort descending by value
            return list;
        }
    }

    public static void main(String[] args) {
        if (args.length < 1) {
            System.out.println("Usage: java Iterators <path_to_file>");
            return;
        }

        try {
            Iterator<List<Map.Entry<String, Integer>>> river = new CountAndSort(args[0]);
            while (river.hasNext()) {
                List<Map.Entry<String, Integer>> freqs = river.next();
                System.out.println("-----------------------------");
                int numToPrint = Math.min(MAX_COUNT, freqs.size());
                for (int i = 0; i < numToPrint; ++i) {
                    System.out.println(freqs.get(i).getKey() + " - " + freqs.get(i).getValue());
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}