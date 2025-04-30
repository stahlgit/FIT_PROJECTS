#ifndef NDP_H

#define NDP_H

#include "console_utils.h"

#include <chrono>
#include <string>

using namespace std;


// Structure for NDP requests (IPv6 equivalent of ARP)
struct NDPRequest
{
    string host;                                // IP address
    chrono::steady_clock::time_point sent_time; // Time when the request was sent
    bool responded = false;                     // Whether a response was received
    string mac;                                 // MAC address of the host
};

/**
 * @brief Create an NDP socket
 * @return File descriptor of the NDP socket
 */
int create_ndp_socket();

/**
 * @brief Send an NDP request to the target IP address
 * @param sock File descriptor of the NDP socket
 * @param target_ip IP address to send the request to
 * @param iface_mac MAC address of the interface
 * @param iface_ip IP address of the interface
 * @param options Program options
 */
void send_ndp_request(int sock, const string &target_ip, const string &iface_mac, const string &iface_ip, const ProgramOptions &options);

/**
 * @brief Process NDP replies received on the NDP socket
 * @param sock File descriptor of the NDP socket
 * @param requests Vector of NDP requests
 */
void process_ndp_replies(int sock, vector<NDPRequest> &requests);

#endif // NDP_H