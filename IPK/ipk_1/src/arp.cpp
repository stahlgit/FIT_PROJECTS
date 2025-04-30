#include "include/arp.h"

#include <netinet/in.h>
#include <net/if.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>

int create_arp_socket(const string &interface)
{
    // create raw socket
    // AF_PACKET: Address family for packet sockets
    // SOCK_RAW: Socket type for raw sockets, allowing direct access to the link layer
    // htons(ETH_P_ARP): Protocol type for Ethernet frames, specifically for ARP (Address Resolution Protocol)
    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    // check if socket was created successfully
    if (sock < 0)
    {
        perror("ARP socket");
        return -1;
    }

    // Bind the socket to a specific network interface
    struct ifreq ifr;
    // Initializing the ifreq structure to all zeros
    memset(&ifr, 0, sizeof(ifr));
    // Coping the interface name to the ifreq structure, ensuring it doesn't exceed the maximum size
    strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);
    // Set the socket option to bind the socket to the specified device
    if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0)
    {
        // If the setsockopt call fails, print an error message and close the socket
        perror("ARP bind");
        close(sock);
        return -1;
    }
    return sock;
}

void send_arp_request(int arp_sock, const string &target_ip, const string &iface_mac,
                      const string &iface_ip, const ProgramOptions &options)
{
    ARPPacket pkt;

    // Ethernet header
    // Set the destination MAC address to the broadcast address (FF:FF:FF:FF:FF:FF)
    memset(pkt.eth_hdr.ether_dhost, 0xFF, ETH_ALEN); // Broadcast
    // Parsing the interface MAC address string and populate the source MAC address in the Ethernet header
    sscanf(iface_mac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &pkt.eth_hdr.ether_shost[0], &pkt.eth_hdr.ether_shost[1],
           &pkt.eth_hdr.ether_shost[2], &pkt.eth_hdr.ether_shost[3],
           &pkt.eth_hdr.ether_shost[4], &pkt.eth_hdr.ether_shost[5]);
    // Set the Ethernet type to ETHERTYPE_ARP (ARP protocol)
    pkt.eth_hdr.ether_type = htons(ETHERTYPE_ARP);

    // ARP header
    /*SET:  the hardware type to Ethernet
            the protocol type to IPv4
            hardware address length to the length of a MAC address (6 bytes)
            protocol address length to the length of an IPv4 address (4 bytes)
            operation to ARP request
    */
    pkt.arp_pkt.arp_hrd = htons(ARPHRD_ETHER);
    pkt.arp_pkt.arp_pro = htons(ETHERTYPE_IP);
    pkt.arp_pkt.arp_hln = ETH_ALEN;
    pkt.arp_pkt.arp_pln = 4;
    pkt.arp_pkt.arp_op = htons(ARPOP_REQUEST);

    // Copy MAC and IP addresses
    sscanf(iface_mac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &pkt.arp_pkt.arp_sha[0], &pkt.arp_pkt.arp_sha[1],
           &pkt.arp_pkt.arp_sha[2], &pkt.arp_pkt.arp_sha[3],
           &pkt.arp_pkt.arp_sha[4], &pkt.arp_pkt.arp_sha[5]);

    // Converting the interface IP address string to a binary IPv4 address and populate the sender protocol address (SPA)
    inet_pton(AF_INET, iface_ip.c_str(), pkt.arp_pkt.arp_spa);
    // Set the target hardware address (THA) to all zeros as we don't know it yet
    memset(pkt.arp_pkt.arp_tha, 0, ETH_ALEN); // Target MAC unknown
    // Convert the target IP address string to a binary IPv4 address and populate the target protocol address (TPA)
    inet_pton(AF_INET, target_ip.c_str(), pkt.arp_pkt.arp_tpa);

    // Send packet
    /* SET: the address family to AF_PACKET (packet interface) IPv4
            the protocol to ETH_P_ARP (ARP protocol)
            interface index to the index of the specified interface
            hardware address length to the length of a MAC address (6 bytes)
    */
    struct sockaddr_ll addr{};
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ARP);
    addr.sll_ifindex = if_nametoindex(options.interface.c_str());
    addr.sll_halen = ETH_ALEN;

    sendto(arp_sock, &pkt, sizeof(pkt), 0, (struct sockaddr *)&addr, sizeof(addr));
}

void process_arp_replies(int arp_sock, vector<ARPRequest> &requests)
{
    // Buffer to store the received packet data
    unsigned char buffer[ETH_FRAME_LEN];
    // Structure to store the source address information
    struct sockaddr_ll src_addr;
    /// Variable to store the length of the source address structure
    socklen_t addr_len = sizeof(src_addr);

    // Receive data from ARP socket
    ssize_t len = recvfrom(arp_sock, buffer, sizeof(buffer), 0,
                           (struct sockaddr *)&src_addr, &addr_len);
    // if no data was received or the packet is smaller than the size of an ARP header, return
    if (len < sizeof(struct ether_arp))
        return;
    // Extract the ARP header from the received packet
    struct ether_arp *arp = (struct ether_arp *)(buffer + sizeof(struct ether_header));
    // Check if the received ARP operation is a reply
    if (ntohs(arp->arp_op) != ARPOP_REPLY)
        return;

    // Buffer to store the string representation of the sender's IP address
    char ip_str[INET_ADDRSTRLEN];
    // Convert the binary IPv4 address to a human-readable string format
    inet_ntop(AF_INET, arp->arp_spa, ip_str, sizeof(ip_str));

    // Iterate through the list of ARP requests
    for (auto &req : requests)
    {
        // If the IP address in the reply matches a requested IP and the request hasn't been responded to yet
        if (req.host == ip_str && !req.responded)
        {
            char mac[18];
            snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
                     arp->arp_sha[0], arp->arp_sha[1], arp->arp_sha[2],
                     arp->arp_sha[3], arp->arp_sha[4], arp->arp_sha[5]);
            req.mac = mac;
            req.responded = true;
        }
    }
}