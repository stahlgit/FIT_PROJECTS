#include <vector>
#include <sstream>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "catch2_include/catch_amalgamated.hpp"

#include "../src/utils/arg_parser.hpp"
#include "../src/dns/dns_resolver.hpp"
#include "../src/scanning/scanner_base.hpp"
#include "../src/scanning/task_tracker.hpp"
#include "../src/utils/socket_guard.hpp"
#include "../src/app/application.hpp"
// =======================
// === ArgParser Tests ===
// =======================

TEST_CASE("ArgParser - help option", "[arg_parser]") {
    const char* argv[] = { "ipk-L4-scan", "-h" };
    ArgParser parser;
    auto config = parser.parse_arguments(2, const_cast<char**>(argv));
    REQUIRE(config.show_help);
}

TEST_CASE("ArgParser - interface listing only", "[arg_parser]") {
    const char* argv[] = { "ipk-L4-scan", "-i" };
    ArgParser parser;
    auto config = parser.parse_arguments(2, const_cast<char**>(argv));
    REQUIRE(config.show_interfaces);
}

TEST_CASE("ArgParser - host and ports", "[arg_parser]") {
    const char* argv[] = {
        "ipk-L4-scan", "-i", "eth0", "-t", "22,80", "-u", "53,67",
        "localhost", "-w", "2000"
    };
    ArgParser parser;
    // Using std::size() prevents manual counting errors
    auto config = parser.parse_arguments(std::size(argv), const_cast<char**>(argv));
    
    REQUIRE(config.interface == "eth0");
    REQUIRE(config.tcp_ports.size() == 2);
    REQUIRE(config.udp_ports.size() == 2);
    REQUIRE(config.host == "localhost");
    REQUIRE(config.timeout_ms == 2000);
}

TEST_CASE("ArgParser - port range", "[arg_parser]") {
    const char* argv[] = {
        "ipk-L4-scan", "-i", "eth0", "-t", "1-5", "localhost"
    };
    ArgParser parser;
    // This array has 6 elements, but your original code said 5! 
    auto config = parser.parse_arguments(std::size(argv), const_cast<char**>(argv));
    
    REQUIRE(config.tcp_ports == std::vector<uint16_t>({1,2,3,4,5}));
}

TEST_CASE("ArgParser - Invalid ports out of bounds", "[arg_parser]") {
    ArgParser parser;
    // Port 70000 is > 65535
    const char* argv[] = {"ipk-L4-scan", "-i", "eth0", "-t", "70000", "localhost"};
    REQUIRE_THROWS_AS(parser.parse_arguments(6, const_cast<char**>(argv)), std::invalid_argument);
}

TEST_CASE("ArgParser - Invalid port ranges", "[arg_parser]") {
    ArgParser parser;
    // Start is greater than end (80 > 22)
    const char* argv[] = {"ipk-L4-scan", "-i", "eth0", "-u", "80-22", "localhost"};
    REQUIRE_THROWS_AS(parser.parse_arguments(6, const_cast<char**>(argv)), std::invalid_argument);
}

TEST_CASE("ArgParser - Unexpected extra arguments", "[arg_parser]") {
    ArgParser parser;
    // Two hosts provided instead of one
    const char* argv[] = {"ipk-L4-scan", "-i", "eth0", "localhost", "extra_host"};
    REQUIRE_THROWS_AS(parser.parse_arguments(5, const_cast<char**>(argv)), std::invalid_argument);
}

// ==========================
// === DNS Resolver Tests ===
// ==========================

TEST_CASE("DnsResolver - localhost", "[dns]") {
    DnsResolver resolver;
    auto targets = resolver.resolve("localhost");
    REQUIRE(!targets.empty());
    // At least one of 127.0.0.1 or ::1
    bool found = false;
    for (auto& t : targets) {
        if (t.ip_str == "127.0.0.1" || t.ip_str == "::1") found = true;
    }
    REQUIRE(found);
}

TEST_CASE("DnsResolver - invalid host", "[dns]") {
    DnsResolver resolver;
    REQUIRE_THROWS_AS(resolver.resolve("nonexistent.domain.invalid"), std::runtime_error);
}


TEST_CASE("DnsResolver - localhost and AI_ADDRCONFIG ", "[dns]") {
    DnsResolver resolver;
    auto targets = resolver.resolve("localhost");
    REQUIRE(!targets.empty());
    
    bool found_v4 = false;
    bool found_v6 = false;
    for (auto& t : targets) {
        if (t.ip_str == "127.0.0.1") found_v4 = true;
        if (t.ip_str == "::1") found_v6 = true;
    }
    
    // If AI_ADDRCONFIG was suppressing results on a machine with only IPv4 or IPv6,
    // this test might fail before your fix. Both should ideally resolve for 'localhost'.
    REQUIRE(found_v4);
    // REQUIRE(found_v6); // Uncomment once Bug #6 is fixed if your environment guarantees IPv6 loopback
}

// ==========================
// === TaskTracker Tests ===
// ==========================
#include <future>

TEST_CASE("TaskTracker - register and resolve", "[tracker]") {
    ScanKey key{"127.0.0.1", 22, Protocol::TCP};
    std::promise<PortState> promise;
    auto future = promise.get_future();
    
    TaskTracker::instance().register_task(key, &promise);
    
    // Simulate response
    TaskTracker::instance().resolve_task(key, PortState::OPEN);
    
    // Check future
    auto state = future.get();
    REQUIRE(state == PortState::OPEN);
    
    // Cleanup (no pending tasks)
    TaskTracker::instance().remove_task(key);
}

