#ifndef OUTPUT_H

#define OUTPUT_H

#include "arp.h"
#include "icmp.h"
#include "ndp.h"

/**
 * @brief Print the output of the program
 * @param ipv4_hosts List of IPv4 hosts
 * @param ipv6_hosts List of IPv6 hosts
 * @param arp_requests List of ARP requests
 * @param icmp_requests List of ICMP requests
 * @param ndp_requests List of NDP requests
 * @param iface_mac MAC address of the interface
 * @param iface_ipv4 IPv4 address of the interface
 * @param iface_ipv6 IPv6 address of the interface
 */
void print_output(vector<string> &ipv4_hosts, vector<string> &ipv6_hosts, vector<ARPRequest> &arp_requests, vector<ICMPRequest> &icmp_requests, vector<NDPRequest> &ndp_requests,
                  NetworkInterface &iface);

#endif // OUTPUT_H