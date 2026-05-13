/**
 * @file udp_socket.hpp
 * @brief RAII wrapper for a UDP socket with IPv4/IPv6 dual-stack support.
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef UDP_SOCKET_HPP
#define UDP_SOCKET_HPP

#include <cstdint>
#include <cstddef>
#include <string>
#include <sys/socket.h>

namespace rdt::infra
{

    /**
     * @brief RAII-managed UDP socket. Owns the underlying file descriptor
     *
     * Two factory paths:
     *   - bind_server()    - creates and binds a server-side socket (dual-stack if possible)
     *   - connect_client() - creates a UDP socket connected to a resolved remote address
     *
     * I/O helpers mirror the BSD socket API but operate on the owned fd.
     */
    class UdpSocket
    {
    public:
        ~UdpSocket();

        UdpSocket(const UdpSocket &) = delete;
        UdpSocket &operator=(const UdpSocket &) = delete;
        UdpSocket(UdpSocket &&) noexcept;
        UdpSocket &operator=(UdpSocket &&) noexcept;

        // Factories

        /**
         * @brief Create UDP socket and bind it to addr:port (server mode).
         *
         * When addr is empty, binds to all interfaces. Prefers  AF_INET6 dual-stack socket
         *
         * @param addr  Numeric IP string or empty for wildcard.
         * @param port  Port number in host byte order.
         */
        [[nodiscard]] static UdpSocket bind_server(const std::string &addr, uint16_t port);

        /**
         * @brief Create a UDP socket and connect() it to the given remote address.
         *
         * After connect(), ::send() / ::recv() work without specifying the
         * destination every time. The family is inferred from remote.ss_family.
         *
         * @param remote      Resolved remote address (from DnsResolver).
         * @param remote_len  Length of the remote address structure.
         */
        [[nodiscard]] static UdpSocket connect_client(const sockaddr_storage &remote, socklen_t remote_len);


        ///@brief Raw file descriptor - safe to pass to poll()/select()
        [[nodiscard]] int fd() const noexcept { return fd_; }

        ///@brief Address family (AF_INET or AF_INET6). 
        [[nodiscard]] int family() const noexcept { return family_; }

    private:
        explicit UdpSocket(int fd, int family) noexcept;

        int fd_;
        int family_;
    };

} // namespace rdt::infra

#endif // UDP_SOCKET_HPP
