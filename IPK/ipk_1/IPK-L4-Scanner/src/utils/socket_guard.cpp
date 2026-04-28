/**
 * @file socket_guard.cpp
 * @brief Implementation of SocketGuard class
 * @author Peter Stahl (xstahl01)
 * // adapted from previous coursework https://github.com/stahlgit/FIT_PROJECTS
 */

#include <sys/socket.h>
#include <stdexcept>
#include <netinet/in.h>

#include "socket_guard.hpp"
#include "exceptions.hpp"
#include <cstring>

SocketGuard SocketGuard::open(int family, int type, int protocol) {
    int fd = ::socket(family, type, protocol); //global namespace to avoid confusion with member function
    if (fd < 0) {
        throw SystemError("Failed to open socket:" + std::string(strerror(errno)));
    }
    return SocketGuard(fd, family);
}

void SocketGuard::set_recv_timeout(int timeout_ms) const {
    struct timeval tv;
    tv.tv_sec  = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

void SocketGuard::set_hdrincl() const {
    if (family_ != AF_INET){
        throw SystemError("IP_HDRINCL is only supported for AF_INET sockets");
    }
    int one = 1;
    if (setsockopt(fd_, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0)
        throw SystemError("Failed to set IP_HDRINCL");
}