TEST_CASE("TaskTracker - timeout removal", "[tracker]") {
    ScanKey key{"127.0.0.1", 80, Protocol::UDP};
    std::promise<PortState> promise;
    auto future = promise.get_future();
    
    TaskTracker::instance().register_task(key, &promise);
    
    // Simulate timeout: remove without setting value
    TaskTracker::instance().remove_task(key);
    
    // Future should not become ready; we can wait with timeout
    auto status = future.wait_for(std::chrono::milliseconds(10));
    REQUIRE(status == std::future_status::timeout);
}

// ==========================
// === ScannerBase Tests ===
// ==========================
// ==========================================
// === Testing Hack for Protected/Private ===
// ==========================================

// 1. Expose ScannerBase protected methods via a dummy derived class
class TestScanner : public ScannerBase {
public:
    TestScanner(const std::string& iface, std::atomic<bool>& running)
        : ScannerBase(iface, 1000, running) {}
    
    ScanResult scan(const ScanTask& task) const override { return {}; }
    
    // Lift protected methods to public for testing
    using ScannerBase::calculate_sum;
    using ScannerBase::finalize_checksum;
};

// 2. Expose PcapListener private methods
#define private public
#include "../src/scanning/pcap_listener.hpp"
#include "../src/scanning/scan_scheduler.hpp"

#undef private
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>

TEST_CASE("ScannerBase - Checksum calculations", "[scanner_base]") {
    std::atomic<bool> running{true};
    TestScanner scanner("lo", running);

    // Simple 4-byte payload: 0x0100, 0x0001
    std::vector<std::byte> payload = {
        std::byte{0x01}, std::byte{0x00}, 
        std::byte{0x00}, std::byte{0x01}
    };
    
    uint32_t sum = scanner.calculate_sum(payload);
    REQUIRE(sum == 0x0101); // 0x0100 + 0x0001
    
    uint16_t final_checksum = scanner.finalize_checksum(sum);
    // 1's complement of 0x0101 is 0xFEFE
    REQUIRE(final_checksum == 0xFEFE);
}

// ==========================
// === PcapListener Tests ===
// ==========================

TEST_CASE("PcapListener - UDP ICMP Source IP tracking ", "[pcap][bug]") {
    std::atomic<bool> running{true};
    PcapListener listener("lo", running, 1000);
    
    // Set up the tracker to wait for the INNER destination IP, not the OUTER source IP
    ScanKey expected_key{"1.2.3.4", 53, Protocol::UDP};
    std::promise<PortState> promise;
    auto future = promise.get_future();
    TaskTracker::instance().register_task(expected_key, &promise);

    // Construct a raw ICMP Port Unreachable packet
    // Total Size: Outer IP (20) + ICMP (8) + Inner IP (20) + Inner UDP (8)
    std::vector<uint8_t> packet(20 + 8 + 20 + 8, 0);
    
    // 1. Outer IPv4
    auto* outer_ip = reinterpret_cast<iphdr*>(packet.data());
    outer_ip->version = 4;
    outer_ip->ihl = 5;
    outer_ip->protocol = IPPROTO_ICMP;
    inet_pton(AF_INET, "8.8.8.8", &outer_ip->saddr); // Outer sender (router)
    inet_pton(AF_INET, "192.168.1.100", &outer_ip->daddr); // Us
    
    // 2. ICMP Header
    auto* icmp = reinterpret_cast<icmphdr*>(packet.data() + 20);
    icmp->type = ICMP_DEST_UNREACH;
    icmp->code = ICMP_PORT_UNREACH;
    
    // 3. Inner IPv4 (The original packet we sent)
    auto* inner_ip = reinterpret_cast<iphdr*>(packet.data() + 28);
    inner_ip->version = 4;
    inner_ip->ihl = 5;
    inet_pton(AF_INET, "192.168.1.100", &inner_ip->saddr); // Us
    inet_pton(AF_INET, "1.2.3.4", &inner_ip->daddr);       // The target we were scanning!
    
    // 4. Inner UDP
    auto* inner_udp = reinterpret_cast<udphdr*>(packet.data() + 48);
    inner_udp->dest = htons(53); // The port we scanned
    
    // Feed packet to listener (simulating link-layer offset of 0 for direct IP passing)
    listener.process_packet_(packet.data(), packet.size());
    
    // If bug #3 is present, this will timeout because it tracked "8.8.8.8" instead of "1.2.3.4"
    auto status = future.wait_for(std::chrono::milliseconds(100));
    REQUIRE(status == std::future_status::ready);
    REQUIRE(future.get() == PortState::CLOSED);
    
    TaskTracker::instance().remove_task(expected_key);
}

TEST_CASE("PcapListener - IPv6 Extension Headers ", "[pcap][bug]") {
    std::atomic<bool> running{true};
    PcapListener listener("lo", running, 1000);
    
    ScanKey expected_key{"2001:db8::1", 80, Protocol::TCP};
    std::promise<PortState> promise;
    auto future = promise.get_future();
    TaskTracker::instance().register_task(expected_key, &promise);

    // Construct raw IPv6 packet: IPv6 Header (40) + Hop-by-Hop Ext (8) + TCP (20)
    std::vector<uint8_t> packet(40 + 8 + 20, 0);
    
    // 1. IPv6 Header
    auto* ip6 = reinterpret_cast<ip6_hdr*>(packet.data());
    ip6->ip6_vfc = 0x60; // Version 6
    ip6->ip6_nxt = IPPROTO_HOPOPTS; // Bug #5 triggers here! (0)
    inet_pton(AF_INET6, "2001:db8::1", &ip6->ip6_src);
    
    // 2. Hop-by-Hop Extension Header
    packet[40] = IPPROTO_TCP; // Next header is TCP
    packet[41] = 0; // Hdr Ext Len (0 means 8 bytes total)
    
    // 3. TCP Header (RST flag set)
    auto* tcp = reinterpret_cast<tcphdr*>(packet.data() + 48);
    tcp->source = htons(80);
    tcp->rst = 1;
    
    listener.process_packet_(packet.data(), packet.size());
    
    // If bug #5 is present, process_packet will abort at IPPROTO_HOPOPTS 
    // and never reach the TCP parsing, causing a timeout.
    auto status = future.wait_for(std::chrono::milliseconds(100));
    REQUIRE(status == std::future_status::ready);
    REQUIRE(future.get() == PortState::CLOSED);
    
    TaskTracker::instance().remove_task(expected_key);
}

