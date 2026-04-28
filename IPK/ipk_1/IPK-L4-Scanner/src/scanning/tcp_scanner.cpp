/**
 * @file tcp_scanner.cpp
 * @brief TCP port scanner implementation
 * @author Peter Stahl (xstahl01)
 */

#include <cstring>
#include <netinet/tcp.h>
#include <random>

#include "tcp_scanner.hpp"
#include "socket_guard.hpp"
#include "task_tracker.hpp"
#include "exceptions.hpp"

ScanResult TCPScanner::scan(const ScanTask &task) const
{
    uint16_t src_port = pick_src_port();
    ScanKey key = {task.target.ip_str, task.port, Protocol::TCP};


    // promise setup for async result retrieval
    std::promise<PortState> promise;
    auto future = promise.get_future();
    TaskGuard guard(key, promise);

    // build and send packet    
    int protocol = (task.target.family == AF_INET) ? IPPROTO_RAW : IPPROTO_TCP;
    auto socket = SocketGuard::open(task.target.family, SOCK_RAW, protocol);

    std::vector<uint8_t> packet;
    if (task.target.family == AF_INET){
        packet = build_packet_ipv4(task, src_port);
        socket.set_hdrincl();
    } else {
        in6_addr src_addr;
        std::string src_ip_str;
        get_interface_ip_v6(get_iface(), src_addr, src_ip_str);  // throws on failure
        packet = build_packet_ipv6(task, src_addr);

        sockaddr_in6 src_sock{};
        src_sock.sin6_family = AF_INET6;
        src_sock.sin6_addr = src_addr;
        src_sock.sin6_port = htons(pick_src_port());
        bind_to_address(socket.get_fd(), reinterpret_cast<sockaddr*>(&src_sock), sizeof(src_sock));
    } 
    

    auto send_packet = [&]()
    { //TCP sendto ???
        return ::sendto(
            socket.get_fd(),
            packet.data(),
            packet.size(),
            0,
            reinterpret_cast<const sockaddr *>(&task.target.addr),
            task.target.addr_len
        );
    };

    if (send_packet() < 0)
    {   
        //not needed since TaskGuard will handle cleanup on exit - lovely RAII :DD
        // TaskTracker::instance().remove_task(key);
        throw SystemError("Failed to send packet: " + std::string(strerror(errno)));
    }

    //  Wait for Listener or Timeout
    if (future.wait_for(std::chrono::milliseconds(get_timeout_ms())) == std::future_status::ready) {
        return {task.target.ip_str, task.port, Protocol::TCP, future.get()};
    }

    // Retry logic on timeout
    if (is_running()){
        std::promise<PortState> retry_promise;
        auto retry_future = retry_promise.get_future();
        TaskTracker::instance().register_task(key, &retry_promise); 

        send_packet(); // resend packet

        if (retry_future.wait_for(std::chrono::milliseconds(get_timeout_ms())) == std::future_status::ready) {
            return {task.target.ip_str, task.port, Protocol::TCP, retry_future.get()};
        }
    }
    // exiting wihout removing task - TaskGuard again 
    return {task.target.ip_str, task.port, Protocol::TCP, PortState::FILTERED};
}


std::vector<uint8_t> TCPScanner::build_packet_ipv4(const ScanTask &task, uint16_t src_port) const
{
    auto buf = build_ipv4_base(task, IPPROTO_TCP, sizeof(tcphdr));
    
    auto *ip = reinterpret_cast<iphdr *>(buf.data());
    auto *tcp = reinterpret_cast<tcphdr *>(buf.data() + sizeof(iphdr));

    tcp->source = htons(src_port);
    tcp->dest = htons(task.port);
    tcp->seq = htonl(random_isn());
    tcp->doff = 5;
    tcp->syn = 1;
    tcp->window = htons(1024);
    
    tcp->check = transport_checksum_ipv4(ip, std::span<const std::byte>(reinterpret_cast<const std::byte *>(tcp), sizeof(tcphdr)), IPPROTO_TCP);

    return buf;
}

std::vector<uint8_t> TCPScanner::build_packet_ipv6(const ScanTask& task, const in6_addr& src_addr) const {
    std::vector<uint8_t> buf(sizeof(tcphdr), 0);
    auto* tcp = reinterpret_cast<tcphdr*>(buf.data());

    tcp->source = htons(pick_src_port());
    tcp->dest = htons(task.port);
    tcp->seq = htonl(random_isn());
    tcp->doff = 5;
    tcp->syn = 1;
    tcp->window = htons(1024);

    // Get destination address from task
    const auto* dst_addr = &reinterpret_cast<const sockaddr_in6*>(&task.target.addr)->sin6_addr;

    // Compute checksum
    tcp->check = transport_checksum_ipv6(src_addr, *dst_addr, std::span<const std::byte>(reinterpret_cast<const std::byte*>(tcp), sizeof(tcphdr)),IPPROTO_TCP);
    return buf;
}

uint32_t TCPScanner::random_isn() {
    thread_local std::mt19937 rng(std::random_device{}());
    thread_local std::uniform_int_distribution<uint32_t> dist;
    return dist(rng);
}