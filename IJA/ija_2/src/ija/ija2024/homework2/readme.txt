
# Otevreni dokumentace (API): v adresari doc otevrete v prohlizeci soubor index.html

# Cesta k junit
JUNIT="junit-platform-console-standalone-1.11.4.jar"

# Kompilace s vyuzitim JUnit:
javac -cp .:ijatool.jar:${JUNIT} -d build ija/ija2024/homework2/Homework2Test.java

# Spusteni testu s vyuzitim JUnit:
java -ea -jar ./${JUNIT} execute --cp build:ijatool.jar --scan-class-path

# Prelozi a spusti aplikaci s vizualizerem
javac -cp .:ijatool.jar -d build ija/ija2024/homework2/Homework2.java 
java -cp build:ijatool.jar ija.ija2024.homework2.Homework2

