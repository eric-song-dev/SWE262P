package api;

import java.util.List;
import java.util.Map;

public interface IWordCounter {
    Map<String, Integer> count(List<String> words);
}