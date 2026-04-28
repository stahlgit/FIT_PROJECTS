/**
 * @file scanner_base.hpp
 * @brief Declaration of the ScannerBase class, providing common utilities for active port scanning.
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef SCANNER_BASE_HPP
#define SCANNER_BASE_HPP

#include <string>
#include <pcap.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <atomic>
#include <span>
#include <optional>
#include <unordered_map>

#include "scan_types.hpp"

/**
 * @brief Abstract base class for active port scanners (TCP/UDP) 
 * Provides common utilities for packet crafting, checksums, and interface handling
 */
class ScannerBase
{
public:
    /**
     * @brief Constructor with common parameters 
     * @param iface Network interface name for raw socket and pcap listener
     * @param timeout_ms Per-port scan timeout in milliseconds
     * @param running Reference to Application's running flag for clean shutdown
     */
    ScannerBase(const std::string &iface, int timeout_ms, const std::atomic<bool> &running)
        : iface_(iface), timeout_ms_(timeout_ms), running_(running) {}

    virtual ~ScannerBase() = default;

    // Non-copyable, non-movable -scanners are short-lived task objects
    ScannerBase(const ScannerBase &) = delete;
    ScannerBase &operator=(const ScannerBase &) = delete;

    // Main scanning method to be implemented by derived classes
    virtual ScanResult scan(const ScanTask &task) const = 0;

protected:
    // shared utilities
    /**
     * @brief Calculate the checksum sum for given data buffer. Used as part of TCP/UDP checksum calculation
     */
    static uint32_t calculate_sum(std::span<const std::byte> data);

    /**
     * @brief Finalize the checksum by folding the 32-bit sum into 16 bits and taking the one's complement
     * Used after calculate_sum to get the final checksum value
     */
    static uint16_t finalize_checksum(uint32_t sum);
    /**
     * @brief backward compatibility method for single-buffer cheksums
     */
    static uint16_t checksum(std::span<const std::byte> data);

    /**
     * @brief Picks src port for outgoing scan packets
     * @details Uses an atomic counter to ensure thread safety and avoid collisions between workers
     */
    static uint16_t pick_src_port();
    
    /**
     * @brief Builds the base IPv4 header for a scan packet, given the task details and transport layer size
     * @param task Scan task details
     * @param protocol Transport layer protocol (TCP/UDP)
     * @param transport_size Size of the transport layer data
     * @return Vector containing the built IPv4 header
     */
    std::vector<uint8_t> build_ipv4_base(const ScanTask &task, uint8_t protocol, size_t transport_size) const;

    /**
     * @brief Calculate the transport layer checksum (TCP/UDP) for IPv4, including the pseudo-header
     * @param ip Pointer to the IPv4 header of the packet
     * @param transport_data Transport layer data
     * @param protocol Transport layer protocol (TCP/UDP)
     * @return Finalized checksum value
     */
    static uint16_t transport_checksum_ipv4(const iphdr *ip, std::span<const std::byte> transport_data, uint8_t protocol);
    /**
     * @brief Calculate the transport layer checksum (TCP/UDP) for IPv6, including the pseudo-header
     * @param src Source IPv6 address
     * @param dst Destination IPv6 address
     * @param transport_data Transport layer data
     * @param protocol Transport layer protocol (TCP/UDP)
     * @return Finalized checksum value
     */
    static uint16_t transport_checksum_ipv6(const in6_addr &src, const in6_addr &dst, std::span<const std::byte> transport_data, uint8_t protocol);

    /**
     * @brief Get the IPv4 address of the specified network interface. Caches results to avoid repeated system calls
     * @param iface Interface name
     */
    static uint32_t get_interface_ip_v4(const std::string &iface);
    /**
     * @brief Get the IPv6 address of the specified network interface. Caches results to avoid repeated system calls
     * @param iface Interface name
     * @param addr Output parameter for the binary IPv6 address
     * @param ip_str Output parameter for the string representation of the IPv6 address
     */
    static void get_interface_ip_v6(const std::string &iface, struct in6_addr &addr, std::string &ip_str);

    /**
     * @brief Bind a raw socket to the specified network interface or address. Used by scanners to ensure packets 
     *  are sent from the correct interface
     */
    void bind_to_interface(int sock_fd, const std::string& iface) const;
    /**
     * @brief Bind a raw socket to the specified local address. Used by scanners to ensure packets are sent from the 
     *  correct IP
     */
    void bind_to_address(int sock_fd, const sockaddr* addre, socklen_t addr_len) const;

    /**
     * @brief Accessors for common parameters
     */
    const std::string &get_iface() const { return iface_; }
    int get_timeout_ms() const { return timeout_ms_; }
    bool is_running() const { return running_; }

    // IANA recommended ephemeral port range - but kept as protected - if other child of this wants to use different range
    static constexpr uint16_t EPHEMERAL_PORT_BEGIN = 49152;
    static constexpr uint16_t EPHEMERAL_PORT_END = 65535;

    static constexpr uint8_t IPV4_VERSION = 4;
    static constexpr uint8_t IPV4_MIN_IHL = 5; // 5 words (20 bytes)
    static constexpr uint8_t DEFAULT_TTL = 64;

private:
    std::string iface_;
    int timeout_ms_;
    const std::atomic<bool> &running_;

    // Interface IP caching - to avoid repeated system calls for each scan task
    struct IfaceCache{
        uint32_t ipv4 = 0; 
        in6_addr ipv6{};
        std::string ipv6_str;
        bool has_ipv4 = false;
        bool has_ipv6 = false;
    };
    static std::mutex iface_cache_mutex_;
    static std::unordered_map<std::string, IfaceCache> iface_cache_;

};

#endif // SCANNER_BASE_HPP