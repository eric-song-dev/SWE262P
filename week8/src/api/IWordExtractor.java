package api;

import java.io.IOException;
import java.util.List;

public interface IWordExtractor {
    List<String> extract(String filePath) throws IOException;
}