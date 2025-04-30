#ifndef CONSOLE_UTILS_H

#define CONSOLE_UTILS_H

#include "network_utils.h"

#include <vector>
#include <iostream>

using namespace std;

// Structure to hold program options from the command line
struct ProgramOptions
{
    string interface = "";  // -i Network interface (default: empty)
    int timeout = 5000;     // -w Timeout in milliseconds (default: 5000)
    vector<string> subnets; // -s List of subnets to scan
};

/**
 * @brief Print the available network interfaces
 * @param interfaces List of network interfaces
 */
void print_interfaces(const vector<NetworkInterface> &interfaces);

/**
 * @brief Print the input arguments
 * @param options Program options
 */
void print_input_arguments(ProgramOptions options);


/**
 * @brief Print the scanning ranges
 * @param subnets List of subnets
 */
void print_scanning_ranges(const vector<string>& subnets);


/**
 * Parse the input arguments
 * @param argc Number of arguments
 * @param argv List of arguments
 * @return Program options
 */
ProgramOptions parse_arguments(int argc, char *argv[]);


/**
 * @brief Print the error message for the failed socket
 * @param arp_sock ARP socket
 * @param icmp_sock ICMP socket
 * @param ndp_sock NDP socket
 * @param icmpv6_sock ICMPv6 socket
 */
void print_socket_error(int arp_sock, int icmp_sock, int ndp_sock, int icmpv6_sock);
#endif // CONSOLE_UTILS_H
