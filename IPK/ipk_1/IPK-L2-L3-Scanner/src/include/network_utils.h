#ifndef NETWORK_UTILS_H

#define NETWORK_UTILS_H

#include <string>
#include <vector>
#include <cstdint>

using namespace std;

// Structure representing a network interface
struct NetworkInterface
{
    string name;
    string ipv4;
    string ipv6;
    string mac;
};

/**
 * @brief Check if the given interface name is valid
 * @param iface_name Interface name
 * @param interfaces List of network interfaces
 * @return True if the interface is valid, false otherwise
 */
bool is_valid_interface(const string &iface_name, const vector<NetworkInterface> &interfaces);

/**
 * @brief Compute the checksum of an IPv4 packet
 * @param buf Pointer to the packet
 * @param len Length of the packet
 */
uint16_t compute_checksum_ipv4(void *buf, int len);

/**
 * @brief get the vecotr of active network interfaces
 * @return vector of active network interfaces
 */
vector<NetworkInterface> get_active_interfaces();

/**
 * @brief Expand a CIDR notation subnet to a list(vector) of IP addresses
 * @param cidr CIDR notation subnet
 * @return List(vector) of IP addresses in the subnet
 */
vector<string> expand_subnet(const string &cidr);

#endif // NETWORK_UTILS_H