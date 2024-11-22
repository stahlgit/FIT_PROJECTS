/**
    @file main.cpp
    @brief Hlavičkový súbor štatistík
    @author Peter Stahl (xstahl01)
*/
#ifndef STATS_H
#define STATS_H

#include <string>
#include <unordered_map>
#include <mutex>

using namespace std;

/**
    @brief Štruktúra reprezentujúca kľúč pre mapu štatistík
*/
struct ConnectionKey {
    string src;
    string dst;
    string proto;

    bool operator==(const ConnectionKey& other) const {
        return src == other.src && dst == other.dst && proto == other.proto;
    }
};

/**
    @brief Hash funkcia pre ConnectionKey
 */
namespace std {
    template <>
    struct hash<ConnectionKey> {
        size_t operator()(const ConnectionKey& k) const {
            return hash<std::string>()(k.src) ^ hash<std::string>()(k.dst) ^ hash<std::string>()(k.proto);
        }
    };
}

/**
    @brief Štruktúra reprezentujúca štatistiky pre jedno pripojenie
*/
struct ConnectionStats {
    double rx_bytes;
    double tx_bytes;
    double rx_packets;
    double tx_packets;
};

/**
    @brief Trieda zodpovedná za spracovanie štatistík
*/
class Stats {
public:
    /**
    @brief Metóda na aktualizáciu štatistík
    @param src zdrojová adresa
    @param dst cieľová adresa
    @param proto protokol
    @param bytes veľkosť paketu
    @param packets počet paketov
    @param is_tx true ak je paket odoslaný, false ak je prijatý
    */
    void update(const string& src, const string& dst, const string& proto, int bytes, int packets, bool is_tx);
    /**
    @brief Získanie snímky(snapshot) aktuálnych štatistík spôsobom bezpečným pre vlákna.
    @return snapshot štatistík
    */
    unordered_map<ConnectionKey, ConnectionStats> get_stats_snapshot();
private:
    /**
    @brief Mapa obsahujúca štatistiky pre jednotlivé pripojenia
     */
    unordered_map<ConnectionKey, ConnectionStats> stats_map_;
    /**
    @brief Mutex zámok pre synchronizáciu prístupu k štatistikám
     */
    mutex mtx_;
};

#endif 
//====END OF stats.h ======
