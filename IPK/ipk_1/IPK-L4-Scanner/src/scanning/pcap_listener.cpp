/**
 * @file pcap_listener.cpp
 * @brief Implementation of the PcapListener class for passive packet capture and analysis.
 * @author Peter Stahl (xstahl01)
 */
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <arpa/inet.h>
#include <pcap/dlt.h>
#include <array>
#include <iostream>

#include "pcap_listener.hpp"
#include "task_tracker.hpp"
#include "exceptions.hpp"


PcapListener::~PcapListener() {
    stop();
    if (handle_) {
        pcap_close(handle_);
    }
}

void PcapListener::start() {
    std::array<char, PCAP_ERRBUF_SIZE> errbuf{};
    
    handle_ = pcap_create(iface_.c_str(), errbuf.data());
    if (!handle_) {
        throw PcapError("PcapListener failed to create handle: " + std::string(errbuf.data()));
    }

    pcap_set_snaplen(handle_, SNAPLEN);
    pcap_set_promisc(handle_, 0);
    pcap_set_timeout(handle_, pcap_timeout_ms_); 
    pcap_set_immediate_mode(handle_, 1);

    if (pcap_activate(handle_) != 0) {
        throw PcapError("PcapListener failed to activate: " + std::string(pcap_geterr(handle_)));
    }

    ll_offset_ = determine_link_layer_offset_();

    // Filter: Catch TCP (SYN-ACK or RST) or any ICMP/ICMPv6 (for UDP closed states)
    std::string filter = "(tcp[tcpflags] & (tcp-syn|tcp-rst) != 0) or icmp or icmp6";
    if (bpf_program fp; pcap_compile(handle_, &fp, filter.c_str(), 1, PCAP_NETMASK_UNKNOWN) == 0) {
        pcap_setfilter(handle_, &fp);
        pcap_freecode(&fp);
    } else {
        std::cerr << "Warning: PcapListener failed to compile BPF filter: " << pcap_geterr(handle_) << "\n";
    }

    // Launch the background thread
    active_ = true;
    listener_thread_ = std::thread(&PcapListener::listen_loop_, this);
}

void PcapListener::stop() {
    active_ = false;
    if (listener_thread_.joinable()) {
        listener_thread_.join();
    }
}

void PcapListener::listen_loop_() {
    pcap_setnonblock(handle_, 1, nullptr);

    while (running_ && active_) {
        pcap_pkthdr* header = nullptr;
        const uint8_t* pkt = nullptr;
        
        int res = pcap_next_ex(handle_, &header, &pkt);
        if (res == 1) { // Packet captured
            if (header->caplen > static_cast<uint32_t>(ll_offset_)) {
                process_packet_(pkt + ll_offset_, header->caplen - ll_offset_);
            }
        } else if (res == 0) { // Timeout
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); 
        } else if (res < 0) { // Error
            break;
        }
    }
}

void PcapListener::process_packet_(const uint8_t* payload, uint32_t len) const {
    if (len < 1) return;
    
    // Determine IP version from the first nibble
    uint8_t version = payload[0] >> 4;
    if (version == 4) {
        parse_ipv4_(payload, len);
    } else if (version == 6) {
        parse_ipv6_(payload, len);
    }
}

void PcapListener::parse_ipv4_(const uint8_t* payload, uint32_t len) const {
    if (len < sizeof(iphdr)) return;
    const auto* ip = reinterpret_cast<const iphdr*>(payload);
    uint32_t ip_hdr_len = ip->ihl * 4;
    if (len < ip_hdr_len) return;

    char src_ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ip->saddr), src_ip_str, INET_ADDRSTRLEN);

    const uint8_t* transport = payload + ip_hdr_len;
    uint32_t transport_len = len - ip_hdr_len;

    if (ip->protocol == IPPROTO_TCP && transport_len >= sizeof(tcphdr)) {
        using enum PortState;
        const auto* tcp = reinterpret_cast<const tcphdr*>(transport);
        uint16_t src_port = ntohs(tcp->source);
        
        PortState state = FILTERED;
        if (tcp->syn && tcp->ack) state = OPEN;
        else if (tcp->rst) state = CLOSED;
        
        if (state != FILTERED) {
            TaskTracker::instance().resolve_task({src_ip_str, src_port, Protocol::TCP}, state);
        }
    } 
    else if (ip->protocol == IPPROTO_ICMP && transport_len >= sizeof(icmphdr) + sizeof(iphdr) + 8) {
        const auto* icmp = reinterpret_cast<const icmphdr*>(transport);
        if (icmp->type == ICMP_DEST_UNREACH && icmp->code == ICMP_PORT_UNREACH) {
            const uint8_t* inner_start = transport + sizeof(icmphdr);
            const auto* inner_ip = reinterpret_cast<const iphdr*>(inner_start);
            uint32_t inner_ip_hdr_len = inner_ip->ihl * 4;
            
            // Safety check: enough bytes for inner IP header + at least 8 bytes of UDP
            if (transport_len < sizeof(icmphdr) + inner_ip_hdr_len + 8) return;
            
            const auto* inner_udp = reinterpret_cast<const udphdr*>(inner_start + inner_ip_hdr_len);
            uint16_t dest_port = ntohs(inner_udp->dest);

            // Use the INNER destination IP — that's the original scan target
            char target_ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(inner_ip->daddr), target_ip_str, INET_ADDRSTRLEN);

            TaskTracker::instance().resolve_task({target_ip_str, dest_port, Protocol::UDP}, PortState::CLOSED);
        }
    }
}

