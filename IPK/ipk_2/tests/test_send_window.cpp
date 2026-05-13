/**
 * @file test_send_window.cpp
 * @brief Unit tests for rdt::domain::SendWindow — state, push, ACK processing,
 *        SACK marking, and retransmit candidate selection.
 */

#include "catch2_include/catch_amalgamated.hpp"
#include "send_window.hpp"
#include "rdt_constants.hpp"

#include <thread>
#include <chrono>

using rdt::domain::SendWindow;

static std::vector<uint8_t> make_data(std::size_t n)
{
    std::vector<uint8_t> v(n);
    for (std::size_t i = 0; i < n; ++i)
        v[i] = static_cast<uint8_t>(i & 0xFF);
    return v;
}

// ---------------------------------------------------------------------------
// INITIAL STATE
// ---------------------------------------------------------------------------

TEST_CASE("SendWindow: initial state", "[sendwindow][state]")
{
    SendWindow w(4, 10);
    REQUIRE(w.is_empty());
    REQUIRE_FALSE(w.is_full());
    REQUIRE(w.base() == 10u);
    REQUIRE(w.next_seq() == 10u);
}

// ---------------------------------------------------------------------------
// IS_FULL / CAPACITY
// ---------------------------------------------------------------------------

TEST_CASE("SendWindow: is_full respects window_size", "[sendwindow][state]")
{
    const uint32_t SIZE = 3;
    SendWindow w(SIZE, 0);

    for (uint32_t i = 0; i < SIZE; ++i)
    {
        REQUIRE_FALSE(w.is_full());
        w.push(w.next_seq(), make_data(4));
    }
    REQUIRE(w.is_full());
}

// ---------------------------------------------------------------------------
// PUSH
// ---------------------------------------------------------------------------

TEST_CASE("SendWindow: push advances next_seq", "[sendwindow][push]")
{
    SendWindow w(4, 0);
    REQUIRE(w.next_seq() == 0u);

    w.push(0, make_data(10));
    REQUIRE(w.next_seq() == 1u);
    REQUIRE_FALSE(w.is_empty());

    w.push(1, make_data(10));
    REQUIRE(w.next_seq() == 2u);
}

TEST_CASE("SendWindow: push sequence stays consistent with base", "[sendwindow][push]")
{
    SendWindow w(8, 100);
    w.push(100, make_data(5));
    w.push(101, make_data(5));
    w.push(102, make_data(5));

    REQUIRE(w.base() == 100u);     // base unchanged until ACK
    REQUIRE(w.next_seq() == 103u);
}

// ---------------------------------------------------------------------------
// ON_ACK
// ---------------------------------------------------------------------------

TEST_CASE("SendWindow: on_ack advances base and returns count", "[sendwindow][ack]")
{
    SendWindow w(8, 0);
    w.push(0, make_data(4));
    w.push(1, make_data(4));
    w.push(2, make_data(4));

    // Cumulative ACK for seq 0 and 1 (ack_num == next expected == 2)
    uint32_t acked = w.on_ack(2);
    REQUIRE(acked == 2u);
    REQUIRE(w.base() == 2u);
    REQUIRE_FALSE(w.is_empty()); // seq 2 still in flight
}

TEST_CASE("SendWindow: on_ack ACKs all in-flight segments", "[sendwindow][ack]")
{
    SendWindow w(4, 5);
    w.push(5, make_data(4));
    w.push(6, make_data(4));

    uint32_t acked = w.on_ack(7);
    REQUIRE(acked == 2u);
    REQUIRE(w.is_empty());
    REQUIRE(w.base() == 7u);
}

TEST_CASE("SendWindow: on_ack duplicate (ack <= base) returns 0", "[sendwindow][ack]")
{
    SendWindow w(4, 0);
    w.push(0, make_data(4));
    w.on_ack(1); // advances base to 1

    uint32_t acked = w.on_ack(1); // same ack again
    REQUIRE(acked == 0u);

    acked = w.on_ack(0); // ack below base
    REQUIRE(acked == 0u);
}

TEST_CASE("SendWindow: on_ack partial — only earlier segments cleared", "[sendwindow][ack]")
{
    SendWindow w(8, 10);
    w.push(10, make_data(4));
    w.push(11, make_data(4));
    w.push(12, make_data(4));

    uint32_t acked = w.on_ack(11); // clears only seq 10
    REQUIRE(acked == 1u);
    REQUIRE(w.base() == 11u);
    REQUIRE_FALSE(w.is_empty()); // 11 and 12 still in flight
}

// ---------------------------------------------------------------------------
// ON_SACK
// ---------------------------------------------------------------------------

TEST_CASE("SendWindow: on_sack marks known segment", "[sendwindow][sack]")
{
    SendWindow w(4, 0);
    w.push(0, make_data(4));
    w.push(1, make_data(4));

    REQUIRE(w.on_sack(1));  // mark seq 1 as received out-of-order
    REQUIRE_FALSE(w.on_sack(1)); // duplicate SACK → false
}

TEST_CASE("SendWindow: on_sack unknown seq returns false", "[sendwindow][sack]")
{
    SendWindow w(4, 0);
    w.push(0, make_data(4));

    REQUIRE_FALSE(w.on_sack(99)); // never pushed
}

// ---------------------------------------------------------------------------
// RETRANSMIT CANDIDATES
// ---------------------------------------------------------------------------

TEST_CASE("SendWindow: freshly pushed segment not yet a retransmit candidate", "[sendwindow][retransmit]")
{
    SendWindow w(4, 0);
    w.push(0, make_data(8));

    // RETRANSMIT_MS = 200 ms; just pushed so timer not expired
    auto candidates = w.retransmit_candidates();
    REQUIRE(candidates.empty());
}

TEST_CASE("SendWindow: SACK-marked segment is not a retransmit candidate", "[sendwindow][retransmit]")
{
    SendWindow w(4, 0);
    w.push(0, make_data(8));
    w.on_sack(0); // receiver already has it

    // Even if timer had expired, SACK-marked entries are skipped
    auto candidates = w.retransmit_candidates();
    REQUIRE(candidates.empty());
}

TEST_CASE("SendWindow: expired segment becomes retransmit candidate", "[sendwindow][retransmit]")
{
    // Use a window with a single segment, then sleep past RETRANSMIT_MS.
    // RETRANSMIT_MS = 200 ms — keep the sleep short but sufficient.
    SendWindow w(4, 0);
    w.push(0, make_data(4));

    std::this_thread::sleep_for(std::chrono::milliseconds(rdt::RETRANSMIT_MS + 50));

    auto candidates = w.retransmit_candidates();
    REQUIRE(candidates.size() == 1);
    REQUIRE(candidates[0].first == 0u);
    REQUIRE(candidates[0].second == make_data(4));
}

TEST_CASE("SendWindow: only non-SACK expired segments returned", "[sendwindow][retransmit]")
{
    SendWindow w(4, 0);
    w.push(0, make_data(4));
    w.push(1, make_data(4));
    w.on_sack(1); // seq 1 already received by peer

    std::this_thread::sleep_for(std::chrono::milliseconds(rdt::RETRANSMIT_MS + 50));

    auto candidates = w.retransmit_candidates();
    REQUIRE(candidates.size() == 1);
    REQUIRE(candidates[0].first == 0u); // only seq 0
}
