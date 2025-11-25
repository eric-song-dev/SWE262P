package main;

import api.IWordCounter;
import api.IWordExtractor;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.lang.reflect.Constructor;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.*;

public class Twenty {

    private static final int MAX_COUNT = 25;

    public static void main(String[] args) {
        if (args.length < 2) {
            System.out.println("Usage: java -cp classes main.Twenty <config.properties> <input.txt>");
            return;
        }

        String propertiesFile = args[0];
        String inputFile = args[1];

        Properties props = new Properties();
        try {
            props.load(new FileInputStream(propertiesFile));
        } catch (IOException e) {
            System.err.println("Could not read properties file: " + propertiesFile);
            return;
        }

        String extractorJarPath = props.getProperty("extractor.jar");
        String extractorClassName = props.getProperty("extractor.class");
        String counterJarPath = props.getProperty("counter.jar");
        String counterClassName = props.getProperty("counter.class");

        try {
            // Load plugins
            IWordExtractor extractor = (IWordExtractor) loadPlugin(extractorJarPath, extractorClassName);
            IWordCounter counter = (IWordCounter) loadPlugin(counterJarPath, counterClassName);

            // Execute pipeline
            List<String> words = extractor.extract(inputFile);
            Map<String, Integer> freqs = counter.count(words);

            List<Map.Entry<String, Integer>> sorted = new ArrayList<>(freqs.entrySet());
            sorted.sort(Map.Entry.<String, Integer>comparingByValue().reversed());

            int numToPrint = Math.min(MAX_COUNT, freqs.size());
            for (int i = 0; i < numToPrint; i++) {
                Map.Entry<String, Integer> entry = sorted.get(i);
                System.out.println(entry.getKey() + " - " + entry.getValue());
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static Object loadPlugin(String jarPath, String className) throws Exception {
        File jarFile = new File(jarPath);
        if (!jarFile.exists()) {
            throw new IOException("Plugin jar not found: " + jarFile.getAbsolutePath());
        }

        URL[] urls = { jarFile.toURI().toURL() };
        URLClassLoader child = new URLClassLoader(urls, Twenty.class.getClassLoader());
        Class<?> classToLoad = Class.forName(className, true, child);
        Constructor<?> constructor = classToLoad.getConstructor();
        return constructor.newInstance();
    }
}