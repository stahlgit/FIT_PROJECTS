# FIT_IZP_project2
Just saving here my old project from VUT FIT

If would someone like to use it (I'm sure i will not ), here is some untranslated manual:

Detailní specifikace
Překlad a odevzdání zdrojového souboru
Odevzdání: Program implementujte ve zdrojovém souboru cluster.c. Zdrojový soubor odevzdejte prostřednictvím informačního systému.

Překlad: Program bude překládán s následujícími argumenty

$ gcc -std=c99 -Wall -Wextra -Werror -DNDEBUG cluster.c -o cluster -lm
Definice makra NDEBUG (argument -DNDEBUG) je z důvodu anulování efektu ladicích informací.

Propojení s matematickou knihovnou (argument -lm) je z důvodu výpočtu vzdálenosti objektů.

Syntax spuštění
Program se spouští v následující podobě:

./cluster SOUBOR [N]
Argumenty programu:

SOUBOR je jméno souboru se vstupními daty.

N je volitelný argument definující cílový počet shluků. N > 0. Výchozí hodnota (při absenci argumentu) je 1