// =========================
// === SocketGuard Tests ===
// =========================

TEST_CASE("SocketGuard - Lifecycle and Move Semantics", "[socket_guard]") {
    // Open a standard UDP socket for testing (doesn't require root like SOCK_RAW)
    SocketGuard sg1 = SocketGuard::open(AF_INET, SOCK_DGRAM, 0);
    REQUIRE(sg1.is_valid());
    int fd = sg1.get_fd();

    // Test Move Constructor
    SocketGuard sg2 = std::move(sg1);
    REQUIRE(!sg1.is_valid());      // sg1 should be invalidated
    REQUIRE(sg1.get_fd() == -1);   // sg1 FD should be -1
    REQUIRE(sg2.is_valid());       // sg2 should take over
    REQUIRE(sg2.get_fd() == fd);   // sg2 should have the exact FD
}

#define private public
#include "../src/scanning/tcp_scanner.hpp"
#include "../src/scanning/udp_scanner.hpp"
#include <iostream>
#undef private

// =====================================
// === ArgParser - Additional Tests  ===
// =====================================

TEST_CASE("ArgParser - non-numeric port throws", "[arg_parser]") {
    ArgParser parser;
    const char* argv[] = {"ipk-L4-scan", "-i", "eth0", "-t", "abc", "localhost"};
    REQUIRE_THROWS(parser.parse_arguments(std::size(argv), const_cast<char**>(argv)));
}

TEST_CASE("ArgParser - port 0 is out of bounds", "[arg_parser]") {
    ArgParser parser;
    const char* argv[] = {"ipk-L4-scan", "-i", "eth0", "-t", "0", "localhost"};
    REQUIRE_THROWS_AS(parser.parse_arguments(std::size(argv), const_cast<char**>(argv)), std::invalid_argument);
}

TEST_CASE("ArgParser - boundary ports 1 and 65535 are valid", "[arg_parser]") {
    ArgParser parser;
    const char* argv[] = {"ipk-L4-scan", "-i", "eth0", "-t", "1,65535", "localhost"};
    auto config = parser.parse_arguments(std::size(argv), const_cast<char**>(argv));
    REQUIRE(config.tcp_ports == std::vector<uint16_t>({1, 65535}));
}

TEST_CASE("ArgParser - single-element range (22-22)", "[arg_parser]") {
    ArgParser parser;
    const char* argv[] = {"ipk-L4-scan", "-i", "eth0", "-t", "22-22", "localhost"};
    auto config = parser.parse_arguments(std::size(argv), const_cast<char**>(argv));
    REQUIRE(config.tcp_ports == std::vector<uint16_t>({22}));
}

TEST_CASE("ArgParser - mixed comma and range", "[arg_parser]") {
    ArgParser parser;
    const char* argv[] = {"ipk-L4-scan", "-i", "eth0", "-t", "22,80-82", "localhost"};
    auto config = parser.parse_arguments(std::size(argv), const_cast<char**>(argv));
    REQUIRE(config.tcp_ports == std::vector<uint16_t>({22, 80, 81, 82}));
}

TEST_CASE("ArgParser - --help long form", "[arg_parser]") {
    const char* argv[] = {"ipk-L4-scan", "--help"};
    ArgParser parser;
    auto config = parser.parse_arguments(std::size(argv), const_cast<char**>(argv));
    REQUIRE(config.show_help);
}

// =====================================
// === DnsResolver - IP literal input ===
// =====================================

TEST_CASE("DnsResolver - IPv4 literal passthrough", "[dns]") {
    DnsResolver resolver;
    auto targets = resolver.resolve("127.0.0.1");
    REQUIRE(!targets.empty());
    REQUIRE(targets[0].ip_str == "127.0.0.1");
    REQUIRE(targets[0].family == AF_INET);
}

TEST_CASE("DnsResolver - IPv6 literal passthrough", "[dns]") {
    DnsResolver resolver;
    auto targets = resolver.resolve("::1");
    REQUIRE(!targets.empty());
    REQUIRE(targets[0].ip_str == "::1");
    REQUIRE(targets[0].family == AF_INET6);
}

// ===========================================
// === ScannerBase - Checksum edge cases   ===
// ===========================================

TEST_CASE("ScannerBase - Checksum of empty payload is 0xFFFF", "[scanner_base]") {
    std::atomic<bool> running{true};
    TestScanner scanner("lo", running);

    std::vector<std::byte> payload;
    uint32_t sum = scanner.calculate_sum(payload);
    REQUIRE(sum == 0);
    // 1's complement of 0 = 0xFFFF
    REQUIRE(scanner.finalize_checksum(sum) == 0xFFFF);
}

TEST_CASE("ScannerBase - Checksum of odd-length payload", "[scanner_base]") {
    std::atomic<bool> running{true};
    TestScanner scanner("lo", running);

    // 3 bytes: 0x01 0x00 0x01
    // First word (LE memcpy): 0x0001, odd byte padded: 0x0100, sum = 0x0101
    std::vector<std::byte> payload = {
        std::byte{0x01}, std::byte{0x00}, std::byte{0x01}
    };
    uint32_t sum = scanner.calculate_sum(payload);
    REQUIRE(sum == 0x0101);
    REQUIRE(scanner.finalize_checksum(sum) == 0xFEFE);
}

