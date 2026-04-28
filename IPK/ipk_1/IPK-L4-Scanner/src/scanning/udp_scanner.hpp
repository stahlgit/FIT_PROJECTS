/**
 * @file udp_scanner.hpp
 * @brief UDP port scanner implementation
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef UDP_SCANNER_HPP
#define UDP_SCANNER_HPP

#include "scanner_base.hpp"

class UDPScanner : public ScannerBase
{
public:
    using ScannerBase::ScannerBase;

    /**
     * @brief Perform a UDP scan on the specified target and port
     * @param task The scan task containing target information 
     * @return ScanResult containing the scan outcome
     */
    ScanResult scan(const ScanTask& task) const override;

private:
    /**
     * @brief Build a UDP packet for IPv4
     * @param task The scan task containing target information
     * @return A vector of bytes representing the constructed packet
     */
    std::vector<uint8_t> build_packet_ipv4(const ScanTask& task) const;

    /**
     * @brief Build a UDP packet for IPv6
     * @param task The scan task containing target information
     * @param src_addr The source IPv6 address to use in the packet
     * @return A vector of bytes representing the constructed packet
     */
    std::vector<uint8_t> build_packet_ipv6(const ScanTask& task, const in6_addr& src_addr) const;
};

#endif // UDP_SCANNER_HPP