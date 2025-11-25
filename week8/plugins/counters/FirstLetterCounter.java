package plugins.counters;

import api.IWordCounter;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class FirstLetterCounter implements IWordCounter {
    @Override
    public Map<String, Integer> count(List<String> words) {
        Map<String, Integer> letterCounts = new HashMap<>();
        for (String w : words) {
            if (w == null || w.isEmpty()) continue;
            String firstChar = String.valueOf(w.charAt(0));
            if (Character.isLetter(w.charAt(0))) {
                letterCounts.put(firstChar, letterCounts.getOrDefault(firstChar, 0) + 1);
            }
        }
        return letterCounts;
    }
}