void PcapListener::parse_ipv6_(const uint8_t* payload, uint32_t len) const {
    if (len < sizeof(ip6_hdr)) return;
    const auto* ip6 = reinterpret_cast<const ip6_hdr*>(payload);

    char src_ip_str[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &(ip6->ip6_src), src_ip_str, INET6_ADDRSTRLEN);

    const uint8_t* transport = payload + sizeof(ip6_hdr);
    uint32_t transport_len = len - sizeof(ip6_hdr);
    uint8_t next_hdr = ip6->ip6_nxt;

    // Extension header types that have a standard (next_hdr, length) layout
    while (transport_len >= 2){
        // found the transport layer 
        // break while loop, not switch ! 
        if (next_hdr == IPPROTO_TCP || next_hdr == IPPROTO_UDP || next_hdr == IPPROTO_ICMPV6) break; 

        switch(next_hdr){
            // known skippable extension headers
            case IPPROTO_HOPOPTS:
            case IPPROTO_ROUTING:
            case IPPROTO_DSTOPTS:
            case IPPROTO_MH: {
                uint32_t ext_len = (transport[1] + 1) * 8; 
                if(transport_len < ext_len) return; // malformed packet
                next_hdr = transport[0]; // next header type is the first byte of the extension
                transport += ext_len; // move to the next header
                transport_len -= ext_len;
                break; // break switch continue with loop
            }
            case IPPROTO_FRAGMENT: {
                // Fragment header is always 8 bytes; fragmented packets can't be reliably parsed
                return;
            }
            default:
                // Unrecognized header type, can't continue parsing
                return;
        }
    }

    if (next_hdr == IPPROTO_TCP && transport_len >= sizeof(tcphdr)) {
        using enum PortState;
        const auto* tcp = reinterpret_cast<const tcphdr*>(transport);
        uint16_t src_port = ntohs(tcp->source);
        
        PortState state = FILTERED;
        if (tcp->syn && tcp->ack) state = OPEN;
        else if (tcp->rst) state = CLOSED;
        
        if (state != FILTERED) {
            TaskTracker::instance().resolve_task({src_ip_str, src_port, Protocol::TCP}, state);
        }
    }
    else if (next_hdr == IPPROTO_ICMPV6 && transport_len >= sizeof(icmp6_hdr) + sizeof(ip6_hdr) + 8) {
        const auto* icmp6 = reinterpret_cast<const icmp6_hdr*>(transport);
        if (icmp6->icmp6_type == ICMP6_DST_UNREACH && icmp6->icmp6_code == ICMP6_DST_UNREACH_NOPORT) {
            const auto* inner_ip6 = reinterpret_cast<const ip6_hdr*>(transport + sizeof(icmp6_hdr));
            const auto* inner_udp = reinterpret_cast<const udphdr*>(
                reinterpret_cast<const uint8_t*>(inner_ip6) + sizeof(ip6_hdr));
            uint16_t dest_port = ntohs(inner_udp->dest);

            // Use the INNER destination IP — the original scan target
            char target_ip_str[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &(inner_ip6->ip6_dst), target_ip_str, INET6_ADDRSTRLEN);

            TaskTracker::instance().resolve_task({target_ip_str, dest_port, Protocol::UDP}, PortState::CLOSED);
        }
    }
}

int PcapListener::determine_link_layer_offset_() const {
    int dl = pcap_datalink(handle_);
    switch (dl) {
        case DLT_NULL:
        case DLT_LOOP: 
            return 4; // BSD loopback / Loccal loopback
        case DLT_LINUX_SLL: 
            return 16; // linix cooked capture
        case DLT_EN10MB: 
            return 14; // Ethernet
        case DLT_RAW: 
            return 0;  // Raw IP
        default:
            std::cerr << "Warning: Unrecognized datalink type " << dl << ", defaulting to Ethernet offset\n";
            return 14;
    }
}