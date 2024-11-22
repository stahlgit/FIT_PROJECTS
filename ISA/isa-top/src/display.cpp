/**
    @file display.cpp
    @brief Súbor obsahujúci implementáciu triedy Display, ktorá je zodpovedná za zobrazovanie štatistík zachytených paketov na obrazovke 
    @author Peter Stahl (xstahl01)
*/
#include "include/display.h"
#include "include/stats.h"
#include <ncurses.h>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iomanip>


/**
    @brief Konštruktor triedy Display 
    @param stats referencia na objekt triedy Stats
    @param sort_option zvolená možnosť zoradenia
    @param refresh_interval interval obnovovania obrazovky
    @param running flag pre indikáciu, či je zobrazovací loop spustený
*/
Display::Display(Stats& stats, char sort_option, int refresh_interval, bool running)
    : stats_(stats), sort_option_(sort_option), refresh_interval_(refresh_interval), running_(running) {
}

/**
    @brief Deštruktor triedy Display
*/
Display::~Display() {
    if (display_thread_.joinable()) {
        // Počka na dokončenie vlákna zobrazenia
        display_thread_.join();
    }
    // zatvorenie okna ncurses
    endwin();
}

/**
    @brief Spustenie zobrazovania
 */
void Display::run() {
    // Inicializácia ncurses 
    initscr();
    // Zakáže ukladanie riadkov do vyrovnávacej pamäte, čím sa znaky napísané používateľom okamžite sprístupnia
    cbreak();
    // Zakázať ozvenu znakov napísaných používateľom
    noecho();
    // Skryť kurzor 
    curs_set(FALSE);
    // Povolenie čítania funkčných klávesov
    nodelay(stdscr, TRUE); //umožňuje používať getch() bez blokovania

    // Spustenie zobrazovacieho loop v samostatnom vlákne;
    display_thread_ = thread(&Display::display_loop, this);
}

/**
    @brief Zastavenie zobrazovania
 */
void Display::stop() {
    if (display_thread_.joinable()) {
        // Počka na dokončenie vlákna zobrazenia

        display_thread_.join();
    }
    // End ncurses mode
    endwin();}

/**
    @brief formátovanie bytov na ľudsky čitateľný formát
    @param bytes počet bytov
    @return formátovaný reťazec
 */