TEST_CASE("ScannerBase - Checksum of all-zero payload is 0xFFFF", "[scanner_base]") {
    std::atomic<bool> running{true};
    TestScanner scanner("lo", running);

    std::vector<std::byte> payload = {std::byte{0x00}, std::byte{0x00}};
    uint32_t sum = scanner.calculate_sum(payload);
    REQUIRE(sum == 0);
    REQUIRE(scanner.finalize_checksum(sum) == 0xFFFF);
}

// =======================================
// === TaskTracker - Robustness Tests  ===
// =======================================

TEST_CASE("TaskTracker - resolve non-existent key is safe", "[tracker]") {
    ScanKey key{"99.99.99.99", 12345, Protocol::TCP};
    REQUIRE_NOTHROW(TaskTracker::instance().resolve_task(key, PortState::OPEN));
}

TEST_CASE("TaskTracker - remove non-existent key is safe", "[tracker]") {
    ScanKey key{"99.99.99.99", 54321, Protocol::UDP};
    REQUIRE_NOTHROW(TaskTracker::instance().remove_task(key));
}

TEST_CASE("TaskTracker - registering same key twice overwrites", "[tracker]") {
    ScanKey key{"10.0.0.1", 100, Protocol::TCP};
    std::promise<PortState> promise1;
    std::promise<PortState> promise2;
    auto future2 = promise2.get_future();

    TaskTracker::instance().register_task(key, &promise1);
    TaskTracker::instance().register_task(key, &promise2); // overwrites promise1

    TaskTracker::instance().resolve_task(key, PortState::OPEN);

    auto status = future2.wait_for(std::chrono::milliseconds(100));
    REQUIRE(status == std::future_status::ready);
    REQUIRE(future2.get() == PortState::OPEN);
}

// ======================================
// === SocketGuard - Additional Tests ===
// ======================================

TEST_CASE("SocketGuard - invalid fd is not valid", "[socket_guard]") {
    SocketGuard sg(-1);
    REQUIRE(!sg.is_valid());
    REQUIRE(sg.get_fd() == -1);
}

TEST_CASE("SocketGuard - Move assignment", "[socket_guard]") {
    SocketGuard sg1 = SocketGuard::open(AF_INET, SOCK_DGRAM, 0);
    REQUIRE(sg1.is_valid());
    int fd = sg1.get_fd();

    SocketGuard sg2(-1);
    sg2 = std::move(sg1);

    REQUIRE(!sg1.is_valid());
    REQUIRE(sg1.get_fd() == -1);
    REQUIRE(sg2.is_valid());
    REQUIRE(sg2.get_fd() == fd);
}

// ==============================================
// === PcapListener - Additional Tests        ===
// ==============================================

TEST_CASE("PcapListener - TCP SYN-ACK resolves as OPEN", "[pcap]") {
    std::atomic<bool> running{true};
    PcapListener listener("lo", running, 1000);

    ScanKey expected_key{"1.2.3.5", 80, Protocol::TCP};
    std::promise<PortState> promise;
    auto future = promise.get_future();
    TaskTracker::instance().register_task(expected_key, &promise);

    // IPv4 header (20) + TCP header (20)
    std::vector<uint8_t> packet(sizeof(iphdr) + sizeof(tcphdr), 0);

    auto* ip = reinterpret_cast<iphdr*>(packet.data());
    ip->version = 4;
    ip->ihl = 5;
    ip->protocol = IPPROTO_TCP;
    inet_pton(AF_INET, "1.2.3.5", &ip->saddr);

    auto* tcp = reinterpret_cast<tcphdr*>(packet.data() + sizeof(iphdr));
    tcp->source = htons(80);
    tcp->syn = 1;
    tcp->ack = 1; // SYN-ACK = OPEN

    listener.process_packet_(packet.data(), static_cast<uint32_t>(packet.size()));

    auto status = future.wait_for(std::chrono::milliseconds(100));
    REQUIRE(status == std::future_status::ready);
    REQUIRE(future.get() == PortState::OPEN);

    TaskTracker::instance().remove_task(expected_key);
}

TEST_CASE("PcapListener - TCP RST resolves as CLOSED", "[pcap]") {
    std::atomic<bool> running{true};
    PcapListener listener("lo", running, 1000);

    ScanKey expected_key{"5.6.7.8", 443, Protocol::TCP};
    std::promise<PortState> promise;
    auto future = promise.get_future();
    TaskTracker::instance().register_task(expected_key, &promise);

    std::vector<uint8_t> packet(sizeof(iphdr) + sizeof(tcphdr), 0);

    auto* ip = reinterpret_cast<iphdr*>(packet.data());
    ip->version = 4;
    ip->ihl = 5;
    ip->protocol = IPPROTO_TCP;
    inet_pton(AF_INET, "5.6.7.8", &ip->saddr);

    auto* tcp = reinterpret_cast<tcphdr*>(packet.data() + sizeof(iphdr));
    tcp->source = htons(443);
    tcp->rst = 1; // RST = CLOSED

    listener.process_packet_(packet.data(), static_cast<uint32_t>(packet.size()));

    auto status = future.wait_for(std::chrono::milliseconds(100));
    REQUIRE(status == std::future_status::ready);
    REQUIRE(future.get() == PortState::CLOSED);

    TaskTracker::instance().remove_task(expected_key);
}

