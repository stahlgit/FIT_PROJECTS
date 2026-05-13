/**
 * @file test_session.cpp
 * @brief Unit tests for SenderSession and ReceiverSession state machines.
 *
 * Tests drive on_packet_received_ / next_packet_to_send_ directly via thin
 * harness subclasses, bypassing the run() event loop entirely. No actual
 * network I/O occurs except for ReceiverSession's ::connect() call in the
 * LISTENING → SYN_RCVD transition, which needs a real UDP socket + src address.
 *
 * Sequence numbers are segment-based (0, 1, 2, ...) as used by SendWindow /
 * ReceiveBuffer — not byte offsets.
 */

#include "catch2_include/catch_amalgamated.hpp"

#include "connconfig.hpp"
#include "packet.hpp"
#include "receiver_session.hpp"
#include "rdt_constants.hpp"
#include "sender_session.hpp"

#include <arpa/inet.h>
#include <atomic>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

using rdt::domain::Packet;
using rdt::domain::ReceiverSession;
using rdt::domain::SenderSession;
using SS = rdt::SessionState;

// ---------------------------------------------------------------------------
// Harness wrappers — expose protected members for white-box testing
// ---------------------------------------------------------------------------

class SenderHarness : public SenderSession
{
public:
    using SenderSession::SenderSession;

    void feed(const Packet &pkt)
    {
        auto raw = pkt.serialize();
        sockaddr_storage src{};
        on_packet_received_(raw.data(), raw.size(), src);
    }

    std::optional<std::vector<uint8_t>> pump() { return next_packet_to_send_(); }

    bool complete() const noexcept { return is_transfer_complete_(); }
    SS st() const noexcept { return state_; }
    uint32_t isn() const noexcept { return isn_local_; }
    uint32_t cid() const noexcept { return config_.conn_id; }
};

class ReceiverHarness : public ReceiverSession
{
public:
    using ReceiverSession::ReceiverSession;

    void feed(const Packet &pkt, const sockaddr_storage &src = {})
    {
        auto raw = pkt.serialize();
        on_packet_received_(raw.data(), raw.size(), src);
    }

    std::optional<std::vector<uint8_t>> pump() { return next_packet_to_send_(); }

    bool complete() const noexcept { return is_transfer_complete_(); }
    SS st() const noexcept { return state_; }
    uint32_t isn() const noexcept { return isn_local_; }
};

// ---------------------------------------------------------------------------
// Test utilities
// ---------------------------------------------------------------------------

// RAII UDP socket bound to 127.0.0.1:0 — provides a valid connectable address.
struct BoundSock
{
    int fd;
    sockaddr_storage addr{};

    BoundSock()
    {
        fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        REQUIRE(fd >= 0);
        sockaddr_in a{};
        a.sin_family = AF_INET;
        a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        a.sin_port = 0;
        REQUIRE(::bind(fd, reinterpret_cast<sockaddr *>(&a), sizeof(a)) == 0);
        socklen_t len = sizeof(addr);
        REQUIRE(::getsockname(fd, reinterpret_cast<sockaddr *>(&addr), &len) == 0);
    }
    ~BoundSock() { ::close(fd); }
    BoundSock(const BoundSock &) = delete;
    BoundSock &operator=(const BoundSock &) = delete;
};

static ConnConfig make_config(uint32_t conn_id = 0xABCD1234, uint32_t win = 4)
{
    ConnConfig cfg;
    cfg.mode = rdt::AppMode::CLIENT;
    cfg.conn_id = conn_id;
    cfg.timeout_s = 30;
    cfg.window_size = win;
    return cfg;
}

// Decode the packet returned by pump(), failing the test if absent or corrupt.
static Packet decode(const std::optional<std::vector<uint8_t>> &opt)
{
    REQUIRE(opt.has_value());
    auto maybe = Packet::deserialize(opt->data(), opt->size());
    REQUIRE(maybe.has_value());
    return *maybe;
}

// Drive a SenderHarness through the 3-way handshake up to TRANSFERRING.
// server_isn is an arbitrary seq number to use for the simulated SYN-ACK.
static void do_sender_handshake(SenderHarness &s, uint32_t server_isn = 42)
{
    s.pump(); // IDLE → SYN_SENT
    s.feed(Packet::make_syn_ack(s.cid(), server_isn, s.isn()));
    s.pump(); // ESTABLISHED → TRANSFERRING (delivers ACK)
}

// Drive a ReceiverHarness through the 3-way handshake up to ESTABLISHED.
static void do_receiver_handshake(ReceiverHarness &r, uint32_t client_conn_id,
                                   uint32_t client_isn, const sockaddr_storage &client_addr)
{
    r.feed(Packet::make_syn(client_conn_id, client_isn), client_addr);
    r.pump(); // consume SYN-ACK
    r.feed(Packet::make_ack(client_conn_id, r.isn()));
    // state == ESTABLISHED
}

