#include "include/icmp.h"
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <algorithm>

int create_icmp_socket()
{
    // create raw socket
    // AF_INET: Address family for IPv4
    // SOCK_RAW: Socket type for raw sockets, allowing direct access to the IP layer
    // IPPROTO_ICMP: Protocol type for ICMP (Internet Control Message Protocol)
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    // check if socket was created successfully
    if (sock < 0)
    {
        perror("ICMP socket");
        return -1;
    }
    // return file descriptor of the ICMP socket
    return sock;
}

void send_icmp_request(int sock, const string &host, uint16_t sequence)
{
    // Structure to store the destination address information
    sockaddr_in dest_addr{};
    // Set the address family to AF_INET (IPv4)
    dest_addr.sin_family = AF_INET;
    // Convert the target IP address string to a binary IPv4 address and populate the destination address structure
    inet_pton(AF_INET, host.c_str(), &dest_addr.sin_addr);

    // ICMP header
    // Create an ICMP packet structure
    icmphdr packet{};
    packet.type = ICMP_ECHO;                   // Set the ICMP type to ECHO request
    packet.code = 0;                           // Set the ICMP code to 0 (default)
    packet.un.echo.id = htons(getpid());       // Set the ICMP identifier to the process ID
    packet.un.echo.sequence = htons(sequence); // Set the ICMP sequence number to the provided sequence value
    packet.checksum = 0;
    packet.checksum = compute_checksum_ipv4(&packet, sizeof(packet)); // showcase of checksum calculation (nowadays it is done by the kernel)

    // Send the ICMP echo request packet to the specified host
    if (sendto(sock, &packet, sizeof(packet), 0,
               (sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
    {
        perror(("sendto " + host).c_str());
        return;
    }
}

void process_icmp_replies(int sock, vector<ICMPRequest> &requests)
{
    // Buffer to store the received packet data
    uint8_t buf[1024];
    // Structure to store the source address information
    sockaddr_in recv_addr{};
    // Variable to store the length of the source address structure
    socklen_t addr_len = sizeof(recv_addr);

    // Receive data from ICMP socket
    ssize_t len = recvfrom(sock, buf, sizeof(buf), 0, (sockaddr *)&recv_addr, &addr_len);
    // If no data is received or error occurs, return
    if (len <= 0)
        return;

    // Casting the received buffer to an IP header structure to access the IP header fields
    ip *ip_hdr = (ip *)buf;
    // Calculating the start of the ICMP header within the received buffer, by skipping the IP header
    // ip_hl is the IP header length in 32-bit words, so we multiply by 4 to get bytes.
    icmphdr *icmp_hdr = (icmphdr *)(buf + (ip_hdr->ip_hl << 2));

    // Check if the received packet is an ICMP ECHO reply
    if (icmp_hdr->type != ICMP_ECHOREPLY)
        return;

    // Convert the source IP address to a human-readable format
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &recv_addr.sin_addr, ip_str, sizeof(ip_str));
    // Get the sequence number from the received ICMP reply (in host byte order)
    uint16_t recv_seq = ntohs(icmp_hdr->un.echo.sequence);
    // Find the corresponding ICMP request in the list of requests
    auto it = find_if(requests.begin(), requests.end(),
                      [recv_seq](const ICMPRequest &req)
                      { return req.sequence_number == recv_seq; });
    // If a matching request is found (it is not the end iterator)
    if (it != requests.end())
    {
        // Calculating the round-trip time (RTT) by subtracting the sent time from the current time
        auto rtt = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - it->sent_time);
        // Remove the processed request from the vector
        requests.erase(it);
    }
}
