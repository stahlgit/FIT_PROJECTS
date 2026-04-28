/**
 * @file pcap_listener.hpp
 * @brief Header for the PcapListener class, which captures and processes network packets using libpcap.
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef PCAP_LISTENER_HPP
#define PCAP_LISTENER_HPP

#include <string>
#include <atomic>
#include <thread>
#include <pcap.h>

constexpr int SNAPLEN = 65535;

class PcapListener
{
public:
    /**
     * @brief Contruct the global pcap listener
     * @param iface The network interface to listen on (e.g., "eth0")
     * @param running Reference to the atomic flag that controls the listener's lifecycle
     * @param pcap_timeout_ms Timeout for pcap packet capture in milliseconds
     */
    PcapListener(const std::string &iface, std::atomic<bool> &running, int pcap_timeout_ms) : iface_(iface), running_(running), pcap_timeout_ms_(pcap_timeout_ms) {}
    /**
     * @brief Destructor to clean up resources and stop the listener thread
     */
    ~PcapListener();

    // Delete copy constructor and assignment operator to prevent copying
    PcapListener(const PcapListener &) = delete;
    PcapListener &operator=(const PcapListener &) = delete;

    // Start the listener thread
    void start();

    // Stop the listener thread
    void stop();

private:
    /** @brief The main listening loop for capturing packets */
    void listen_loop_();
    /**
     * @brief Process a captured packet, determine its protocol, and extract relevant information for task resolution
     * @param packet Pointer to the raw packet data (starting after the link layer header)
     * @param caplen The length of the captured packet data
     */
    void process_packet_(const uint8_t *packet, uint32_t caplen) const;

    // Parsers for different protocols
    /**
     * @brief Parse an IPv4 packet, extract transport layer information, and resolve tasks based on TCP/ICMP responses
     * @param payload Pointer to the start of the IPv4 header
     * @param len The length of the IPv4 packet data
     */
    void parse_ipv4_(const uint8_t *payload, uint32_t len) const;
    /**
     * @brief Parse an IPv6 packet, handle extension headers, extract transport layer information, and resolve tasks
     *  based on TCP/ICMPv6 responses
     * @param payload Pointer to the start of the IPv6 header
     * @param len The length of the IPv6 packet data
     */
    void parse_ipv6_(const uint8_t *payload, uint32_t len) const;

    /**
     * @brief Determine the offset of the link layer header based on the data link type of the pcap handle
     */
    int determine_link_layer_offset_() const;

    std::string iface_;
    std::atomic<bool> &running_;
    std::thread listener_thread_;
    pcap_t *handle_ = nullptr;
    int ll_offset_ = 0;
    std::atomic<bool> active_{false};
    int pcap_timeout_ms_;
};

#endif // PCAP_LISTENER_HPP