// ===========================================================================
// SENDER SESSION
// ===========================================================================

TEST_CASE("SenderSession: starts in IDLE", "[session][sender]")
{
    BoundSock sock;
    std::atomic<bool> stop{false};
    std::istringstream in("x");
    SenderHarness s(make_config(), sock.fd, stop, in);

    REQUIRE(s.st() == SS::IDLE);
}

TEST_CASE("SenderSession: IDLE → SYN_SENT, pump returns SYN", "[session][sender]")
{
    BoundSock sock;
    std::atomic<bool> stop{false};
    std::istringstream in("hello");
    SenderHarness s(make_config(0x1001), sock.fd, stop, in);

    Packet syn = decode(s.pump());

    REQUIRE(s.st() == SS::SYN_SENT);
    REQUIRE(syn.is_syn());
    REQUIRE_FALSE(syn.is_ack());
    REQUIRE(syn.seq_num == s.isn());
    REQUIRE(syn.conn_id == 0x1001u);
}

TEST_CASE("SenderSession: SYN-ACK with correct ack_num → ESTABLISHED", "[session][sender]")
{
    BoundSock sock;
    std::atomic<bool> stop{false};
    std::istringstream in("data");
    SenderHarness s(make_config(0x1002), sock.fd, stop, in);

    s.pump(); // SYN_SENT
    s.feed(Packet::make_syn_ack(0x1002, 9999, s.isn()));

    REQUIRE(s.st() == SS::ESTABLISHED);
}

TEST_CASE("SenderSession: SYN-ACK with wrong ack_num is dropped", "[session][sender]")
{
    BoundSock sock;
    std::atomic<bool> stop{false};
    std::istringstream in("x");
    SenderHarness s(make_config(0x1003), sock.fd, stop, in);

    s.pump();
    s.feed(Packet::make_syn_ack(0x1003, 1, s.isn() + 1)); // wrong ack

    REQUIRE(s.st() == SS::SYN_SENT);
}

TEST_CASE("SenderSession: non-SYN-ACK in SYN_SENT is dropped", "[session][sender]")
{
    BoundSock sock;
    std::atomic<bool> stop{false};
    std::istringstream in("x");
    SenderHarness s(make_config(0x1004), sock.fd, stop, in);

    s.pump();
    s.feed(Packet::make_ack(0x1004, s.isn())); // plain ACK, missing SYN flag

    REQUIRE(s.st() == SS::SYN_SENT);
}

TEST_CASE("SenderSession: ESTABLISHED pump delivers ACK and transitions to TRANSFERRING", "[session][sender]")
{
    BoundSock sock;
    std::atomic<bool> stop{false};
    std::istringstream in("payload");
    SenderHarness s(make_config(0x1005), sock.fd, stop, in);

    s.pump();
    s.feed(Packet::make_syn_ack(0x1005, 42, s.isn()));
    REQUIRE(s.st() == SS::ESTABLISHED);

    Packet ack = decode(s.pump());
    REQUIRE(ack.is_ack());
    REQUIRE(s.st() == SS::TRANSFERRING);
}

TEST_CASE("SenderSession: TRANSFERRING pumps DATA segments from input", "[session][sender]")
{
    BoundSock sock;
    std::atomic<bool> stop{false};
    std::istringstream in("ABCDEF");
    SenderHarness s(make_config(0x1006, 8), sock.fd, stop, in);

    do_sender_handshake(s);
    REQUIRE(s.st() == SS::TRANSFERRING);

    Packet data = decode(s.pump());
    REQUIRE(data.payload_len > 0);
    REQUIRE_FALSE(data.is_syn());
    REQUIRE_FALSE(data.is_ack());
    REQUIRE_FALSE(data.is_fin());
}

TEST_CASE("SenderSession: window full returns nullopt", "[session][sender]")
{
    BoundSock sock;
    std::atomic<bool> stop{false};
    std::string big(3 * rdt::TYPICAL_PAYLOAD_SIZE, 'Z');
    std::istringstream in(big);
    SenderHarness s(make_config(0x1007, 1), sock.fd, stop, in); // window_size=1

    do_sender_handshake(s);

    REQUIRE(s.pump().has_value()); // loads seg 0, window now full
    REQUIRE_FALSE(s.pump().has_value()); // window full, retransmit timer not elapsed
}

