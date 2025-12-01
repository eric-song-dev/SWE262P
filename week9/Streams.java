/* ************************************************************************
> File Name:     Streams.java
> Author:        Zhenyu Song
> Mail:          zhenyus4@uci.edu
> Created Time:  Sat Nov 29 02:51:54 2025
> Description:   
 ************************************************************************/

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.*;
import java.util.function.Function;
import java.util.regex.Pattern;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class Streams {

    private static final String STOP_WORDS_PATH = "../stop_words.txt";

    private static final int MAX_COUNT = 25;

    private static final Pattern SPLIT_PATTERN = Pattern.compile("[^a-zA-Z0-9]+");

    public static void main(String[] args) throws IOException {
        if (args.length < 1) {
            System.out.println("Usage: java Streams <path_to_file>");
            return;
        }

        // 1. Load stop words into a Set
        Set<String> stopWords = new HashSet<>();
        String stopContent = new String(Files.readAllBytes(Paths.get(STOP_WORDS_PATH)));
        Collections.addAll(stopWords, stopContent.split(","));
        for (char c = 'a'; c <= 'z'; c++) {
            stopWords.add(String.valueOf(c));
        }

        // 2. Stream Pipeline
        try (Stream<String> lines = Files.lines(Paths.get(args[0]))) {
            lines.parallel()
                 .flatMap(SPLIT_PATTERN::splitAsStream)
                 .filter(word -> !word.isEmpty())
                 .map(String::toLowerCase)
                 .filter(word -> !stopWords.contains(word))
                 .collect(Collectors.groupingByConcurrent(Function.identity(), Collectors.counting()))
                 .entrySet()
                 .stream()
                 .sorted(Map.Entry.<String, Long>comparingByValue().reversed())
                 .limit(MAX_COUNT)
                 .forEach(e -> System.out.println(e.getKey() + " - " + e.getValue()));
        }
    }
}