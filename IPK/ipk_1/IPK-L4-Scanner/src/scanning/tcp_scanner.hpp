/**
 * @file tcp_scanner.hpp
 * @brief TCP port scanner implementation
 * @author Peter Stahl (xstahl01)
 */


#pragma once
#ifndef TCP_SCANNER_HPP
#define TCP_SCANNER_HPP

#include "scanner_base.hpp"

class TCPScanner : public ScannerBase
{
public:
    using ScannerBase::ScannerBase;

    /**
     * @brief Perform a TCP SYN scan on the specified target and port
     * @param task The scan task containing target information
     * @return ScanResult containing the scan outcome
     */
    ScanResult scan(const ScanTask& task) const override;

private:
    /**
     * @brief Build a TCP SYN packet for IPv4
     * @param task The scan task containing target information
     * @param src_port The source port to use in the packet
     * @return A vector of bytes representing the constructed packet
     */
    std::vector<uint8_t> build_packet_ipv4(const ScanTask& task, uint16_t src_port) const;

    /**
     * @brief Build a TCP SYN packet for IPv6
     * @param task The scan task containing target information
     * @param src_addr The source IPv6 address to use in the packet
     * @return A vector of bytes representing the constructed packet
     */
    std::vector<uint8_t> build_packet_ipv6(const ScanTask& task, const in6_addr& src_addr) const;

    /**
     * @brief Generate a random Initial Sequence Number (ISN) for TCP packets
     * @return A random 32-bit unsigned integer to be used as the ISN
     */
    static uint32_t random_isn();
};

#endif // TCP_SCANNER_HPP