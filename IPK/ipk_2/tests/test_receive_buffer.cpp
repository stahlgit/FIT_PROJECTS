/**
 * @file test_receive_buffer.cpp
 * @brief Unit tests for rdt::domain::ReceiveBuffer — insert results,
 *        drain ordering, SACK entry reporting, and window boundary checks.
 */

#include "catch2_include/catch_amalgamated.hpp"
#include "receive_buffer.hpp"
#include "rdt_constants.hpp"

using rdt::domain::ReceiveBuffer;
using IR = ReceiveBuffer::InsertResult;

static std::vector<uint8_t> bytes(std::initializer_list<uint8_t> il)
{
    return std::vector<uint8_t>(il);
}

// ---------------------------------------------------------------------------
// INITIAL STATE
// ---------------------------------------------------------------------------

TEST_CASE("ReceiveBuffer: initial state", "[recvbuf][state]")
{
    ReceiveBuffer rb(8, 0);
    REQUIRE(rb.is_empty());
    REQUIRE(rb.ack_num() == 0u);
    REQUIRE(rb.sack_entries().empty());
    REQUIRE(rb.drain().empty());
}

TEST_CASE("ReceiveBuffer: initial ISN is reflected in ack_num", "[recvbuf][state]")
{
    ReceiveBuffer rb(4, 42);
    REQUIRE(rb.ack_num() == 42u);
}

// ---------------------------------------------------------------------------
// INSERT — RESULT CLASSIFICATION
// ---------------------------------------------------------------------------

TEST_CASE("ReceiveBuffer: insert returns NEW for in-window segment", "[recvbuf][insert]")
{
    ReceiveBuffer rb(4, 0);
    REQUIRE(rb.insert(0, bytes({1})) == IR::NEW);
    REQUIRE_FALSE(rb.is_empty());
}

TEST_CASE("ReceiveBuffer: insert returns NEW for out-of-order in-window segment", "[recvbuf][insert]")
{
    ReceiveBuffer rb(8, 0);
    REQUIRE(rb.insert(3, bytes({1, 2})) == IR::NEW); // gap at 0,1,2 is OK
}

TEST_CASE("ReceiveBuffer: insert returns DUPLICATE for already-delivered seq", "[recvbuf][insert]")
{
    ReceiveBuffer rb(4, 0);
    rb.insert(0, bytes({1}));
    rb.drain(); // delivers seq 0, next_expected_ becomes 1

    REQUIRE(rb.insert(0, bytes({1})) == IR::DUPLICATE);
}

TEST_CASE("ReceiveBuffer: insert returns DUPLICATE for already-buffered seq", "[recvbuf][insert]")
{
    ReceiveBuffer rb(4, 0);
    REQUIRE(rb.insert(1, bytes({1})) == IR::NEW);
    REQUIRE(rb.insert(1, bytes({1})) == IR::DUPLICATE); // same seq again
}

TEST_CASE("ReceiveBuffer: insert returns INVALID for seq at window upper boundary", "[recvbuf][insert]")
{
    ReceiveBuffer rb(4, 0); // window [0, 4)
    REQUIRE(rb.insert(3, bytes({1})) == IR::NEW);      // last valid seq
    REQUIRE(rb.insert(4, bytes({1})) == IR::INVALID);  // exactly at upper bound
    REQUIRE(rb.insert(5, bytes({1})) == IR::INVALID);  // beyond
}

// ---------------------------------------------------------------------------
// DRAIN — ORDERING AND PROGRESS
// ---------------------------------------------------------------------------

TEST_CASE("ReceiveBuffer: drain returns empty when nothing buffered", "[recvbuf][drain]")
{
    ReceiveBuffer rb(4, 0);
    REQUIRE(rb.drain().empty());
}

TEST_CASE("ReceiveBuffer: drain delivers single in-order segment", "[recvbuf][drain]")
{
    ReceiveBuffer rb(4, 0);
    rb.insert(0, bytes({0xAA}));

    auto delivered = rb.drain();
    REQUIRE(delivered.size() == 1);
    REQUIRE(delivered[0].seq == 0u);
    REQUIRE(delivered[0].data == bytes({0xAA}));
    REQUIRE(rb.ack_num() == 1u);
    REQUIRE(rb.is_empty());
}

