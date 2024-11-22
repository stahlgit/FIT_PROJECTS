/**
    @file main.cpp
    @brief Hlavný súbor programu isa-top
    @author Peter Stahl (xstahl01)
*/
#include <iostream>
#include "include/packetcapture.h"
#include "include/stats.h"
#include "include/display.h"
#include "include/utils.h"

using namespace std;

int main(int argc, char* argv[]){
    try{
        // Analyzujte argumenty príkazového riadka na konfiguráciu aplikácie
        Config config = parse_arguments(argc, argv);
        // Vytvorte inštanciu triedy Stats, ktorá bude obsahovať štatistiky o zachytených paketoch
        Stats stats;
        // flag na controlovanie behu programu
        bool running = true;

        // Vytvorte inštanciu triedy PacketCapture, ktorá bude zodpovedná za zachytávanie paketov
        PacketCapture capture(config.interface, stats);
        // Vytvorte inštanciu triedy Display, ktorá bude zodpovedná za zobrazovanie štatistík
        Display display(stats, config.sort_option, config.interval, running);

        // Vytvorte vlákno, ktoré bude zodpovedné za zachytávanie paketov
        thread capture_thread([&](){
            capture.start_capture();
        });

        // Spustite zobrazovanie štatistík
        display.run();
        // po skončení zobrazovania štatistík zastavenie programu
        display.stop();
        capture.stop_capture();
        // čakanie na ukončenie vlákna
        if(capture_thread.joinable()){
            capture_thread.join();
        }
    }
    catch(const exception& e){
        cerr << e.what() << endl;
        print_usage();
        return 1;
    }
    return 0; 
}

//====END OF main.cpp ======