string format_bytes(double bytes) {
    const char* suffixes[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    while (bytes >= 1024 && i < 4) {
        bytes /= 1024;
        i++;
    }
    ostringstream oss;
    oss << fixed << setprecision(1) << bytes << " " << suffixes[i];
    return oss.str();
}

/**
    @brief formátovanie packetov na ľudsky čitateľný formát
    @param packets počet packetov
    @return formátovaný reťazec
 */
string format_packets(double packets) {
    ostringstream oss;
    if (packets >= 1000){
        oss << fixed << setprecision(1) << packets / 1000 << " K";
    }
    else{
        oss << fixed << setprecision(1) << packets;
    }
    return oss.str();
}

/**
    @brief Rozdelí reťazec podľa zadaného oddeľovača
    @param str reťazec na rozdelenie
    @param delimiter oddeľovač
    @return vektor reťazcov
 */
vector<string> split(const string& str, char delimiter) {
    vector<string> tokens;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


/**
    @brief Nepretržite zobrazuje štatistiku siete v slučke, kým sa nezastaví alebo neukončí vstupom používateľa.
 */
void Display::display_loop() {
    // rozmery stĺpcov
    constexpr int col_width_src = 25;
    constexpr int col_width_dst = 25;
    constexpr int col_width_proto = 10;
    constexpr int col_width_rx = 15;
    constexpr int col_width_tx = 15;

    while (running_) {
        clear();
        if (check_exit()) {
            running_ = false;
            break; 
        }

        display_header(col_width_src, col_width_dst, col_width_proto, col_width_rx, col_width_tx);

        auto connections = get_sorted_connections();

        display_connections(connections, col_width_src, col_width_dst, col_width_proto, col_width_rx, col_width_tx);

        refresh();
        this_thread::sleep_for(chrono::seconds(refresh_interval_));
    }
}

/**
    @brief Kontrola vstupu používateľa
    @return true ak používateľ stlačil klávesu 'q', inak false
 */
bool Display::check_exit() {
    int ch = getch();
    return ch == 'q';
}

/**
    @brief Zobrazí hlavičku tabuľky 
    @param col_width_src šírka stĺpca pre zdrojovú IP adresu
    @param col_width_dst šírka stĺpca pre cieľovú IP adresu
    @param col_width_proto šírka stĺpca pre protokol
    @param col_width_rx šírka stĺpca pre prijaté dáta
    @param col_width_tx šírka stĺpca pre odoslané dáta
 */
void Display::display_header(int col_width_src, int col_width_dst, int col_width_proto, int col_width_rx, int col_width_tx) {
    // Zobrazenie hlavičky tabuľky v bytoch 
    if (sort_option_ == 'b') {
        mvprintw(0, 0, "%-*s %-*s %-*s %-*s %-*s", 
            col_width_src, "Src IP:port",
            col_width_dst, "Dst IP:port",
            col_width_proto, "Proto",
            col_width_rx, "Rx (b/s)",
            col_width_tx, "Tx (b/s)");
    }
    // Zobrazenie hlavičky tabuľky v packetoch
    else if (sort_option_ == 'p') {
        mvprintw(0, 0, "%-*s %-*s %-*s %-*s %-*s", 
            col_width_src, "Src IP:port",
            col_width_dst, "Dst IP:port",
            col_width_proto, "Proto",
            col_width_rx, "Rx (p/s)",
            col_width_tx, "Tx (p/s)");
    }
}


/**
    @brief Získa zoradený zoznam pripojení podľa zvoleného kritéria
    @return zoradený zoznam pripojení
 */
vector<pair<ConnectionKey, ConnectionStats>> Display::get_sorted_connections() {
    auto snapshot = stats_.get_stats_snapshot();
    unordered_map<string, ConnectionStats> merged_connections;

    // Merge revezného toku dát
    for (const auto& [key, stats] : snapshot) {
        // Vytvorenie kombinovaného kľúča (nezávisle od smeru)
        string merged_key = key.src < key.dst 
                            ? key.src + "|" + key.dst + "|" + key.proto 
                            : key.dst + "|" + key.src + "|" + key.proto;

        if (merged_connections.find(merged_key) == merged_connections.end()) {
            merged_connections[merged_key] = stats;
        } else {
            merged_connections[merged_key].rx_bytes += stats.rx_bytes;
            merged_connections[merged_key].tx_bytes += stats.tx_bytes;
            merged_connections[merged_key].rx_packets += stats.rx_packets;
            merged_connections[merged_key].tx_packets += stats.tx_packets;
        }
    }

    // Konverzia mapy na vektor pre zoradenie
    vector<pair<ConnectionKey, ConnectionStats>> connections;
    for (const auto& [merged_key, stats] : merged_connections) {
        auto parts = split(merged_key, '|'); 
        ConnectionKey combined_key{parts[0], parts[1], parts[2]};
        connections.emplace_back(combined_key, stats);
    }

    // Sort connections based on selected criteria
    if (sort_option_ == 'b') {
        sort(connections.begin(), connections.end(), [](const pair<ConnectionKey, ConnectionStats>& a, const pair<ConnectionKey, ConnectionStats>& b) {
            return a.second.rx_bytes + a.second.tx_bytes > b.second.rx_bytes + b.second.tx_bytes;
        });
    } else if (sort_option_ == 'p') {
        sort(connections.begin(), connections.end(), [](const pair<ConnectionKey, ConnectionStats>& a, const pair<ConnectionKey, ConnectionStats>& b) {
            return a.second.rx_packets + a.second.tx_packets > b.second.rx_packets + b.second.tx_packets;
        });
    }

    return connections;
}

/**
    @brief Zobrazí jednotlivé štatistky pripojení v formátovaných stĺpcoch
 */
void Display::display_connections(const vector<pair<ConnectionKey, ConnectionStats>>& connections, int col_width_src, int col_width_dst, int col_width_proto, int col_width_rx, int col_width_tx) {
    const int max_display_count = 10;
    
    for (int count = 0; count < min(static_cast<int>(connections.size()), max_display_count); ++count) {
        const auto& [key, stats] = connections[count];
        
        double rx_bps = stats.rx_bytes / refresh_interval_;
        double rx_pps = stats.rx_packets / refresh_interval_;
        double tx_bps = stats.tx_bytes / refresh_interval_;
        double tx_pps = stats.tx_packets / refresh_interval_;

        if (sort_option_ == 'b') {
            mvprintw(2 + count, 0, "%-*s %-*s %-*s %-*s %-*s",
                col_width_src, key.src.c_str(),
                col_width_dst, key.dst.c_str(),
                col_width_proto, key.proto.c_str(),
                col_width_rx, format_bytes(rx_bps).c_str(),
                col_width_tx, format_bytes(tx_bps).c_str());
        } else if (sort_option_ == 'p') {
            mvprintw(2 + count, 0, "%-*s %-*s %-*s %-*s %-*s",
                col_width_src, key.src.c_str(),
                col_width_dst, key.dst.c_str(),
                col_width_proto, key.proto.c_str(),
                col_width_rx, format_packets(rx_pps).c_str(),
                col_width_tx, format_packets(tx_pps).c_str());
        }
    }
}
//====END OF display.cpp ======
