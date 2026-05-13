/**
 * @file udp_socket.cpp
 * @brief Implementation of UdpSocket - RAII UDP socket wrapper.
 * @author Peter Stahl (xstahl01)
 */

#include "udp_socket.hpp"
#include "exceptions.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <system_error>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

namespace rdt::infra
{

    UdpSocket::UdpSocket(int fd, int family) noexcept : fd_(fd), family_(family) {}

    UdpSocket::~UdpSocket()
    {
        if (fd_ >= 0)
            ::close(fd_);
    }

    UdpSocket::UdpSocket(UdpSocket &&o) noexcept : fd_(o.fd_), family_(o.family_)
    {
        o.fd_ = -1;
    }

    UdpSocket &UdpSocket::operator=(UdpSocket &&o) noexcept
    {
        if (this != &o)
        {
            if (fd_ >= 0)
                ::close(fd_);
            fd_ = o.fd_;
            family_ = o.family_;
            o.fd_ = -1;
        }
        return *this;
    }


    UdpSocket UdpSocket::bind_server(const std::string &addr, uint16_t port)
    {
        const std::string port_str = std::to_string(port);
        const char *node = addr.empty() ? nullptr : addr.c_str();

        addrinfo hints{};
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE; // wildcard when node is null

        // Prefer dual-stack IPv6; fall back to IPv4 if the system lacks it. (i guess)
        for (int family : {AF_INET6, AF_INET})
        {
            hints.ai_family = family;

            addrinfo *res = nullptr;
            if (int rc = ::getaddrinfo(node, port_str.c_str(), &hints, &res); rc != 0)
                continue; // try next family

            int fd = -1;
            int bound_family = AF_UNSPEC;

            for (addrinfo *p = res; p != nullptr; p = p->ai_next)
            {
                fd = ::socket(p->ai_family, SOCK_DGRAM, 0);
                if (fd < 0)
                    continue;

                // Enable IPv4-mapped addresses on the dual-stack socket.
                if (p->ai_family == AF_INET6)
                {
                    int off = 0;
                    ::setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof(off));
                }

                // Allow quick restart when the port is in TIME_WAIT.
                int reuse = 1;
                ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

                if (::bind(fd, p->ai_addr, p->ai_addrlen) == 0)
                {
                    bound_family = p->ai_family;
                    break;
                }

                ::close(fd);
                fd = -1;
            }

            ::freeaddrinfo(res);

            if (fd >= 0)
                return UdpSocket{fd, bound_family};
        }

        throw NetworkException("UdpSocket: could not bind UDP socket on port" + port_str + (addr.empty() ? "" : " address " + addr));
    }

    UdpSocket UdpSocket::connect_client(const sockaddr_storage &remote, socklen_t remote_len)
    {
        const int family = remote.ss_family;

        int fd = ::socket(family, SOCK_DGRAM, 0);
        if (fd < 0)
            throw NetworkException("UdpSocket: socket creation failed", errno);

        // UDP connect() records the peer address; subsequent ::send() calls
        // do not need to specify the destination explicitly.
        if (::connect(fd, static_cast<const sockaddr*>(static_cast<const void*>(&remote)), remote_len) != 0)
        {
            const int saved = errno;
            ::close(fd);
            throw NetworkException("UdpSocket: connect failed", saved);
        }

        return UdpSocket{fd, family};
    }

    ///@brief Dead code of original UDP Socket which would introduce thin 
    /** 
    ssize_t UdpSocket::send_pkt(const void *buf, std::size_t len) const
    {
        return ::send(fd_, buf, len, 0);
    }

    ssize_t UdpSocket::sendto_pkt(const void *buf, std::size_t len, const sockaddr_storage &dst, socklen_t dst_len) const
    {
        return ::sendto(fd_, buf, len, 0, reinterpret_cast<const sockaddr *>(&dst), dst_len);
    }

    ssize_t UdpSocket::recvfrom_pkt(void *buf, std::size_t len, sockaddr_storage &src, socklen_t &src_len) const
    {
        return ::recvfrom(fd_, buf, len, 0, reinterpret_cast<sockaddr *>(&src), &src_len);
    }
    */
    
} // namespace rdt::infra
