/**
 * @file socket_guard.hpp
 * @brief RAII wrapper for socket file descriptors
 * @author Peter Stahl (xstahl01)
 * adapted from previous coursework https://github.com/stahlgit/FIT_PROJECTS
 */
#pragma once
#ifndef SOCKET_GUARD_HPP
#define SOCKET_GUARD_HPP

#include <unistd.h>

class SocketGuard {
public:
    /**
     * @brief Contructor with already-opened file descriptor. Takes ownership - will close on destruction.
     * @param fd The file descriptor to manage
     * @param family The address family (e.g., AF_INET)
     */
    explicit SocketGuard(int fd, int family = AF_UNSPEC): fd_ (fd),  family_(family) {}

    /**
     * @brief Destructor - closes the file descriptor if it's still open
     */
    ~SocketGuard() { close_fd(); }

    /**
     * @brief Open a new socket. 
     * @param family The address family (e.g., AF_INET)
     * @param type The socket type (e.g., SOCK_STREAM)
     * @param protocol The protocol (e.g., 0 for default)
     * @throws std::runtime_error if socket creation fails
     * @return A SocketGuard managing the newly created socket
     */
    static SocketGuard open (int family, int type, int protocol);

    SocketGuard(const SocketGuard&) = delete; // No copy
    SocketGuard& operator=(const SocketGuard&) = delete; // No copy assignment

    
    SocketGuard(SocketGuard&& other) noexcept : fd_(other.fd_), family_(other.family_) { 
        other.fd_ = -1; // Prevent double close
    }

    SocketGuard& operator=(SocketGuard&& other) noexcept {
        if (this != &other){
            close_fd();
            fd_ = other.fd_;
            family_ = other.family_;
            other.fd_ = -1; 
        }
        return *this;
    }

    bool is_valid() const {return fd_ >= 0; }
    int get_fd() const { return fd_; }

    /**
     * @brief Set socket receive timeout.
     * @param timeout_ms Timeout in milliseconds.
     */
    void set_recv_timeout(int timeout_ms) const;

    /**
     * @brief Enable IP_HDRINCL — tells kernel we are building IP header ourselves.
     *        Required for raw TCP/UDP scanning.
     */
    void set_hdrincl() const;

    /**
     * @brief Get the address family of the socket (e.g., AF_INET, AF_INET6).
     */
    int get_family() const { return family_; }

private:
    int fd_;
    int family_;
    void close_fd(){
        if (fd_ >= 0){
            ::close(fd_);
            fd_ = -1;
        }
    }
};


#endif  // SOCKET_GUARD_HPP