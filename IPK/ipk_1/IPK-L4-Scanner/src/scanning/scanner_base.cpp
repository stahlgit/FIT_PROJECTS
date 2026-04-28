/**
 * @file scanner_base.cpp
 * @brief Implementation of the ScannerBase class, providing common utilities for active port scanning.
 * @author Peter Stahl (xstahl01)
 */

#include <cstddef>
#include <cstdint>
#include <pcap/dlt.h>
#include <pcap/pcap.h>
#include <stdexcept>
#include <string>

#include <poll.h>
#include <chrono>
#include <cerrno>

#include <ifaddrs.h>
#include <mutex>

#include "scanner_base.hpp"
#include "exceptions.hpp"
#include <cstring>

std::mutex ScannerBase::iface_cache_mutex_;
std::unordered_map<std::string, ScannerBase::IfaceCache> ScannerBase::iface_cache_;

// utils

uint32_t ScannerBase::calculate_sum(std::span<const std::byte> data)
{
    uint32_t sum = 0;
    size_t len = data.size();
    size_t i = 0;

    // Read bytes safely without breaking strict aliasing rules
    while (len > 1)
    {
        uint16_t word;
        std::memcpy(&word, &data[i], sizeof(word));
        sum += word;
        i += 2;
        len -= 2;
    }

    if (len == 1)
    {
        // for odd byte, pad with zero to form the last 16-bit word
        uint16_t word = static_cast<uint16_t>(std::to_integer<uint8_t>(data[i])) << 8;
        sum += word;
    }

    return sum; // Return the raw, unfolded sum
}

uint16_t ScannerBase::finalize_checksum(uint32_t sum)
{
    // Fold 32-bit sum to 16 bits
    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return static_cast<uint16_t>(~sum); // Apply 1's complement
}

uint16_t ScannerBase::checksum(std::span<const std::byte> data)
{
    // backward compatibility for single-buffer checksums
    return finalize_checksum(calculate_sum(data));
}

uint16_t ScannerBase::pick_src_port()
{
    // Ephemeral port range: 49152–65535 (IANA recommendation)
    static constexpr uint16_t RANGE = EPHEMERAL_PORT_END - EPHEMERAL_PORT_BEGIN + 1;
    static std::atomic<uint_fast32_t> port_counter{0}; 

    uint32_t current = port_counter.fetch_add(1, std::memory_order_relaxed);
    return EPHEMERAL_PORT_BEGIN + (current % RANGE);
}

std::vector<uint8_t> ScannerBase::build_ipv4_base(const ScanTask &task, uint8_t protocol, size_t transport_size) const
{
    size_t total = sizeof(iphdr) + transport_size;
    std::vector<uint8_t> buf(total, 0);

    auto *ip = reinterpret_cast<iphdr *>(buf.data());
    const auto *dst = reinterpret_cast<const sockaddr_in *>(&task.target.addr);

    ip->ihl = IPV4_MIN_IHL;
    ip->version = IPV4_VERSION;
    ip->tot_len = htons(static_cast<uint16_t>(total));
    ip->ttl = DEFAULT_TTL;
    ip->protocol = protocol;
    ip->saddr = ScannerBase::get_interface_ip_v4(ScannerBase::get_iface());
    ip->daddr = dst->sin_addr.s_addr;

    return buf;
}

uint16_t ScannerBase::transport_checksum_ipv4(const iphdr *ip, std::span<const std::byte> transport_data, uint8_t protocol)
{
    // Pseudo-header for checksum calculation
    struct
    {
        uint32_t src;
        uint32_t dst;
        uint8_t zero;
        uint8_t protocol;
        uint16_t length;
    } phdr;

    phdr.src = ip->saddr;
    phdr.dst = ip->daddr;
    phdr.zero = 0;
    phdr.protocol = protocol;
    phdr.length = htons(static_cast<uint16_t>(transport_data.size()));

    // Calculate sums independently
    uint32_t pseudo_sum = calculate_sum(std::span<const std::byte>(reinterpret_cast<const std::byte *>(&phdr), sizeof(phdr)));
    uint32_t transport_sum = calculate_sum(transport_data);

    // Combine and finalize
    return finalize_checksum(pseudo_sum + transport_sum);
}

