# ISA-TOP README

**Author:** Peter Stáhl
**Login:** xstahl01  
**Dátum vytvorenia:** 5.10.2024  

## Popis
ISA-TOP je nástroj na monitorovanie siete určený na zobrazenie prenosových rýchlostí pre jednotlivé IP adresy komunikujúce so zariadením, na ktorom nástroj beží. ISA-TOP pomocou knižnice `libpcap` zachytáva sieťovú prevádzku na špecifikovanom rozhraní a vypočítava prenosovú rýchlosť pre každé zachytené spojenie. Program funguje ako konzolová aplikácia, ktorá poskytuje štatistiky v reálnom čase zobrazené priamo v termináli.


## Príklad použitia

```bash
  make (kompilácia projektu)

  ./isa-top -i <názov_rozhrania> [-s b|p] [-t <interval>]

  -i <názov_rozhrania> : Názov sieťového rozhrania, ktoré sa má monitorovať.
  -s b|p               : Zoradenia štatistík podľa bajtov ('b') alebo paketov ('p'). Predvolená hodnota sú bajty.
  -t <interval>        : Nastavenia intervalu monitorovania v sekundách. Predvolená hodnota je 1.

  make clean
```

## Zoznam odovzdaných súborov
#### Build nástroje
**CMakeLists.txt**
**Makefile**

#### Zdrojové kódy
**src**
**Kód pre testy:** **tests** (chýbajú testy pre PacketCapture)

#### Dokumentácia
**manual.pdf**
**README.md**