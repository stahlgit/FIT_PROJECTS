#include "include/ndp.h"

#include <arpa/inet.h> // #include <netinet/in.h this is inside
#include <netinet/icmp6.h>

int create_ndp_socket()
{
    // create raw socket
    // AF_INET6: Address family for IPv6
    // SOCK_RAW: Socket type for raw sockets, allowing direct access to the IP layer
    // IPPROTO_ICMPV6: Protocol type for ICMPv6 (Internet Control Message Protocol for IPv6)
    int sock = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    // check if socket was created successfully
    if (sock < 0)
    {
        perror("NDP socket");
        return -1;
    }
    // return file descriptor of the NDP socket
    return sock;
}

void send_ndp_request(int sock, const string &target_ip, const string &iface_mac,
                      const string &iface_ip, const ProgramOptions &options)
{
    // NDP is ICMPv6 based, so it uses very similiar structure
    struct sockaddr_in6 dest_addr{};
    dest_addr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, target_ip.c_str(), &dest_addr.sin6_addr);

    uint8_t packet[sizeof(icmp6_hdr) + 24]; // 8 (ICMP header) + 16 (IPv6 address) + 8 (L2 option)
    icmp6_hdr *icmp6 = reinterpret_cast<icmp6_hdr *>(packet);
    memset(icmp6, 0, sizeof(icmp6_hdr));

    icmp6->icmp6_type = ND_NEIGHBOR_SOLICIT; // Neighbor Solicitation(request)
    icmp6->icmp6_code = 0;
    *reinterpret_cast<uint32_t *>(icmp6->icmp6_data8) = 0;

    // Target address
    inet_pton(AF_INET6, target_ip.c_str(), icmp6->icmp6_data8 + 4); // 4 bytes offset for reserved

    // Source link-layer option
    uint8_t *opt = packet + sizeof(icmp6_hdr);
    opt[0] = 1; // Type
    opt[1] = 1; // Length

    sscanf(iface_mac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &opt[2], &opt[3], &opt[4], &opt[5], &opt[6], &opt[7]);

    // Fill it by kernel
    icmp6->icmp6_cksum = 0;
    // send packet
    sendto(sock, packet, sizeof(packet), 0, (sockaddr *)&dest_addr, sizeof(dest_addr));
}

void process_ndp_replies(int sock, vector<NDPRequest> &requests)
{
    uint8_t buf[1024];
    sockaddr_in6 src_addr{};
    socklen_t addr_len = sizeof(src_addr);

    ssize_t len = recvfrom(sock, buf, sizeof(buf), 0, (sockaddr *)&src_addr, &addr_len);
    if (len < static_cast<ssize_t>(sizeof(icmp6_hdr)))
        return; // Ensure packet is large enough

    icmp6_hdr *icmp6 = reinterpret_cast<icmp6_hdr *>(buf);
    if (icmp6->icmp6_type != ND_NEIGHBOR_ADVERT)
        return;

    char ip_str[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &src_addr.sin6_addr, ip_str, sizeof(ip_str));

    // process target link-layer address option
    // Start parsing options after ICMPv6 header (8 bytes) + Reserved (4 bytes) + Target Address (16 bytes)
    uint8_t *opt = buf + sizeof(icmp6_hdr) + 16;
    uint8_t *end = buf + len;

    while (opt < end)
    {
        if (opt + 2 > end)
            break; // Ensure we can read type + length fields
        uint8_t type = opt[0];
        uint8_t length = opt[1] * 8; // Length is in units of 8 bytes

        if (length == 0 || opt + length > end)
            break; // Prevent infinite loop & out-of-bounds read

        if (type == 2)
        { // Target Link-Layer Address option
            if (length < 8)
                break; // Ensure the option is large enough

            char mac[18];
            snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
                     opt[2], opt[3], opt[4], opt[5], opt[6], opt[7]);

            for (auto &req : requests)
            {
                if (req.host == ip_str && !req.responded)
                {
                    req.mac = mac;
                    req.responded = true;
                }
            }
            break; // We found the MAC, no need to check further
        }

        // Move to the next option
        opt += length;
    }

    for (auto &req : requests)
    {
        if (req.host == ip_str && !req.responded)
        {
            char mac[18];
            snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
                     opt[2], opt[3], opt[4], opt[5], opt[6], opt[7]);
            req.mac = mac;
            req.responded = true;
        }
    }
}
