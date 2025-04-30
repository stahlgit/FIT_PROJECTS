#ifndef ARP_H

#define ARP_H

#include "console_utils.h"

#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <chrono>
#include <string>

using namespace std;

// Structure representing an ARP packet (Ethernet + ARP headers)
struct ARPPacket
{
    struct ether_header eth_hdr; // Ethernet header
    struct ether_arp arp_pkt;    // ARP header
};

struct ARPRequest
{
    string host;                                // IP address
    chrono::steady_clock::time_point sent_time; // Time when the request was sent
    bool responded = false;                     // Whether a response was received
    string mac;                                 // MAC address of the host
};

/**
 * @brief Create a raw socket for ARP requests
 * @param interface Name of the interface to use
 * @return File descriptor of the created socket
 */
int create_arp_socket(const string &interface);

/**
 * @brief Send an ARP request to the target IP address
 * @param arp_sock File descriptor of the ARP socket
 * @param target_ip IP address to send the request to
 * @param iface_mac MAC address of the interface
 * @param iface_ip IP address of the interface
 * @param options Program options
 */
void send_arp_request(int arp_sock, const string &target_ip, const string &iface_mac, const string &iface_ip, const ProgramOptions &options);

/**
 * @brief Process ARP replies received on the ARP socket
 * @param arp_sock File descriptor of the ARP socket
 * @param requests Vector of ARP requests
 */
void process_arp_replies(int arp_sock, vector<ARPRequest> &requests);

#endif // ARP_H