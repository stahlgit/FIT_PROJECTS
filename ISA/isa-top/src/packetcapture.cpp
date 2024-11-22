/**
    @file packetcapture.cpp
    @brief Implementácia triedy PacketCapture, ktorá je zodpovedná za zachytávanie paketov zo sieťového rozhrania
    @author Peter Stahl (xstahl01)
*/
#include "include/packetcapture.h"
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <iostream>

#include <netdb.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <net/if.h>

string global_interface; // globalna premenna pre rozhranie 


/**
    @brief Funkcia na získanie IP adresy zadaného rozhrania
    @param interface názov rozhrania
    @return IP adresa rozhrania
    inspiration: https://dev.to/fmtweisszwerg/cc-how-to-get-all-interface-addresses-on-the-local-device-3pki
    @author Fomalhaut Weisszwerg 
*/
string getInterfaceIPAddress(const string& interface = global_interface) {
    struct ifaddrs* ifaddr;
    struct ifaddrs* ifa;
    char ip[INET6_ADDRSTRLEN];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs"); // chyba pri získavaní informácií o rozhraniach
        return "";
    }

    string local_ip;

    // iterovanie cez zoznam rozhraní
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) {
            continue; // preskočiť, ak rozhranie nemá priradenú IP adresu
        }

        // Porovnajte názov rozhrania
        if (interface == ifa->ifa_name) {
            // IPv4 adresa
            if (ifa->ifa_addr->sa_family == AF_INET) {
                struct sockaddr_in* sa = (struct sockaddr_in*)ifa->ifa_addr;
                inet_ntop(AF_INET, &(sa->sin_addr), ip, INET_ADDRSTRLEN);
                local_ip = ip;
                break;
            }
        }
    }

    // uvoľnenie pamäte alokovanej z getifaddrs
    freeifaddrs(ifaddr);

    if (local_ip.empty()) {
        throw invalid_argument("Interface " + interface + " not found or has no IP address.");
    }

    return local_ip;
}


/**
    @brief Konštruktor triedy PacketCapture
    @param interface názov sieťového rozhrania
    @param stats referencia na objekt triedy Stats
*/
PacketCapture::PacketCapture(const string& interface, Stats& stats)
    : interface_(interface), stats_(stats), handle_(nullptr) {
        char errbuf[PCAP_ERRBUF_SIZE];
        global_interface = interface; // nastavenie globálnej premennej
        handle_ = pcap_open_live(interface_.c_str(), BUFSIZ, 1, 1000, errbuf); // otvorenie zariadenia pre zachytávanie paketov
        
        if (handle_ == nullptr) { 
            cerr << "Error opening device " << interface_ << ": " << errbuf << endl;
            exit(1); // chyba pri otváraní zariadenia
        }
    }


/**
    @brief Destruktor triedy PacketCapture
 */
PacketCapture::~PacketCapture() {
    if (handle_ != nullptr) {
        pcap_breakloop(handle_); 
    }
}


/**
    @brief Metóda na spustenie zachytávania paketov
 */
void PacketCapture::start_capture() {
    pcap_loop(handle_, 0, PacketCapture::packet_handler, reinterpret_cast<u_char*>(&stats_));
}


/**
    @brief Metóda na spracovanie zachyteného paketu
    @param user pointer na objekt triedy Stats
    @param header pointer na hlavičku paketu
    @param packet pointer na zachytený paket
 */
void PacketCapture::packet_handler(u_char *user, const struct pcap_pkthdr *header, const u_char *packet) {
    Stats* stats = reinterpret_cast<Stats*>(user); // Prenesenie používateľských údajov späť do ukazovateľa štatistiky

    ether_header *eth_header;

    eth_header = (struct ether_header *) packet;

    if (ntohs(eth_header->ether_type) != 0x0800) { // IPv4 check
        return;
    }

    // IP hlavička z packatu, zdrojová a cieľová IP adresa
    struct ip* ip_header = (struct ip*)(packet + sizeof(struct ether_header));
    string src_ip = inet_ntoa(ip_header->ip_src);
    string dst_up = inet_ntoa(ip_header->ip_dst);
    int ip_header_len = ip_header->ip_hl * 4;

    string protocol;
    string scr_port = "-";
    string dst_port = "-";

    // TCP, UDP, ICMP alebo iný protokol
    if (ip_header->ip_p == IPPROTO_TCP){
        protocol = "tcp";
        struct tcphdr* tcp_header = (struct tcphdr*)(packet + sizeof(struct ether_header) + ip_header_len);
        scr_port = to_string(ntohs(tcp_header->th_sport));
        dst_port = to_string(ntohs(tcp_header->th_dport));
    } 
    else if (ip_header->ip_p == IPPROTO_UDP) {
        protocol = "udp";
        struct udphdr* udp_header = (struct udphdr*)(packet + sizeof(struct ether_header) + ip_header_len);
        scr_port = to_string(ntohs(udp_header->uh_sport));
        dst_port = to_string(ntohs(udp_header->uh_dport));
    } 
    else if (ip_header->ip_p == IPPROTO_ICMP) {
        protocol = "icmp";
    } 
    else {
        protocol = "other";
    }

    string src = src_ip + ":" + scr_port;
    string dst = dst_up + ":" + dst_port;

    string local_ip = getInterfaceIPAddress();

    if (src_ip == local_ip){
        // Transmitted (Tx)
        stats->update(src, dst, protocol, header->len, 1, true); 
    }
    else if (dst_up == local_ip){
        // Received (Rx)
        stats->update(src, dst, protocol, header->len, 1, false); //packetsize = header->len
    }
}

/**
    @brief Metóda na zastavenie zachytávania paketov
 */
void PacketCapture::stop_capture() {
    if (handle_ != nullptr) {
        pcap_breakloop(handle_);
    }}
//====END OF packetcapture.cpp ======