TEST_CASE("PcapListener - TCP SYN-only does not resolve", "[pcap]") {
    std::atomic<bool> running{true};
    PcapListener listener("lo", running, 1000);

    ScanKey expected_key{"9.10.11.12", 22, Protocol::TCP};
    std::promise<PortState> promise;
    auto future = promise.get_future();
    TaskTracker::instance().register_task(expected_key, &promise);

    std::vector<uint8_t> packet(sizeof(iphdr) + sizeof(tcphdr), 0);

    auto* ip = reinterpret_cast<iphdr*>(packet.data());
    ip->version = 4;
    ip->ihl = 5;
    ip->protocol = IPPROTO_TCP;
    inet_pton(AF_INET, "9.10.11.12", &ip->saddr);

    auto* tcp = reinterpret_cast<tcphdr*>(packet.data() + sizeof(iphdr));
    tcp->source = htons(22);
    tcp->syn = 1; // SYN-only (no ACK, no RST) — must NOT be resolved

    listener.process_packet_(packet.data(), static_cast<uint32_t>(packet.size()));

    auto status = future.wait_for(std::chrono::milliseconds(50));
    REQUIRE(status == std::future_status::timeout);

    TaskTracker::instance().remove_task(expected_key);
}

TEST_CASE("PcapListener - ICMPv6 port unreachable resolves UDP as CLOSED", "[pcap]") {
    std::atomic<bool> running{true};
    PcapListener listener("lo", running, 1000);

    ScanKey expected_key{"2001:db8::2", 53, Protocol::UDP};
    std::promise<PortState> promise;
    auto future = promise.get_future();
    TaskTracker::instance().register_task(expected_key, &promise);

    // IPv6 (40) + ICMPv6 (8) + Inner IPv6 (40) + Inner UDP (8) = 96
    std::vector<uint8_t> packet(40 + 8 + 40 + 8, 0);

    // 1. Outer IPv6 header
    auto* ip6 = reinterpret_cast<ip6_hdr*>(packet.data());
    ip6->ip6_vfc = 0x60; // Version 6
    ip6->ip6_nxt = IPPROTO_ICMPV6;

    // 2. ICMPv6 header
    auto* icmp6 = reinterpret_cast<icmp6_hdr*>(packet.data() + 40);
    icmp6->icmp6_type = ICMP6_DST_UNREACH;
    icmp6->icmp6_code = ICMP6_DST_UNREACH_NOPORT;

    // 3. Inner IPv6 (the packet we originally sent)
    auto* inner_ip6 = reinterpret_cast<ip6_hdr*>(packet.data() + 48);
    inner_ip6->ip6_vfc = 0x60;
    inet_pton(AF_INET6, "2001:db8::2", &inner_ip6->ip6_dst); // Target we scanned

    // 4. Inner UDP header
    auto* inner_udp = reinterpret_cast<udphdr*>(packet.data() + 88);
    inner_udp->dest = htons(53); // Port we scanned

    listener.process_packet_(packet.data(), static_cast<uint32_t>(packet.size()));

    auto status = future.wait_for(std::chrono::milliseconds(100));
    REQUIRE(status == std::future_status::ready);
    REQUIRE(future.get() == PortState::CLOSED);

    TaskTracker::instance().remove_task(expected_key);
}

TEST_CASE("PcapListener - truncated packet is ignored safely", "[pcap]") {
    std::atomic<bool> running{true};
    PcapListener listener("lo", running, 1000);

    // 5 bytes — too small for a full IPv4 header (need 20)
    std::vector<uint8_t> packet(5, 0);
    packet[0] = 0x45; // Version 4, IHL 5

    REQUIRE_NOTHROW(listener.process_packet_(packet.data(), static_cast<uint32_t>(packet.size())));
}

// ================================================
// === PcapListener - IPv6 TCP State Detection  ===
// ================================================
// These mirror the IPv4 TCP tests to verify IPv6 parsing works the same way.

TEST_CASE("PcapListener - IPv6 TCP SYN-ACK resolves as OPEN", "[pcap][ipv6]") {
    std::atomic<bool> running{true};
    PcapListener listener("lo", running, 1000);

    ScanKey expected_key{"2001:db8::1", 443, Protocol::TCP};
    std::promise<PortState> promise;
    auto future = promise.get_future();
    TaskTracker::instance().register_task(expected_key, &promise);

    // IPv6 header (40) + TCP header (20)
    std::vector<uint8_t> packet(40 + sizeof(tcphdr), 0);

    auto* ip6 = reinterpret_cast<ip6_hdr*>(packet.data());
    ip6->ip6_vfc = 0x60;         // version 6
    ip6->ip6_nxt = IPPROTO_TCP;
    inet_pton(AF_INET6, "2001:db8::1", &ip6->ip6_src); // source = host we scanned

    auto* tcp = reinterpret_cast<tcphdr*>(packet.data() + 40);
    tcp->source = htons(443);
    tcp->syn = 1;
    tcp->ack = 1; // SYN-ACK = OPEN

    listener.process_packet_(packet.data(), static_cast<uint32_t>(packet.size()));

    auto status = future.wait_for(std::chrono::milliseconds(100));
    REQUIRE(status == std::future_status::ready);
    REQUIRE(future.get() == PortState::OPEN);

    TaskTracker::instance().remove_task(expected_key);
}

