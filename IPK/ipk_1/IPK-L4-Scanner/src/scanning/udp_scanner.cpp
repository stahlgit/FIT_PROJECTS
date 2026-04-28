#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <netinet/udp.h>

#include "udp_scanner.hpp"
#include "socket_guard.hpp"
#include "exceptions.hpp"
#include "task_tracker.hpp"

ScanResult UDPScanner::scan(const ScanTask &task) const
{
    ScanKey key = {task.target.ip_str, task.port, Protocol::UDP};

    std::promise<PortState> promise;
    auto future = promise.get_future();
    TaskGuard guard(key, promise);

    int protocol = (task.target.family == AF_INET) ? IPPROTO_RAW : IPPROTO_UDP;
    auto socket = SocketGuard::open(task.target.family, SOCK_RAW, protocol);

    std::vector<uint8_t> packet;
    if (task.target.family == AF_INET){
        packet = build_packet_ipv4(task);
        socket.set_hdrincl();
    }
    else {
        in6_addr src_addr;
        std::string src_ip_str;
        get_interface_ip_v6(get_iface(), src_addr, src_ip_str); //TODO: also check
        packet = build_packet_ipv6(task, src_addr);

        sockaddr_in6 src_sock{};
        src_sock.sin6_family = AF_INET6;
        src_sock.sin6_addr = src_addr;
        src_sock.sin6_port = htons(pick_src_port());
        bind_to_address(socket.get_fd(), reinterpret_cast<sockaddr *>(&src_sock), sizeof(src_sock));
    }
    // check send to result
    if (::sendto(
            socket.get_fd(),
            packet.data(),
            packet.size(),
            0,
            reinterpret_cast<const sockaddr *>(&task.target.addr),
            task.target.addr_len) < 0)
    {
        throw PcapError("sendto failed");
    }

    // wait for icmp unreachable
    if (future.wait_for(std::chrono::milliseconds(get_timeout_ms())) == std::future_status::ready)
    {
        return {task.target.ip_str, task.port, Protocol::UDP, future.get()};
    }

    return {task.target.ip_str, task.port, Protocol::UDP, PortState::OPEN};
}

std::vector<uint8_t> UDPScanner::build_packet_ipv4(const ScanTask &task) const
{
    auto buf = build_ipv4_base(task, IPPROTO_UDP, sizeof(udphdr));

    auto *ip = reinterpret_cast<iphdr *>(buf.data());
    auto *udp = reinterpret_cast<udphdr *>(buf.data() + sizeof(iphdr));

    udp->source = htons(pick_src_port());
    udp->dest = htons(task.port);
    udp->len = htons(sizeof(udphdr));

    udp->check = transport_checksum_ipv4(ip, std::span<const std::byte>(reinterpret_cast<const std::byte *>(udp), sizeof(udphdr)), IPPROTO_UDP);

    return buf;
}

std::vector<uint8_t> UDPScanner::build_packet_ipv6(const ScanTask &task, const in6_addr &src_addr) const
{
    std::vector<uint8_t> buf(sizeof(udphdr), 0);
    auto *udp = reinterpret_cast<udphdr *>(buf.data());

    udp->source = htons(pick_src_port());
    udp->dest = htons(task.port);
    udp->len = htons(sizeof(udphdr));

    const auto *dst_addr = &reinterpret_cast<const sockaddr_in6 *>(&task.target.addr)->sin6_addr;

    udp->check = transport_checksum_ipv6(src_addr, *dst_addr, std::span<const std::byte>(reinterpret_cast<const std::byte *>(udp), sizeof(udphdr)), IPPROTO_UDP);
    return buf;
}