uint16_t ScannerBase::transport_checksum_ipv6(const in6_addr &src, const in6_addr &dst, std::span<const std::byte> transport_data, uint8_t protocol)
{
    struct
    {
        uint8_t src[16];
        uint8_t dst[16];
        uint32_t len;
        uint8_t zero[3];
        uint8_t next_hdr;
    } pseudo{};

    std::memcpy(pseudo.src, &src, 16);
    std::memcpy(pseudo.dst, &dst, 16);
    pseudo.len = htonl(static_cast<uint32_t>(transport_data.size()));
    pseudo.next_hdr = protocol;

    uint32_t pseudo_sum = calculate_sum(std::span<const std::byte>(reinterpret_cast<const std::byte *>(&pseudo), sizeof(pseudo)));
    uint32_t transport_sum = calculate_sum(transport_data);
    return finalize_checksum(pseudo_sum + transport_sum);
}

uint32_t ScannerBase::get_interface_ip_v4(const std::string& iface)
{
    // Lock to check the cache
    {
        std::lock_guard lock(iface_cache_mutex_);
        auto it = iface_cache_.find(iface);
        if (it != iface_cache_.end() && it->second.has_ipv4) {
            return it->second.ipv4;
        }
    } // unlocks here

    // expensive OS call lock-free
    ifaddrs* ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1) {
        throw SystemError("getifaddrs failed");
    }

    // RAII guard for v4!
    std::unique_ptr<struct ifaddrs, void(*)(struct ifaddrs*)> ifaddr_guard(ifaddr, freeifaddrs);

    uint32_t found_ip = 0;
    bool found = false;

    for (ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || !ifa->ifa_name) continue;
        if (ifa->ifa_addr->sa_family == AF_INET && iface == ifa->ifa_name) {
            found_ip = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr)->sin_addr.s_addr;
            found = true;
            break;
        }
    }

    if (!found) {
        throw InterfaceError("No IPv4 address for interface: " + iface);
    }

    // Lock again to update the cache
    {
        std::lock_guard lock(iface_cache_mutex_);
        auto& entry = iface_cache_[iface];
        entry.ipv4 = found_ip;
        entry.has_ipv4 = true;
        return entry.ipv4;
    }
}

void ScannerBase::get_interface_ip_v6(const std::string &iface, struct in6_addr &addr, std::string &ip_str)
{
    // lock + check cache
    {
        std::lock_guard lock(iface_cache_mutex_);
        auto it = iface_cache_.find(iface);
        if (it != iface_cache_.end() && it->second.has_ipv6) {
            addr   = it->second.ipv6;
            ip_str = it->second.ipv6_str;
            return;
        }
    } // unlock

    ifaddrs* ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1)
        throw SystemError("getifaddrs failed");

    // RAII for C-style memory: ensures freeifaddrs is called automatically when this goes out of scope!
    std::unique_ptr<struct ifaddrs, void(*)(struct ifaddrs*)> ifaddr_guard(ifaddr, freeifaddrs);

    in6_addr found_addr{};
    std::string found_ip_str;
    bool found = false;

    for (ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || !ifa->ifa_name) continue;
        if (ifa->ifa_addr->sa_family == AF_INET6 && iface == ifa->ifa_name) {
            auto const* sin6 = reinterpret_cast<sockaddr_in6*>(ifa->ifa_addr);
            std::memcpy(&found_addr, &sin6->sin6_addr, sizeof(in6_addr));
            
            if (char buf[INET6_ADDRSTRLEN]; inet_ntop(AF_INET6, &found_addr, buf, sizeof(buf))) {
                found_ip_str = buf;
            }
            found = true;
            break;
        }
    }
    // freeifaddrs(ifaddr);  - handled by unique_ptr guard 

    if (!found)
        throw InterfaceError("No IPv6 address for interface: " + iface);

    // lock + update cache
    {
        std::lock_guard lock(iface_cache_mutex_);
        auto& entry = iface_cache_[iface];
        
        entry.ipv6 = found_addr;
        entry.ipv6_str = found_ip_str;
        entry.has_ipv6 = true;
        
        addr   = entry.ipv6;
        ip_str = entry.ipv6_str;
    }
}

void ScannerBase::bind_to_interface(int sock_fd, const std::string& iface) const {
    if (setsockopt(sock_fd, SOL_SOCKET, SO_BINDTODEVICE, iface.c_str(), iface.size() + 1) < 0) {
        throw SystemError("Failed to bind socket to interface " + iface);
    }
}

void ScannerBase::bind_to_address(int sock_fd, const sockaddr* addr, socklen_t addr_len) const {
    if (::bind(sock_fd, addr, addr_len) < 0) {
        throw SystemError("Failed to bind socket to address");
    }
}