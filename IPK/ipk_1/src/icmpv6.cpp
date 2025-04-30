#include "include/icmpv6.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/icmp6.h>
#include <algorithm>

int create_icmpv6_socket()
{
    // create raw socket
    // AF_INET6: Address family for IPv6
    // SOCK_RAW: Socket type for raw sockets, allowing direct access to the IP layer
    // IPPROTO_ICMPV6: Protocol type for ICMPv6 (Internet Control Message Protocol for IPv6)
    int sock = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    // check if socket was created successfully
    if (sock < 0)
    {
        perror("ICMPv6 socket");
        return -1;
    }
    // return file descriptor of the ICMPv6 socket
    return sock;
}

void send_icmpv6_request(int sock, const string &host, uint16_t seq)
{
    // Structure to store the destination address information
    struct sockaddr_in6 dest_addr{};
    // Set the address family to AF_INET6 (IPv6)
    dest_addr.sin6_family = AF_INET6;
    // Convert the target IP address string to a binary IPv6 address and populate the destination address structure
    inet_pton(AF_INET6, host.c_str(), &dest_addr.sin6_addr);

    // ICMPv6 header
    // Create an ICMPv6 packet structure
    icmp6_hdr packet{};
    packet.icmp6_type = ICMP6_ECHO_REQUEST; // Set the ICMPv6 type to ECHO request
    packet.icmp6_code = 0;                  // Set the ICMPv6 code to 0 (default)
    packet.icmp6_id = htons(getpid());      // Set the ICMPv6 identifier to the process ID
    packet.icmp6_seq = htons(seq);          // Set the ICMPv6 sequence number to the provided sequence value
    packet.icmp6_cksum = 0;                 // Kernel fills it in
    // Send the ICMPv6 echo request packet to the specified host
    if (sendto(sock, &packet, sizeof(packet), 0, (sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
    {
        perror(("sendto " + host).c_str());
        return;
    }
}

void process_icmpv6_replies(int sock, vector<ICMPRequest> &requests)
{
    // Basically the same as process_icmp_replies, but for ICMPv6
    // Buffer to store the received packet data
    uint8_t buf[1024];
    // Structure to store the source address information
    sockaddr_in6 src_addr{};
    // Variable to store the length of the source address structure
    socklen_t addr_len = sizeof(src_addr);
    // Receive data from ICMPv6 socket
    ssize_t len = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&src_addr, &addr_len);
    if (len < static_cast<ssize_t>(sizeof(icmp6_hdr)))
        return;
    // Ensure packet is large enough
    icmp6_hdr *icmp6 = reinterpret_cast<icmp6_hdr *>(buf);
    // Check if the received packet is an ICMPv6 ECHO reply
    if (icmp6->icmp6_type != ICMP6_ECHO_REPLY)
        return;
    // Convert the source IP address to a human-readable format
    char ip_str[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &src_addr.sin6_addr, ip_str, sizeof(ip_str));
    // Extract the sequence number from the ICMPv6 packet
    uint16_t seq = ntohs(icmp6->icmp6_seq);
    // Find the corresponding ICMPv6 request in the list of requests
    auto it = find_if(requests.begin(), requests.end(),
                      [seq, &ip_str](const ICMPRequest &r)
                      {
                          return r.sequence_number == seq && r.host == ip_str;
                      });

    if (it != requests.end())
    {
        requests.erase(it);
    }
}
