#!/bin/bash

# Clean up
rm -rf classes plugins_out
mkdir -p classes/api
mkdir -p classes/plugins/extractors
mkdir -p classes/plugins/counters
mkdir -p plugins_out/extractors
mkdir -p plugins_out/counters

# 1. Compile Interfaces
javac -d classes src/api/*.java

# 2. Compile Plugins
javac -cp classes -d classes plugins/extractors/NormalExtractor.java
javac -cp classes -d classes plugins/extractors/ZExtractor.java
javac -cp classes -d classes plugins/counters/NormalCounter.java
javac -cp classes -d classes plugins/counters/FirstLetterCounter.java

# 3. Package Plugins into JARs
jar cvf plugins_out/extractors/NormalExtractor.jar -C classes plugins/extractors/NormalExtractor.class
jar cvf plugins_out/extractors/ZExtractor.jar -C classes plugins/extractors/ZExtractor.class
jar cvf plugins_out/extractors/ZExtractor.jar -C classes plugins/extractors/ZExtractor.class -C classes plugins/extractors/NormalExtractor.class

jar cvf plugins_out/counters/NormalCounter.jar -C classes plugins/counters/NormalCounter.class
jar cvf plugins_out/counters/FirstLetterCounter.jar -C classes plugins/counters/FirstLetterCounter.class

# 4. Compile Twenty
javac -cp classes -d classes src/main/Twenty.java

# 5. Run Instructions
echo "--------------------------------------------------"
echo "Build Complete."
echo "To run with config_normal_and_normal (Normal Words + Normal Freq):"
echo "  java -cp classes main.Twenty config_normal_and_normal.properties ../pride-and-prejudice.txt"
echo ""
echo "To run with config_normal_and_first_letter (Normal Words + First Letter):"
echo "  java -cp classes main.Twenty config_normal_and_first_letter.properties ../pride-and-prejudice.txt"
echo ""
echo "To run with config_z_and_normal (Z Words + Normal Freq):"
echo "  java -cp classes main.Twenty config_z_and_normal.properties ../pride-and-prejudice.txt"
echo ""
echo "To run with config_z_and_first_letter (Z Words + Normal Freq):"
echo "  java -cp classes main.Twenty config_z_and_first_letter.properties ../pride-and-prejudice.txt"
echo "--------------------------------------------------"