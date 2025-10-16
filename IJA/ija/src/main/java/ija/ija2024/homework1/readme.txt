
# Otevreni dokumentace (API): v adresari doc otevrete v prohlizeci soubor index.html

# Cesta k junit
JUNIT="junit-platform-console-standalone-1.11.4.jar"

# Kompilace s vyuzitim JUnit:
javac -cp .:${JUNIT} -d cls ija/ija2024/homework1/Homework1Test.java.

Spusteni testu s vyuzitim JUnit:
java -ea -jar ./${JUNIT} execute --cp cls --scan-class-path
