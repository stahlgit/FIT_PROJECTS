#ifndef ICMPV6_H

#define ICMPV6_H

#include "icmp.h" // import ICMPRequest

/**
 * @brief Create an ICMPv6 socket
 * @return File descriptor of the ICMPv6 socket
 */
int create_icmpv6_socket();

/**
 * @brief Send an ICMPv6 request to the target IP address
 * @param sock File descriptor of the ICMPv6 socket
 * @param host IP address to send the request to
 * @param seq ICMP sequence number
 */
void send_icmpv6_request(int sock, const string &host, uint16_t seq);

/**
 * @brief Process ICMPv6 replies received on the ICMPv6 socket
 * @param sock File descriptor of the ICMPv6 socket
 * @param requests Vector of ICMPv6 requests
 */
void process_icmpv6_replies(int sock, vector<ICMPRequest> &requests);

#endif // ICMPV6_H