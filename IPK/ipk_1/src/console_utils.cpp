#include "include/console_utils.h"
#include "include/network_utils.h"
#include <getopt.h>

using namespace std;

void print_interfaces(const vector<NetworkInterface> &interfaces)
{
    cout << "Available Interfaces: " << endl;
    for (const auto &iface : interfaces)
    {
        cout << " - Name: " << iface.name << endl;
        cout << "   IPv4: " << iface.ipv4 << endl;
        cout << "   IPv6: " << iface.ipv6 << endl;
        cout << "   MAC: " << iface.mac << endl;
    }
}

void print_input_arguments(ProgramOptions options)
{
    cout << "Selected Interface: " << (options.interface.empty() ? "None (list interfaces)" : options.interface) << endl;
    cout << "Timeout: " << options.timeout << " ms" << endl;
    cout << "Subnets: " << endl;
    for (const string &subnet : options.subnets)
    {
        cout << " - " << subnet << endl;
    }
}

void print_scanning_ranges(const vector<string>& subnets) {
    cout << "Scanning ranges:\n";
    for (const auto& subnet : subnets) {
        auto hosts = expand_subnet(subnet);
        cout << subnet << " " << hosts.size() << "\n";
    }
    cout << endl;
}


ProgramOptions parse_arguments(int argc, char *argv[])
{
    ProgramOptions options;
    // parsing for short and long options
    static struct option long_options[] = {
        {"interface", required_argument, 0, 'i'},
        {"wait", required_argument, 0, 'w'},
        {"subnets", required_argument, 0, 's'},
        {nullptr, 0, nullptr, 0}};

    int opt;
    while ((opt = getopt_long(argc, argv, "i:w:s:", long_options, nullptr)) != -1)
    {
        switch (opt)
        {
        case 'i':
            options.interface = optarg;
            break;
        case 'w':
            options.timeout = stoi(optarg);
            break;
        case 's':
            options.subnets.push_back(optarg);
            break;
        default:
            cerr << "Usage: " << argv[0] << " [-i interface | --interface interface ] {-w timeout | --wait timeout} [-s ipv4-subnet | -s ipv6-subnet | --subnet ipv4-subnet | --subnet ipv6-subnet]" << endl;
            exit(EXIT_FAILURE);
        }
    }
    return options;
}

void print_socket_error(int arp_sock, int icmp_sock, int ndp_sock, int icmpv6_sock)
{
    if (icmp_sock < 0)
    {
        cerr << "Error creating ICMP socket" << endl;
    }
    if (icmpv6_sock < 0)
    {
        cerr << "Error creating ICMPv6 socket" << endl;
    }
    if (ndp_sock < 0)
    {
        cerr << "Error creating NDP socket" << endl;
    }
    if (arp_sock < 0)
    {
        cerr << "Error creating ARP socket" << endl;
    }
}