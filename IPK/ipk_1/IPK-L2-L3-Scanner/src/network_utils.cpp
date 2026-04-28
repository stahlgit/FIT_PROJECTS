#include "include/network_utils.h"
#include <ifaddrs.h>
#include <arpa/inet.h> // #include <netinet/in.h this is inside
#include <linux/if_packet.h>
#include <iostream>
#include <bitset>

using namespace std;

bool is_valid_interface(const string &iface_name, const vector<NetworkInterface> &interfaces)
{
    for (const auto &iface : interfaces)
    {
        if (iface.name == iface_name)
            return true;
    }
    return false;
}

vector<NetworkInterface> get_active_interfaces()
{
    // Vector to store network interfaces
    vector<NetworkInterface> interfaces;
    struct ifaddrs *ifap, *ifa;

    // Get list of network interfaces
    if (getifaddrs(&ifap) == -1)
    {
        // if getifaddrs fails, print error and exit
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    // Iterating through the linked list of interface addresses
    for (ifa = ifap; ifa; ifa = ifa->ifa_next)
    {
        // Skip entries that don't have an address associated with them
        if (!ifa->ifa_addr)
        {
            continue;
        }

        // Get current interface name
        string iface_name = ifa->ifa_name;
        NetworkInterface *iface = nullptr;

        // Check if interface already exists
        for (auto &entry : interfaces)
        {
            if (entry.name == iface_name)
            {
                // If found, point to the existing entry
                iface = &entry;
                break;
            }
        }

        // Create new interface entry if not found
        if (!iface)
        {
            interfaces.push_back({iface_name, "", "", ""});
            iface = &interfaces.back();
        }

        // Get IPv4 address
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            char ip[INET_ADDRSTRLEN];
            // Convert binary IP to human readable format
            inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, ip, sizeof(ip));
            iface->ipv4 = ip;
        }
        // Get IPv6 address
        else if (ifa->ifa_addr->sa_family == AF_INET6)
        {
            char ip[INET6_ADDRSTRLEN];
            // Convert binary IP to human readable format
            inet_ntop(AF_INET6, &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr, ip, sizeof(ip));
            iface->ipv6 = ip;
        }
        else if (ifa->ifa_addr->sa_family == AF_PACKET)
        {
            // If the address family is AF_PACKET (used for link-layer information)
            struct sockaddr_ll *s = (struct sockaddr_ll *)ifa->ifa_addr;
            char mac[18];
            snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
                     s->sll_addr[0], s->sll_addr[1], s->sll_addr[2],
                     s->sll_addr[3], s->sll_addr[4], s->sll_addr[5]);
            iface->mac = mac;
        }
    }
    // free memonry and return the list of interfaces
    freeifaddrs(ifap);
    return interfaces;
}

uint16_t compute_checksum_ipv4(void *buf, int len)
{
    // Cast the input buffer to a pointer to uint16_t, as we'll be processing data in 16-bit chunks
    uint16_t *data = (uint16_t *)buf;
    // Initialize sum to accumulate the 16-bit one's complement sum
    uint32_t sum = 0;

    // Iterate through the buffer as long as there are at least two bytes remaining
    for (; len > 1; len -= 2)
    {
        // Add current 16-bit chunk to the sum
        sum += *data++;
    }

    // If there's a single byte left, pad it with zero and add it to the sum
    if (len == 1)
    {
        sum += *(uint8_t *)data;
    }
    // Performing the "fold" operation to reduce the 32-bit sum to a 16-bit checksum. (This involves adding the carry (if any) to the lower 16 bits of the sum.)
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    return ~sum;
}

