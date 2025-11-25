package plugins.extractors;

import api.IWordExtractor;
import java.io.IOException;
import java.util.List;
import java.util.stream.Collectors;

public class ZExtractor implements IWordExtractor {
    private final NormalExtractor normalExtractor = new NormalExtractor();

    @Override
    public List<String> extract(String filePath) throws IOException {
        List<String> words = normalExtractor.extract(filePath);
        return words.stream()
                .filter(w -> w.contains("z"))
                .collect(Collectors.toList());
    }
}