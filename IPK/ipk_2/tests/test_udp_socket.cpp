/**
 * @file test_udp_socket.cpp
 * @brief Unit and integration tests for rdt::infra::UdpSocket.
 *
 * Loopback tests bind to port 0 (OS assigns) and query the actual port
 * via getsockname() before connecting the peer — no hard-coded ports.
 */

#include "catch2_include/catch_amalgamated.hpp"
#include "udp_socket.hpp"
#include "exceptions.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>

using rdt::infra::UdpSocket;

// ---------------------------------------------------------------------------
// CONSTRUCTION
// ---------------------------------------------------------------------------

TEST_CASE("UdpSocket: bind_server on explicit IPv4 loopback yields valid fd", "[udp][bind]")
{
    auto sock = UdpSocket::bind_server("127.0.0.1", 0);
    REQUIRE(sock.fd() >= 0);
    REQUIRE(sock.family() == AF_INET);
}

TEST_CASE("UdpSocket: bind_server with empty address (wildcard) yields valid fd", "[udp][bind]")
{
    auto sock = UdpSocket::bind_server("", 0);
    REQUIRE(sock.fd() >= 0);
    REQUIRE((sock.family() == AF_INET || sock.family() == AF_INET6));
}

TEST_CASE("UdpSocket: bind_server on invalid address throws NetworkException", "[udp][bind][error]")
{
    REQUIRE_THROWS_AS(UdpSocket::bind_server("999.999.999.999", 9999), rdt::NetworkException);
}

// ---------------------------------------------------------------------------
// MOVE SEMANTICS
// ---------------------------------------------------------------------------

TEST_CASE("UdpSocket: move constructor transfers fd and invalidates source", "[udp][move]")
{
    auto sock = UdpSocket::bind_server("127.0.0.1", 0);
    const int fd_before = sock.fd();
    REQUIRE(fd_before >= 0);

    auto moved = std::move(sock);

    REQUIRE(moved.fd() == fd_before);
    REQUIRE(sock.fd() == -1); // NOLINT(bugprone-use-after-move) — intentional check
}

TEST_CASE("UdpSocket: move assignment transfers fd and invalidates source", "[udp][move]")
{
    auto a = UdpSocket::bind_server("127.0.0.1", 0);
    auto b = UdpSocket::bind_server("127.0.0.1", 0);
    const int fd_a = a.fd();

    b = std::move(a);

    REQUIRE(b.fd() == fd_a);
    REQUIRE(a.fd() == -1); // NOLINT(bugprone-use-after-move)
}