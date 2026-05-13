/**
 * @file dns_resolver.cpp
 * @brief Implementation of DnsResolver.
 * @author Peter Stahl (xstahl01)
 */

#include "dns_resolver.hpp"
#include "exceptions.hpp"

#include <cstring>
#include <string>

#include <netdb.h>

namespace rdt::infra
{

    std::vector<AddrEntry> DnsResolver::resolve(const std::string &host, uint16_t port)
    {
        addrinfo hints{};
        hints.ai_family = AF_UNSPEC; // accept both IPv4 and IPv6
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_ADDRCONFIG; // skip families with no usable interface

        const std::string port_str = std::to_string(port);

        addrinfo *res = nullptr;
        if (const int rc = ::getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res); rc != 0) {
            throw DnsException("DnsResolver: " + host + ": " + ::gai_strerror(rc));
        }

        std::vector<AddrEntry> out;
        for (const addrinfo *p = res; p != nullptr; p = p->ai_next)
        {
            AddrEntry e;
            std::memcpy(&e.addr, p->ai_addr, p->ai_addrlen);

            e.len = p->ai_addrlen;
            e.family = p->ai_family;
            
            out.push_back(e);
        }

        ::freeaddrinfo(res);
        return out;
    }

    std::optional<AddrEntry> DnsResolver::resolve_first(const std::string &host, uint16_t port)
    {
        auto addrs = resolve(host, port);
        if (addrs.empty())
            return std::nullopt;
        return addrs.front();
    }

} // namespace rdt::infra
