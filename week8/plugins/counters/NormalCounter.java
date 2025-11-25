package plugins.counters;

import api.IWordCounter;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class NormalCounter implements IWordCounter {
    @Override
    public Map<String, Integer> count(List<String> words) {
        Map<String, Integer> freqs = new HashMap<>();
        for (String w : words) {
            freqs.put(w, freqs.getOrDefault(w, 0) + 1);
        }
        return freqs;
    }
}