TEST_CASE("ReceiveBuffer: drain delivers multiple contiguous in-order segments", "[recvbuf][drain]")
{
    ReceiveBuffer rb(8, 0);
    rb.insert(0, bytes({1}));
    rb.insert(1, bytes({2}));
    rb.insert(2, bytes({3}));

    auto delivered = rb.drain();
    REQUIRE(delivered.size() == 3);
    REQUIRE(delivered[0].seq == 0u);
    REQUIRE(delivered[1].seq == 1u);
    REQUIRE(delivered[2].seq == 2u);
    REQUIRE(rb.ack_num() == 3u);
    REQUIRE(rb.is_empty());
}

TEST_CASE("ReceiveBuffer: drain stops at gap", "[recvbuf][drain]")
{
    ReceiveBuffer rb(8, 0);
    rb.insert(0, bytes({1}));
    // seq 1 missing
    rb.insert(2, bytes({3}));

    auto delivered = rb.drain();
    REQUIRE(delivered.size() == 1);         // only seq 0
    REQUIRE(delivered[0].seq == 0u);
    REQUIRE(rb.ack_num() == 1u);
    REQUIRE_FALSE(rb.is_empty());           // seq 2 still buffered
}

TEST_CASE("ReceiveBuffer: drain flushes when gap is filled", "[recvbuf][drain]")
{
    ReceiveBuffer rb(8, 0);
    rb.insert(2, bytes({3}));
    rb.insert(0, bytes({1}));

    // First drain: delivers only seq 0 (seq 1 missing)
    auto d1 = rb.drain();
    REQUIRE(d1.size() == 1);
    REQUIRE(d1[0].seq == 0u);
    REQUIRE(rb.ack_num() == 1u);

    // Fill the gap
    rb.insert(1, bytes({2}));

    // Second drain: delivers seq 1 and seq 2
    auto d2 = rb.drain();
    REQUIRE(d2.size() == 2);
    REQUIRE(d2[0].seq == 1u);
    REQUIRE(d2[1].seq == 2u);
    REQUIRE(rb.ack_num() == 3u);
    REQUIRE(rb.is_empty());
}

TEST_CASE("ReceiveBuffer: drain data payloads are preserved in order", "[recvbuf][drain]")
{
    ReceiveBuffer rb(4, 10);
    rb.insert(10, bytes({0x10, 0x11}));
    rb.insert(11, bytes({0x20, 0x21}));

    auto delivered = rb.drain();
    REQUIRE(delivered[0].data == bytes({0x10, 0x11}));
    REQUIRE(delivered[1].data == bytes({0x20, 0x21}));
}

TEST_CASE("ReceiveBuffer: ack_num only advances when contiguous run drains", "[recvbuf][drain]")
{
    ReceiveBuffer rb(8, 5);
    REQUIRE(rb.ack_num() == 5u);

    rb.insert(7, bytes({1})); // out-of-order
    REQUIRE(rb.ack_num() == 5u); // no drain, no advance

    rb.drain(); // gap at 5,6 — nothing drainable
    REQUIRE(rb.ack_num() == 5u);

    rb.insert(5, bytes({1}));
    rb.drain();
    REQUIRE(rb.ack_num() == 6u); // seq 5 delivered; seq 6 missing; seq 7 held
}

// ---------------------------------------------------------------------------
// SACK ENTRIES
// ---------------------------------------------------------------------------

TEST_CASE("ReceiveBuffer: sack_entries reports buffered out-of-order seqs", "[recvbuf][sack]")
{
    ReceiveBuffer rb(8, 0);
    rb.insert(2, bytes({1}));
    rb.insert(4, bytes({1}));

    auto sack = rb.sack_entries();
    REQUIRE(sack.size() == 2);
    REQUIRE(sack[0] == 2u);
    REQUIRE(sack[1] == 4u);
}

TEST_CASE("ReceiveBuffer: sack_entries empty after full drain", "[recvbuf][sack]")
{
    ReceiveBuffer rb(4, 0);
    rb.insert(0, bytes({1}));
    rb.insert(1, bytes({1}));
    rb.drain();
    REQUIRE(rb.sack_entries().empty());
}

TEST_CASE("ReceiveBuffer: sack_entries capped at MAX_SACK_ENTRIES", "[recvbuf][sack]")
{
    // Window must be large enough to hold MAX_SACK_ENTRIES + 2 out-of-order entries
    const uint32_t WIN = rdt::MAX_SACK_ENTRIES + 2;
    ReceiveBuffer rb(WIN, 0);

    // Insert all except seq 0 (create a gap so nothing drains)
    for (uint32_t i = 1; i < WIN; ++i)
        rb.insert(i, bytes({static_cast<uint8_t>(i)}));

    auto sack = rb.sack_entries();
    REQUIRE(sack.size() == rdt::MAX_SACK_ENTRIES);
}
