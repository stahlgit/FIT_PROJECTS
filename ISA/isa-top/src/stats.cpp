/**
    @file main.cpp
    @brief Implemtácia štatistík 
    @author Peter Stahl (xstahl01)
*/
#include "include/stats.h"
#include <mutex>

using namespace std;

/**
    @brief Metóda na aktualizáciu štatistík
    @param src zdrojová adresa
    @param dst cieľová adresa
    @param proto protokol
    @param bytes veľkosť paketu
    @param packets počet paketov
    @param is_tx true ak je paket odoslaný, false ak je prijatý
 */
void Stats::update(const string& src, const string& dst, const string& proto, int bytes, int packets, bool is_tx) {
    // zámok na synchronizáciu pre bezpečný prístup k štatistikám
    lock_guard<mutex> lock(mtx_); 
    // vytvorenie jedinečného kľúča pre mapu štatistík
    ConnectionKey key = {src, dst, proto};
    // prístup k štatistikám pre daný kľúč
    auto& conn = stats_map_[key];

    if (is_tx) {
        // aktualizácia štatistík pre odoslaný paket
        conn.tx_bytes += bytes;
        conn.tx_packets += packets;
        return;
    }
    else{
        // aktualizácia štatistík pre prijatý paket
        conn.rx_bytes += bytes;
        conn.rx_packets += packets;
    }
}

/**
    @brief Získanie snímky(snapshot) aktuálnych štatistík spôsobom bezpečným pre vlákna.
    @return snapshot štatistík
 */
unordered_map<ConnectionKey, ConnectionStats> Stats::get_stats_snapshot() {
    // zámok na synchronizáciu pre bezpečný prístup k štatistikám
    lock_guard<mutex> lock(mtx_);
    return stats_map_;
}
//====END OF stats.cpp ======
