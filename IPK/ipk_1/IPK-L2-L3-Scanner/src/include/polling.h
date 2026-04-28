#ifndef POLLING_H

#define POLLING_H

#include "arp.h"
#include "icmp.h"
#include "icmpv6.h"
#include "ndp.h"

/**
 * @brief Poll the sockets for incoming packets
 * @param arp_sock File descriptor of the ARP socket
 * @param icmp_sock File descriptor of the ICMP socket
 * @param icmpv6_sock File descriptor of the ICMPv6 socket
 * @param ndp_sock File descriptor of the NDP socket
 * @param arp_requests Vector of ARP requests
 * @param icmp_requests Vector of ICMP requests
 * @param ndp_requests Vector of NDP requests
 * @param timeout Timeout in milliseconds
 */
void poll_sockets(int arp_sock, int icmp_sock, int icmpv6_sock, int ndp_sock,
                  vector<ARPRequest> &arp_requests, vector<ICMPRequest> &icmp_requests,
                  vector<NDPRequest> &ndp_requests, int timeout);

#endif // POLLING_H