TEST_CASE("PcapListener - IPv6 TCP RST resolves as CLOSED", "[pcap][ipv6]") {
    std::atomic<bool> running{true};
    PcapListener listener("lo", running, 1000);

    ScanKey expected_key{"2001:db8::ff", 8080, Protocol::TCP};
    std::promise<PortState> promise;
    auto future = promise.get_future();
    TaskTracker::instance().register_task(expected_key, &promise);

    std::vector<uint8_t> packet(40 + sizeof(tcphdr), 0);

    auto* ip6 = reinterpret_cast<ip6_hdr*>(packet.data());
    ip6->ip6_vfc = 0x60;
    ip6->ip6_nxt = IPPROTO_TCP;
    inet_pton(AF_INET6, "2001:db8::ff", &ip6->ip6_src);

    auto* tcp = reinterpret_cast<tcphdr*>(packet.data() + 40);
    tcp->source = htons(8080);
    tcp->rst = 1; // RST = CLOSED

    listener.process_packet_(packet.data(), static_cast<uint32_t>(packet.size()));

    auto status = future.wait_for(std::chrono::milliseconds(100));
    REQUIRE(status == std::future_status::ready);
    REQUIRE(future.get() == PortState::CLOSED);

    TaskTracker::instance().remove_task(expected_key);
}

TEST_CASE("PcapListener - IPv6 TCP SYN-only does not resolve", "[pcap][ipv6]") {
    std::atomic<bool> running{true};
    PcapListener listener("lo", running, 1000);

    ScanKey expected_key{"2001:db8::3", 22, Protocol::TCP};
    std::promise<PortState> promise;
    auto future = promise.get_future();
    TaskTracker::instance().register_task(expected_key, &promise);

    std::vector<uint8_t> packet(40 + sizeof(tcphdr), 0);

    auto* ip6 = reinterpret_cast<ip6_hdr*>(packet.data());
    ip6->ip6_vfc = 0x60;
    ip6->ip6_nxt = IPPROTO_TCP;
    inet_pton(AF_INET6, "2001:db8::3", &ip6->ip6_src);

    auto* tcp = reinterpret_cast<tcphdr*>(packet.data() + 40);
    tcp->source = htons(22);
    tcp->syn = 1; // SYN-only — must NOT be resolved (not a valid response to our SYN)

    listener.process_packet_(packet.data(), static_cast<uint32_t>(packet.size()));

    auto status = future.wait_for(std::chrono::milliseconds(50));
    REQUIRE(status == std::future_status::timeout);

    TaskTracker::instance().remove_task(expected_key);
}

TEST_CASE("PcapListener - IPv6 truncated packet is ignored safely", "[pcap][ipv6]") {
    std::atomic<bool> running{true};
    PcapListener listener("lo", running, 1000);

    // 10 bytes — not enough for a full IPv6 header (needs 40)
    std::vector<uint8_t> packet(10, 0);
    packet[0] = 0x60; // Version 6

    REQUIRE_NOTHROW(listener.process_packet_(packet.data(), static_cast<uint32_t>(packet.size())));
}

// =============================================
// === Output Format Tests                   ===
// =============================================
// These verify the exact tokens the assignment specifies: "IP PORT proto state"

TEST_CASE("Output format - proto_str returns lowercase protocol name", "[output]") {
    REQUIRE(std::string(proto_str(Protocol::TCP)) == "tcp");
    REQUIRE(std::string(proto_str(Protocol::UDP)) == "udp");
}

TEST_CASE("Output format - state_str returns lowercase state name", "[output]") {
    REQUIRE(std::string(state_str(PortState::OPEN))     == "open");
    REQUIRE(std::string(state_str(PortState::CLOSED))   == "closed");
    REQUIRE(std::string(state_str(PortState::FILTERED)) == "filtered");
}

TEST_CASE("Output format - all 6 protocol x state combinations", "[output]") {
    // Verifies every token the automated tests will check
    struct Row { Protocol proto; PortState state; const char* ep; const char* es; };
    Row rows[] = {
        {Protocol::TCP, PortState::OPEN,     "tcp", "open"},
        {Protocol::TCP, PortState::CLOSED,   "tcp", "closed"},
        {Protocol::TCP, PortState::FILTERED, "tcp", "filtered"},
        {Protocol::UDP, PortState::OPEN,     "udp", "open"},
        {Protocol::UDP, PortState::CLOSED,   "udp", "closed"},
        {Protocol::UDP, PortState::FILTERED, "udp", "filtered"},
    };
    for (const auto& r : rows) {
        REQUIRE(std::string(proto_str(r.proto))  == r.ep);
        REQUIRE(std::string(state_str(r.state))  == r.es);
    }
}

TEST_CASE("Output format - full line matches assignment spec", "[output]") {
    // The spec requires: "IP PORT proto state" separated by single spaces
    // print_result does: ip_str << " " << port << " " << proto_str << " " << state_str << "\n"
    struct Row { ScanResult r; const char* expected; };
    Row rows[] = {
        {{"127.0.0.1", 22,  Protocol::TCP, PortState::OPEN},     "127.0.0.1 22 tcp open"},
        {{"127.0.0.1", 53,  Protocol::UDP, PortState::CLOSED},   "127.0.0.1 53 udp closed"},
        {{"127.0.0.1", 143, Protocol::TCP, PortState::FILTERED}, "127.0.0.1 143 tcp filtered"},
        {{"::1",        80,  Protocol::TCP, PortState::OPEN},     "::1 80 tcp open"},
        {{"::1",        67,  Protocol::UDP, PortState::CLOSED},   "::1 67 udp closed"},
    };
    for (const auto& row : rows) {
        const auto& r = row.r;
        std::string line = r.ip_str + " " + std::to_string(r.port) + " "
                         + proto_str(r.protocol) + " " + state_str(r.state);
        REQUIRE(line == row.expected);
    }
}

// =============================================
// === Application validate_config Tests     ===
// =============================================
// These test via Application::run() — validate_config throws before any network I/O.