TEST_CASE("SenderSession: ACK opens window for next segment", "[session][sender]")
{
    BoundSock sock;
    std::atomic<bool> stop{false};
    std::string big(3 * rdt::TYPICAL_PAYLOAD_SIZE, 'A');
    std::istringstream in(big);
    SenderHarness s(make_config(0x1008, 1), sock.fd, stop, in);

    do_sender_handshake(s);

    Packet seg0 = decode(s.pump()); // seq=0, window full
    REQUIRE_FALSE(s.pump().has_value());

    // ACK: ack_num = seg0.seq_num + 1 (segment-based ACK)
    s.feed(Packet::make_ack(0x1008, seg0.seq_num + 1));

    REQUIRE(s.pump().has_value()); // window opened → next segment loads
}

TEST_CASE("SenderSession: empty input → FIN sent after handshake", "[session][sender]")
{
    BoundSock sock;
    std::atomic<bool> stop{false};
    std::istringstream in(""); // empty
    SenderHarness s(make_config(0x1009, 4), sock.fd, stop, in);

    do_sender_handshake(s);

    // First pump in TRANSFERRING: tries to load, sets input_exhausted_, returns nullopt
    REQUIRE_FALSE(s.pump().has_value());
    // Second pump: window empty + exhausted → FIN
    Packet fin = decode(s.pump());

    REQUIRE(fin.is_fin());
    REQUIRE_FALSE(fin.is_ack());
    REQUIRE(s.st() == SS::FIN_WAIT);
}

TEST_CASE("SenderSession: FIN-ACK → TIME_WAIT, complete() false before linger expires", "[session][sender]")
{
    BoundSock sock;
    std::atomic<bool> stop{false};
    std::istringstream in("");
    SenderHarness s(make_config(0x100A, 4), sock.fd, stop, in);

    do_sender_handshake(s);
    s.pump(); // nullopt (empty input, sets exhausted)
    s.pump(); // FIN → FIN_WAIT

    s.feed(Packet::make_fin_ack(0x100A, 2, 0));

    REQUIRE(s.st() == SS::TIME_WAIT);
    REQUIRE_FALSE(s.complete()); // linger period has not elapsed
}

// ===========================================================================
// RECEIVER SESSION
// ===========================================================================

TEST_CASE("ReceiverSession: starts in LISTENING", "[session][receiver]")
{
    BoundSock sock;
    std::atomic<bool> stop{false};
    std::ostringstream out;
    ReceiverHarness r(make_config(), sock.fd, stop, out);

    REQUIRE(r.st() == SS::LISTENING);
}

TEST_CASE("ReceiverSession: non-SYN in LISTENING is dropped", "[session][receiver]")
{
    BoundSock sock;
    std::atomic<bool> stop{false};
    std::ostringstream out;
    ReceiverHarness r(make_config(0x2001), sock.fd, stop, out);

    r.feed(Packet::make_ack(0x2001, 0));
    REQUIRE(r.st() == SS::LISTENING);

    r.feed(Packet::make_fin(0x2001, 0));
    REQUIRE(r.st() == SS::LISTENING);
}

TEST_CASE("ReceiverSession: SYN+ACK in LISTENING is dropped (not a plain SYN)", "[session][receiver]")
{
    BoundSock sock;
    std::atomic<bool> stop{false};
    std::ostringstream out;
    ReceiverHarness r(make_config(0x2002), sock.fd, stop, out);

    r.feed(Packet::make_syn_ack(0x2002, 1, 0)); // has ACK flag → rejected
    REQUIRE(r.st() == SS::LISTENING);
}

TEST_CASE("ReceiverSession: SYN → SYN_RCVD, pump returns SYN-ACK", "[session][receiver]")
{
    BoundSock client;
    BoundSock server;
    std::atomic<bool> stop{false};
    std::ostringstream out;
    ReceiverHarness r(make_config(0, 4), server.fd, stop, out);

    r.feed(Packet::make_syn(0xDEAD, 100), client.addr);

    REQUIRE(r.st() == SS::SYN_RCVD);

    Packet synack = decode(r.pump());
    REQUIRE(synack.is_syn());
    REQUIRE(synack.is_ack());
    REQUIRE(synack.ack_num == 100u); // acks client's ISN
    REQUIRE(synack.seq_num == r.isn());
}

TEST_CASE("ReceiverSession: SYN_RCVD → ESTABLISHED via correct ACK", "[session][receiver]")
{
    BoundSock client;
    BoundSock server;
    std::atomic<bool> stop{false};
    std::ostringstream out;
    ReceiverHarness r(make_config(), server.fd, stop, out);

    r.feed(Packet::make_syn(0xBEEF, 0), client.addr);
    r.pump(); // consume SYN-ACK

    r.feed(Packet::make_ack(0xBEEF, r.isn())); // ack_num = server's ISN
    REQUIRE(r.st() == SS::ESTABLISHED);
}

