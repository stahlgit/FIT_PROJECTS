/**
 * @file test_dns_resolver.cpp
 * @brief Unit tests for rdt::infra::DnsResolver.
 */

#include "catch2_include/catch_amalgamated.hpp"
#include "dns_resolver.hpp"
#include "exceptions.hpp"

#include <netinet/in.h>

using rdt::infra::DnsResolver;
using rdt::infra::AddrEntry;

// ---------------------------------------------------------------------------
// HAPPY-PATH RESOLUTION
// ---------------------------------------------------------------------------

TEST_CASE("DnsResolver: resolve localhost returns at least one address", "[dns][resolve]")
{
    auto addrs = DnsResolver::resolve("localhost", 8080);
    REQUIRE_FALSE(addrs.empty());
    for (const auto &e : addrs)
    {
        REQUIRE((e.family == AF_INET || e.family == AF_INET6));
        REQUIRE(e.len > 0);
    }
}

TEST_CASE("DnsResolver: resolve numeric IPv4 returns AF_INET entry", "[dns][resolve]")
{
    auto addrs = DnsResolver::resolve("127.0.0.1", 9000);
    REQUIRE_FALSE(addrs.empty());
    REQUIRE(addrs[0].family == AF_INET);

    // Verify the encoded port matches what we asked for
    const auto *sin = reinterpret_cast<const sockaddr_in *>(&addrs[0].addr);
    REQUIRE(ntohs(sin->sin_port) == 9000);
}

TEST_CASE("DnsResolver: resolve numeric IPv6 ::1 does not throw", "[dns][resolve]")
{
    // AI_ADDRCONFIG may suppress IPv6 results on hosts without a configured
    // IPv6 interface, so we only assert no exception — not a non-empty list.
    REQUIRE_NOTHROW(DnsResolver::resolve("::1", 9000));
}

TEST_CASE("DnsResolver: resolve_first returns a value for 127.0.0.1", "[dns][resolve_first]")
{
    auto first = DnsResolver::resolve_first("127.0.0.1", 8080);
    REQUIRE(first.has_value());
    REQUIRE(first->family == AF_INET);
    REQUIRE(first->len == sizeof(sockaddr_in));
}

TEST_CASE("DnsResolver: resolve_first result has correct port", "[dns][resolve_first]")
{
    constexpr uint16_t PORT = 12345;
    auto first = DnsResolver::resolve_first("127.0.0.1", PORT);
    REQUIRE(first.has_value());

    const auto *sin = reinterpret_cast<const sockaddr_in *>(&first->addr);
    REQUIRE(ntohs(sin->sin_port) == PORT);
}

// ---------------------------------------------------------------------------
// ERROR CASES
// ---------------------------------------------------------------------------

TEST_CASE("DnsResolver: non-existent hostname throws DnsException", "[dns][error]")
{
    REQUIRE_THROWS_AS(
        DnsResolver::resolve("this.host.absolutely.does.not.exist.nxdomain.invalid", 8080),
        rdt::DnsException);
}

TEST_CASE("DnsResolver: resolve_first for non-existent hostname propagates DnsException", "[dns][error]")
{
    REQUIRE_THROWS_AS(
        DnsResolver::resolve_first("no.such.host.nxdomain.invalid", 1234),
        rdt::DnsException);
}
