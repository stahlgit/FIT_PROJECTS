/**
    @file display.cpp
    @brief Hlavičkový súbor obsahujúci deklaráciu triedy Display, ktorá je zodpovedná za zobrazovanie štatistík zachytených paketov na obrazovke
    @author Peter Stahl (xstahl01)
*/
#ifndef DISPLAY_H
#define DISPLAY_H

#include "stats.h"
#include <thread>
#include <atomic>
#include <vector>

using namespace std;

/**
    @brief Trieda zobrazenia zodpovedná za vykresľovanie sieťových štatistík v termináli pomocou ncurses.
 */
class Display {
    public:
        /**
        @brief Konštruktor triedy Display 
        @param stats referencia na objekt triedy Stats
        @param sort_option zvolená možnosť zoradenia
        @param refresh_interval interval obnovovania obrazovky
        @param running flag pre indikáciu, či je zobrazovací loop spustený
        */
        Display(Stats& stats, char sort_option, int refresh_interval, bool running);
        /**
        @brief Deštruktor triedy Display
         */
        ~Display();
        /**
        @brief Spustenie zobrazovania
         */
        void run();
        /**
        @brief Zastavenie zobrazovania
         */
        void stop();


    private:
        /**
        @brief Kontrola vstupu používateľa
        @return true ak používateľ stlačil klávesu 'q', inak false
        */
        bool check_exit();

        /**
        @brief Zobrazí hlavičku tabuľky 
        @param col_width_src šírka stĺpca pre zdrojovú IP adresu
        @param col_width_dst šírka stĺpca pre cieľovú IP adresu
        @param col_width_proto šírka stĺpca pre protokol
        @param col_width_rx šírka stĺpca pre prijaté dáta
        @param col_width_tx šírka stĺpca pre odoslané dáta
        */
        void display_header(int col_width_src, int col_width_dst, int col_width_proto, int col_width_rx, int col_width_tx);
        /**
        @brief Získa zoradený zoznam pripojení podľa zvoleného kritéria
        @return zoradený zoznam pripojení
        */
        vector<pair<ConnectionKey, ConnectionStats>> get_sorted_connections();
        /**
        @brief Zobrazí jednotlivé štatistky pripojení v formátovaných stĺpcoch
        */
        void display_connections(const vector<pair<ConnectionKey, ConnectionStats>>& connections, int col_width_src, int col_width_dst, int col_width_proto, int col_width_rx, int col_width_tx);
        /**
        @brief Nepretržite zobrazuje štatistiku siete v slučke, kým sa nezastaví alebo neukončí vstupom používateľa.
        */
        void display_loop();
        /**
        @brief Referencia na objekt triedy Stats
        */
        Stats& stats_;
        // Sort option for displaying network statistics
        /**
        @brief Zvolená možnosť zoradenia
         */
        char sort_option_;
        /**
        @brief Interval obnovovania obrazovky
         */
        int refresh_interval_;
        /**
        @brief flag pre indikáciu, či má program pokračovať v zobrazovaní
         */
        atomic<bool> running_;
        /**
        @brief Vlákno pre zobrazovací loop
         */
        thread display_thread_;

};
#endif 
//====END OF display.h ======