TEST_CASE("ReceiverSession: wrong ack_num in SYN_RCVD is dropped", "[session][receiver]")
{
    BoundSock client;
    BoundSock server;
    std::atomic<bool> stop{false};
    std::ostringstream out;
    ReceiverHarness r(make_config(), server.fd, stop, out);

    r.feed(Packet::make_syn(0xBEEF, 0), client.addr);
    r.pump();

    r.feed(Packet::make_ack(0xBEEF, r.isn() + 1)); // wrong ack
    REQUIRE(r.st() == SS::SYN_RCVD);
}

TEST_CASE("ReceiverSession: first data → RECEIVING, payload written to output", "[session][receiver]")
{
    BoundSock client;
    BoundSock server;
    std::atomic<bool> stop{false};
    std::ostringstream out;
    ReceiverHarness r(make_config(), server.fd, stop, out);

    do_receiver_handshake(r, 0xCAFE, 0, client.addr);
    REQUIRE(r.st() == SS::ESTABLISHED);

    r.feed(Packet::make_data(0xCAFE, 0, {'H', 'e', 'l', 'l', 'o'}));

    REQUIRE(r.st() == SS::RECEIVING);
    REQUIRE(out.str() == "Hello");
}

TEST_CASE("ReceiverSession: in-order segments are delivered in order", "[session][receiver]")
{
    BoundSock client;
    BoundSock server;
    std::atomic<bool> stop{false};
    std::ostringstream out;
    ReceiverHarness r(make_config(), server.fd, stop, out);

    do_receiver_handshake(r, 0xCAFE, 0, client.addr);

    r.feed(Packet::make_data(0xCAFE, 0, {'A', 'B'})); // seq=0
    r.feed(Packet::make_data(0xCAFE, 1, {'C', 'D'})); // seq=1

    REQUIRE(out.str() == "ABCD");
}

TEST_CASE("ReceiverSession: out-of-order segment buffered, delivered after gap filled", "[session][receiver]")
{
    BoundSock client;
    BoundSock server;
    std::atomic<bool> stop{false};
    std::ostringstream out;
    ReceiverHarness r(make_config(), server.fd, stop, out);

    do_receiver_handshake(r, 0xCAFE, 0, client.addr);

    r.feed(Packet::make_data(0xCAFE, 1, {'B'})); // out-of-order, gap at 0
    REQUIRE(out.str().empty()); // not delivered yet

    r.feed(Packet::make_data(0xCAFE, 0, {'A'})); // fills the gap
    REQUIRE(out.str() == "AB"); // both delivered in order
}

TEST_CASE("ReceiverSession: duplicate segment triggers re-ACK, not double-output", "[session][receiver]")
{
    BoundSock client;
    BoundSock server;
    std::atomic<bool> stop{false};
    std::ostringstream out;
    ReceiverHarness r(make_config(), server.fd, stop, out);

    do_receiver_handshake(r, 0x1111, 0, client.addr);

    r.feed(Packet::make_data(0x1111, 0, {'X'}));
    r.pump(); // consume first ACK

    r.feed(Packet::make_data(0x1111, 0, {'X'})); // duplicate

    Packet reack = decode(r.pump());
    REQUIRE(reack.is_ack());
    REQUIRE(out.str() == "X"); // not written twice
}

TEST_CASE("ReceiverSession: FIN → FIN_RCVD, pump returns FIN-ACK", "[session][receiver]")
{
    BoundSock client;
    BoundSock server;
    std::atomic<bool> stop{false};
    std::ostringstream out;
    ReceiverHarness r(make_config(), server.fd, stop, out);

    do_receiver_handshake(r, 0x2222, 0, client.addr);

    r.feed(Packet::make_fin(0x2222, 0));
    REQUIRE(r.st() == SS::FIN_RCVD);

    Packet finack = decode(r.pump());
    REQUIRE(finack.is_fin());
    REQUIRE(finack.is_ack());
}

TEST_CASE("ReceiverSession: final ACK → CLOSED, is_transfer_complete true", "[session][receiver]")
{
    BoundSock client;
    BoundSock server;
    std::atomic<bool> stop{false};
    std::ostringstream out;
    ReceiverHarness r(make_config(), server.fd, stop, out);

    do_receiver_handshake(r, 0x3333, 0, client.addr);
    r.feed(Packet::make_fin(0x3333, 0));
    r.pump(); // consume FIN-ACK

    r.feed(Packet::make_ack(0x3333, 0)); // final ACK

    REQUIRE(r.st() == SS::CLOSED);
    REQUIRE(r.complete());
}

TEST_CASE("ReceiverSession: is_transfer_complete false in ESTABLISHED", "[session][receiver]")
{
    BoundSock client;
    BoundSock server;
    std::atomic<bool> stop{false};
    std::ostringstream out;
    ReceiverHarness r(make_config(), server.fd, stop, out);

    do_receiver_handshake(r, 0x4444, 0, client.addr);
    REQUIRE_FALSE(r.complete());
}
