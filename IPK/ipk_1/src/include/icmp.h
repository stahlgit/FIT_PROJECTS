#ifndef ICMP_H

#define ICMP_H

#include "network_utils.h"
#include <chrono>

using namespace std;

// Structure representing an ICMP request
struct ICMPRequest
{
    string host;                                // Targer Host - IP address
    uint16_t sequence_number;                   // Sequence number of tracking replies
    chrono::steady_clock::time_point sent_time; // Time when the request was sent
    bool is_ipv6 = false;                       // Whether the request is for IPv6
};

/**
 * @brief Create an ICMP socket
 * @return File descriptor of the ICMP socket
 */
int create_icmp_socket();

/**
 * @brief Send an ICMP request to the target IP address
 * @param sock File descriptor of the ICMP socket
 * @param host IP address to send the request to
 * @param sequence ICMP sequence number
 */
void send_icmp_request(int sock, const string &host, uint16_t sequence);

/**
 * @brief Process ICMP replies received on the ICMP socket
 * @param sock File descriptor of the ICMP socket
 * @param requests Vector of ICMP requests
 */
void process_icmp_replies(int sock, vector<ICMPRequest> &requests);

#endif // ICMP_H