vector<string> expand_subnet(const string &cidr)
{
    size_t slash_pos = cidr.find('/');

    if (slash_pos == string::npos)
    {
        cerr << "Invalid CIDR format: " << cidr << endl;
        return {};
    }
    // Extracting base IP from CIDR notation
    string base_ip = cidr.substr(0, slash_pos);
    int prefix;
    try
    {
        // Extracting prefix from CIDR notation
        prefix = stoi(cidr.substr(slash_pos + 1));
    }
    catch (const exception &e)
    {
        cerr << "Invalid prefix: " << cidr.substr(slash_pos + 1) << endl;
        return {};
    }

    // IPv4 handling
    if (base_ip.find(':') == string::npos)
    {
        struct in_addr net_ip;
        if (inet_pton(AF_INET, base_ip.c_str(), &net_ip) != 1)
        {
            cerr << "Invalid IPv4 address: " << base_ip << endl;
            return {};
        }

        if (prefix < 0 || prefix > 32)
        {
            cerr << "Invalid IPv4 prefix: " << prefix << endl;
            return {};
        }
        // Convert network byte order to host byte order
        uint32_t net_addr = ntohl(net_ip.s_addr);
        // Computing subnet mas based on prefix length
        uint32_t mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;
        // First usable host in the subnet
        uint32_t first_host = (net_addr & mask) + 1;
        // Last usable host in the subnet
        uint32_t last_host = (net_addr | ~mask) - 1;

        vector<string> hosts;
        for (uint32_t ip = first_host; ip <= last_host; ip++)
        {
            struct in_addr addr;
            // Converting back to network byte order
            addr.s_addr = htonl(ip);
            char ip_str[INET_ADDRSTRLEN];
            // Convert binary IP to human readable format
            inet_ntop(AF_INET, &addr, ip_str, sizeof(ip_str));
            // Add this IP to lists of hosts
            hosts.push_back(ip_str);
        }
        return hosts;
    }
    // IPv6 handling
    else
    {
        // Handle IPv6 subnets similarly using bit manipulation
        struct in6_addr net_ip;
        // Check if the provided base IP address is a valid IPv6 address
        if (inet_pton(AF_INET6, base_ip.c_str(), &net_ip) != 1)
        {
            // If inet_pton fails, it means the IP address is not a valid IPv6 format
            cerr << "Invalid IP address: " << base_ip << endl;
            return {}; // return empty vector to indicate failure
        }
        // Check if the provided prefix length is within the valid range for IPv6 (0 to 128)
        if (prefix < 0 || prefix > 128)
        {
            cerr << "Invalid IPv6 prefix length: " << prefix << endl;
            return {};
        }
        // Convert the IPv6 address to a bitset for easier manipulation
        bitset<128> net_bits;
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                // Extract each bit from the byte and set it in the bitset.
                // The bitset is indexed from the least significant bit (index 0) to the most significant (index 127).
                // We iterate through the bytes (i from 0 to 15) and then through the bits within each byte (j from 0 to 7).
                // The formula (15 - i) * 8 + j calculates the correct bit position in the 128-bit bitset.
                net_bits[(15 - i) * 8 + j] = (net_ip.s6_addr[i] >> (7 - j)) & 1;
            }
        }

        // Computing the network range

        /* Create a bitmask to isolate the network portion of the IP address.
            ~bitset(0) creates a bitset with all bits set to 1.
            << (128 - prefix) shifts this bitset to the left by (128 - prefix) positions,
            effectively creating a mask where the first 'prefix' bits are 1 and the rest are 0. */
        bitset<128> mask = (~bitset<128>(0)) << (128 - prefix);
        /*Calculate the network start address by performing a bitwise AND operation between the network IP (in bits) and the mask.
            This keeps only the network bits and sets the host bits to 0.*/
        bitset<128> net_start = net_bits & mask;
        /* Calculate the network end address by performing a bitwise OR operation between the network IP (in bits) and the inverse of the mask.
            The inverse of the mask has the first 'prefix' bits as 0 and the rest as 1.
            ORing with this effectively sets all the host bits to 1. */
        bitset<128> net_end = net_bits | ~mask;
        // Vector to store the generated host IP addresses
        vector<string> hosts;
        // Initialize a bitset 'current' with the network start address. This will be incremented to generate each host IP.
        bitset<128> current = net_start;

        // Loop to iterate through all the host addresses within the subnet
        while (true)
        {
            // Structure to hold the IPv6 address for the current iteration
            struct in6_addr addr = {};
            for (int i = 0; i < 16; ++i)
            {
                uint8_t byte = 0;
                for (int j = 0; j < 8; ++j)
                {
                    // Extract each bit from the bitset and set the corresponding bit in the byte.
                    byte |= current[(15 - i) * 8 + j] << (7 - j);
                }
                addr.s6_addr[i] = byte;
            }
            // Buffer to store the string representation of the IPv6 address
            char ip_str[INET6_ADDRSTRLEN];
            // Convert the binary IPv6 address to a human-readable string format
            inet_ntop(AF_INET6, &addr, ip_str, sizeof(ip_str));
            // Add the generated IP address string to the vector of hosts
            hosts.push_back(ip_str);

            // Check if the current IP address is equal to the network end address.
            // If it is, we have reached the end of the subnet.
            if (current == net_end)
            {
                break;
            }

            // Increment the current IP address to get the next one in the subnet
            bool carry = true;
            for (int i = 0; i < 128 && carry; ++i)
            {
                bool prev = current[i];
                current[i] = prev ^ carry; // Toggle the bit if there's a carry
                carry = prev && carry;     // Carry remains true only if both prev and carry are true
            }
        }
        return hosts; // return the vector of generated host IP addresses
    }
}
