package plugins.extractors;

import api.IWordExtractor;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.*;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class NormalExtractor implements IWordExtractor {
    @Override
    public List<String> extract(String filePath) throws IOException {
        Set<String> stopWords = new HashSet<>();
        try {
            String stopWordsContent = new String(Files.readAllBytes(Paths.get("../stop_words.txt")));
            String[] stops = stopWordsContent.split(",");
            stopWords.addAll(Arrays.asList(stops));
        } catch (IOException e) {
            System.err.println("[Warning] ../stop_words.txt not found");
        }
        // Add single letters a-z to stop words
        for (char c = 'a'; c <= 'z'; c++) {
            stopWords.add(String.valueOf(c));
        }

        String content = new String(Files.readAllBytes(Paths.get(filePath)));
        String[] allWords = content.toLowerCase().split("[\\W_]+");

        return Arrays.stream(allWords)
                .filter(w -> !w.isEmpty())
                .filter(w -> !stopWords.contains(w))
                .collect(Collectors.toList());
    }
}