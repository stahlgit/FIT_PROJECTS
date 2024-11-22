#include "include/utils.h"
#include <cstring>
#include <getopt.h>
#include <ifaddrs.h>
#include <algorithm>
#include <sstream>


/**
    @brief Vypíše nápovedu na použitie programu
 */
void print_usage() {
    cout << "Usage: isa-top -i <interface> [-s b|p] [-t <interval>]\n";
    cout << "  -i <interface> : Specify the network interface to monitor.\n";
    cout << "  -s b|p         : Sort by bytes ('b') or packets ('p'). Default is 'b'.\n";
    cout << "  -t <interval>  : Set the interval in seconds for monitoring. Default is 1.\n";
}

/**
    @brief Zoznam dostupných sieťových rozhraní
    @return vektor názvov sieťových rozhraní
    inspiration: https://dev.to/fmtweisszwerg/cc-how-to-get-all-interface-addresses-on-the-local-device-3pki
    @author Fomalhaut Weisszwerg 
 */
vector<string> list_interfaces() {
    vector<string> interfaces;
    struct ifaddrs* ifaddr = nullptr;

    if (getifaddrs(&ifaddr) == -1) {
        cerr << "Error retrieving network interfaces: " << strerror(errno) << endl;
        return interfaces;
    }

    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) { //  list IPv4 rozhrania
            interfaces.emplace_back(ifa->ifa_name);
        }
    }

    freeifaddrs(ifaddr);
    return interfaces;
}

/**
    @brief Spracovanie argumentov programu
    @param argc počet argumentov
    @param argv pole argumentov
    @return konfiguračný objekt
 */
Config parse_arguments(int argc, char* argv[]) {
    Config config;
    int opt;
    if (argc <= 1 || argv == nullptr || argv[0] == nullptr) {
    throw invalid_argument("Invalid arguments passed to parse_arguments.");
}
    while ((opt = getopt(argc, argv, "i:s:t:")) != -1) {
        switch (opt) {
            case 'i':
                config.interface = optarg;
                break;
            case 's':
                if (optarg[0] == 'b' || optarg[0] == 'p') {
                    config.sort_option = optarg[0];
                } else {
                    throw invalid_argument("Invalid sort option. Use 'b' for bytes or 'p' for packets.");
                }
                break;
            case 't':
                try {
                    config.interval = stoi(optarg);
                    if (config.interval <= 0) throw invalid_argument("Interval must be positive.");
                } catch (const invalid_argument& e) {
                    throw invalid_argument("Invalid interval value.");
                }
                break;
            default:
                throw invalid_argument("Invalid argument.");
        }
    }
    if (config.interface.empty()) {
        cerr << "Error: No interface specified.\n";
        cerr << "Available interfaces:\n";
        vector<string> interfaces = list_interfaces();
        if (interfaces.empty()) {
            cerr << "  No available interfaces found.\n";
        } else {
            for (const auto& iface : interfaces) {
                cout << "  " << iface << endl;
            }
        }
        throw invalid_argument("Interface is required.");
    }
    else{
        //overenie či vstupné rozhranie existuje v zozname dostupných rozhraní
        vector<string> interfaces = list_interfaces();
        if (find(interfaces.begin(), interfaces.end(), config.interface) == interfaces.end()) {
            ostringstream oss;
            cerr << "Error: Specified interface '" << config.interface << "' is not valid.\n";
            cerr << "Available interfaces:\n";
            for (const auto& iface : interfaces) {
                oss << "  " << iface << "\n";
            }
            throw invalid_argument(oss.str());
        }
    }


    return config;
}