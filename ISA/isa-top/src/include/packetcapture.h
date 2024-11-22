/**
    @file packetcapture.cpp
    @brief Hlavičkový súbor triedy PacketCapture, ktorá je zodpovedná za zachytávanie paketov zo sieťového rozhrania
    @author Peter Stahl (xstahl01)
*/

#ifndef PACKETCAPTURE_H
#define PACKETCAPTURE_H

#include <pcap.h>
#include "stats.h"

using namespace std;


// Definícia štruktúry hlavičky Ethernet pre analýzu paketov.
struct ether_header {
    u_int8_t ether_dhost[6];
    u_int8_t ether_shost[6];
    u_int16_t ether_type;
};

// Trieda PacketCapture zodpovedná za zachytávanie paketov zo sieťového rozhrania.
class PacketCapture {
    public:
        /**
        @brief Konštruktor triedy PacketCapture
        @param interface názov sieťového rozhrania
        @param stats referencia na objekt triedy Stats
        */
        PacketCapture(const string& interface, Stats& stats);
        /**
        @brief Destruktor triedy PacketCapture
        */
        ~PacketCapture();
        /**
        @brief Metóda na spustenie zachytávania paketov
         */
        void start_capture();
        /**
        @brief Metóda na zastavenie zachytávania paketov
        */ 
        void stop_capture();
        


    private:
        /**
        @brief Metóda na spracovanie zachyteného paketu
        @param user pointer na objekt triedy Stats
        @param header pointer na hlavičku paketu
        @param packet pointer na zachytený paket
        */
        static void packet_handler(u_char *user, const struct pcap_pkthdr *header, const u_char *packet);
        /**
        @brief Názov sieťového rozhrania
        */
        string interface_;
        /**
        @brief Referencia na objekt triedy Stats
        */
        Stats& stats_;
        /**
        @brief Handler pre zachytávanie paketov
         */
        pcap_t* handle_;

};

#endif 
//====END OF packetcapture.h ======
