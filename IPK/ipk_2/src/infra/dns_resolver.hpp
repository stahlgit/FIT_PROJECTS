/**
 * @file dns_resolver.hpp
 * @brief DNS/address resolution using getaddrinfo() with IPv4/IPv6 support.
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef DNS_RESOLVER_HPP
#define DNS_RESOLVER_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <sys/socket.h>

namespace rdt::infra
{

    /// @brief Single resolved address ready to be passed to socket/connect
    struct AddrEntry
    {
        sockaddr_storage addr{};
        socklen_t len = 0;
        int family = 0;
    };

    /**
     * @brief Thin wrapper around getaddrinfo() that returns strongly-typed results
     * Resolves hostname (or numeric IP string) and port to one or more candidate addresses for use with UdpSocket::connect_client()
     *
     * Resolution is AF_UNSPEC so both IPv4 and IPv6 results are returned in the order the system prefers
     */
    class DnsResolver
    {
    public:
        /**
         * @brief Resolve host+port and return all candidate addresses
         * @return All addresses the system returned; may be empty
         * @throws DnsException if getaddrinfo() returns a hard error
         */
        [[nodiscard]] static std::vector<AddrEntry> resolve(const std::string &host, uint16_t port);

        /**
         * @brief Convenience wrapper - returns the first (preferred) address
         * @return First address, or std::nullopt if resolution found nothing
         */
        [[nodiscard]] static std::optional<AddrEntry> resolve_first(const std::string &host, uint16_t port);
    };

} // namespace rdt::infra

#endif // DNS_RESOLVER_HPP