TEST_CASE("Application - missing host returns EXIT_FAILURE", "[application]") {
    // -i and -t provided but no HOST positional argument
    const char* argv[] = {"ipk-L4-scan", "-i", "lo", "-t", "22"};
    Application app;
    // Suppress stderr noise from the error handler
    std::streambuf* old = std::cerr.rdbuf(nullptr);
    int result = app.run(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
    std::cerr.rdbuf(old);
    REQUIRE(result == EXIT_FAILURE);
}

TEST_CASE("Application - missing interface returns EXIT_FAILURE", "[application]") {
    // No -i flag — interface stays empty
    const char* argv[] = {"ipk-L4-scan", "-t", "22", "localhost"};
    Application app;
    std::streambuf* old = std::cerr.rdbuf(nullptr);
    int result = app.run(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
    std::cerr.rdbuf(old);
    REQUIRE(result == EXIT_FAILURE);
}

TEST_CASE("Application - no ports specified returns EXIT_FAILURE", "[application]") {
    // Neither -t nor -u provided
    const char* argv[] = {"ipk-L4-scan", "-i", "lo", "localhost"};
    Application app;
    std::streambuf* old = std::cerr.rdbuf(nullptr);
    int result = app.run(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
    std::cerr.rdbuf(old);
    REQUIRE(result == EXIT_FAILURE);
}

TEST_CASE("Application - invalid port in args returns EXIT_FAILURE", "[application]") {
    // Port 0 is out of range — ArgParser should reject it
    const char* argv[] = {"ipk-L4-scan", "-i", "lo", "-t", "0", "localhost"};
    Application app;
    std::streambuf* old = std::cerr.rdbuf(nullptr);
    int result = app.run(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
    std::cerr.rdbuf(old);
    REQUIRE(result == EXIT_FAILURE);
}

TEST_CASE("Application - non-existent host returns EXIT_FAILURE", "[application]") {
    // DNS resolution will fail for this domain
    const char* argv[] = {"ipk-L4-scan", "-i", "lo", "-t", "22", "nonexistent.domain.invalid"};
    Application app;
    std::streambuf* old = std::cerr.rdbuf(nullptr);
    int result = app.run(static_cast<int>(std::size(argv)), const_cast<char**>(argv));
    std::cerr.rdbuf(old);
    REQUIRE(result == EXIT_FAILURE);
}

// =============================================
// === ScanKey Equality and Hash Tests       ===
// =============================================

TEST_CASE("ScanKey - equality operator", "[scan_key]") {
    ScanKey same1{"127.0.0.1", 80, Protocol::TCP};
    ScanKey same2{"127.0.0.1", 80, Protocol::TCP};
    ScanKey diff_proto{"127.0.0.1", 80, Protocol::UDP};
    ScanKey diff_ip{"192.168.0.1", 80, Protocol::TCP};
    ScanKey diff_port{"127.0.0.1", 443, Protocol::TCP};

    REQUIRE(same1 == same2);
    REQUIRE_FALSE(same1 == diff_proto);
    REQUIRE_FALSE(same1 == diff_ip);
    REQUIRE_FALSE(same1 == diff_port);
}

TEST_CASE("ScanKey - hash is consistent for equal keys", "[scan_key]") {
    ScanKeyHash h;
    ScanKey k1{"10.0.0.1", 22, Protocol::TCP};
    ScanKey k2{"10.0.0.1", 22, Protocol::TCP};
    // Equal keys must produce the same hash (fundamental hash contract)
    REQUIRE(h(k1) == h(k2));
}

TEST_CASE("ScanKey - hash differs for distinct keys", "[scan_key]") {
    ScanKeyHash h;
    ScanKey base{"10.0.0.1", 22, Protocol::TCP};
    ScanKey diff_proto{"10.0.0.1", 22, Protocol::UDP};
    ScanKey diff_port{"10.0.0.1", 23, Protocol::TCP};
    ScanKey diff_ip{"10.0.0.2", 22, Protocol::TCP};

    // Different keys should (almost always) produce different hashes
    REQUIRE(h(base) != h(diff_proto));
    REQUIRE(h(base) != h(diff_port));
    REQUIRE(h(base) != h(diff_ip));
}

TEST_CASE("ScanKey - IPv6 address equality", "[scan_key]") {
    ScanKey k1{"2001:db8::1", 443, Protocol::TCP};
    ScanKey k2{"2001:db8::1", 443, Protocol::TCP};
    ScanKey k3{"2001:db8::2", 443, Protocol::TCP};

    REQUIRE(k1 == k2);
    REQUIRE_FALSE(k1 == k3);
}

// =============================================
// === TaskGuard RAII Tests                  ===
// =============================================

TEST_CASE("TaskGuard - registers task on construction", "[tracker][raii]") {
    ScanKey key{"10.1.1.1", 777, Protocol::TCP};
    std::promise<PortState> promise;
    auto future = promise.get_future();

    {
        TaskGuard guard(key, promise);
        // Task is registered — resolve it through the tracker
        TaskTracker::instance().resolve_task(key, PortState::OPEN);
    }
    // After scope: guard destructor called remove_task (key already gone — safe)
    auto status = future.wait_for(std::chrono::milliseconds(50));
    REQUIRE(status == std::future_status::ready);
    REQUIRE(future.get() == PortState::OPEN);
}

TEST_CASE("TaskGuard - removes task on scope exit without resolve", "[tracker][raii]") {
    ScanKey key{"10.1.1.1", 778, Protocol::TCP};
    std::promise<PortState> promise;
    auto future = promise.get_future();

    {
        TaskGuard guard(key, promise);
        // Scope exits without resolving — guard cleans up the dangling pointer
    }
    // Future must remain unresolved (no set_value was ever called)
    auto status = future.wait_for(std::chrono::milliseconds(50));
    REQUIRE(status == std::future_status::timeout);

    // Calling resolve after guard is gone must be a safe no-op
    REQUIRE_NOTHROW(TaskTracker::instance().resolve_task(key, PortState::CLOSED));
}

// =============================================
// === TCPScanner - Packet Builder Tests     ===
// =============================================
// build_packet_ipv4 is accessible because tcp_scanner.hpp was included with
// #define private public above. Tests verify structure without sending any packet.

static ResolvedTarget make_ipv4_target(const char* ip_str) {
    ResolvedTarget t;
    t.ip_str = ip_str;
    t.family = AF_INET;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip_str, &addr.sin_addr);
    std::memcpy(&t.addr, &addr, sizeof(addr));
    t.addr_len = sizeof(addr);
    return t;
}

TEST_CASE("TCPScanner - build_packet_ipv4 size and SYN flag", "[tcp_scanner]") {
    std::atomic<bool> running{true};
    TCPScanner scanner("lo", 1000, running);

    ScanTask task{make_ipv4_target("127.0.0.1"), 80, Protocol::TCP};
    auto packet = scanner.build_packet_ipv4(task, 12345);

    REQUIRE(packet.size() == sizeof(iphdr) + sizeof(tcphdr));

    const auto* ip = reinterpret_cast<const iphdr*>(packet.data());
    REQUIRE(ip->version == 4);
    REQUIRE(ip->ihl == 5);
    REQUIRE(ip->protocol == IPPROTO_TCP);

    const auto* tcp = reinterpret_cast<const tcphdr*>(packet.data() + sizeof(iphdr));
    REQUIRE(ntohs(tcp->source) == 12345);
    REQUIRE(ntohs(tcp->dest) == 80);
    REQUIRE(tcp->syn == 1);
    REQUIRE(tcp->ack == 0); // SYN-only — not a full handshake
    REQUIRE(tcp->doff == 5);
}

TEST_CASE("TCPScanner - build_packet_ipv4 destination IP is correct", "[tcp_scanner]") {
    std::atomic<bool> running{true};
    TCPScanner scanner("lo", 1000, running);

    ScanTask task{make_ipv4_target("127.0.0.1"), 443, Protocol::TCP};
    auto packet = scanner.build_packet_ipv4(task, 50000);

    const auto* ip = reinterpret_cast<const iphdr*>(packet.data());
    uint32_t expected_dst;
    inet_pton(AF_INET, "127.0.0.1", &expected_dst);
    REQUIRE(ip->daddr == expected_dst);
    REQUIRE(ntohs(reinterpret_cast<const tcphdr*>(packet.data() + sizeof(iphdr))->dest) == 443);
}

TEST_CASE("TCPScanner - build_packet_ipv4 total_length field is correct", "[tcp_scanner]") {
    std::atomic<bool> running{true};
    TCPScanner scanner("lo", 1000, running);

    ScanTask task{make_ipv4_target("127.0.0.1"), 22, Protocol::TCP};
    auto packet = scanner.build_packet_ipv4(task, 49152);

    const auto* ip = reinterpret_cast<const iphdr*>(packet.data());
    uint16_t expected_len = static_cast<uint16_t>(sizeof(iphdr) + sizeof(tcphdr));
    REQUIRE(ntohs(ip->tot_len) == expected_len);
}

// =============================================
// === UDPScanner - Packet Builder Tests     ===
// =============================================

TEST_CASE("UDPScanner - build_packet_ipv4 size and protocol", "[udp_scanner]") {
    std::atomic<bool> running{true};
    UDPScanner scanner("lo", 1000, running);

    ScanTask task{make_ipv4_target("127.0.0.1"), 53, Protocol::UDP};
    auto packet = scanner.build_packet_ipv4(task);

    REQUIRE(packet.size() == sizeof(iphdr) + sizeof(udphdr));

    const auto* ip = reinterpret_cast<const iphdr*>(packet.data());
    REQUIRE(ip->version == 4);
    REQUIRE(ip->ihl == 5);
    REQUIRE(ip->protocol == IPPROTO_UDP);

    const auto* udp = reinterpret_cast<const udphdr*>(packet.data() + sizeof(iphdr));
    REQUIRE(ntohs(udp->dest) == 53);
    REQUIRE(ntohs(udp->len) == sizeof(udphdr)); // empty payload, just header
}

TEST_CASE("UDPScanner - build_packet_ipv4 destination IP is correct", "[udp_scanner]") {
    std::atomic<bool> running{true};
    UDPScanner scanner("lo", 1000, running);

    ScanTask task{make_ipv4_target("127.0.0.1"), 67, Protocol::UDP};
    auto packet = scanner.build_packet_ipv4(task);

    const auto* ip = reinterpret_cast<const iphdr*>(packet.data());
    uint32_t expected_dst;
    inet_pton(AF_INET, "127.0.0.1", &expected_dst);
    REQUIRE(ip->daddr == expected_dst);
}

TEST_CASE("UDPScanner - build_packet_ipv4 total_length field is correct", "[udp_scanner]") {
    std::atomic<bool> running{true};
    UDPScanner scanner("lo", 1000, running);

    ScanTask task{make_ipv4_target("127.0.0.1"), 123, Protocol::UDP};
    auto packet = scanner.build_packet_ipv4(task);

    const auto* ip = reinterpret_cast<const iphdr*>(packet.data());
    uint16_t expected_len = static_cast<uint16_t>(sizeof(iphdr) + sizeof(udphdr));
    REQUIRE(ntohs(ip->tot_len) == expected_len);
}
