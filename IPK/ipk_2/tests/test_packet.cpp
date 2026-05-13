/**
 * @file test_packet.cpp
 * @brief Unit tests for rdt::domain::Packet — flags, SACK helpers, factory methods,
 *        serialisation/deserialisation round-trips, and checksum.
 */

#include "catch2_include/catch_amalgamated.hpp"
#include "packet.hpp"
#include "rdt_constants.hpp"

using rdt::domain::Packet;

// ---------------------------------------------------------------------------
// FLAG HELPERS
// ---------------------------------------------------------------------------

TEST_CASE("Packet: individual flag set/clear/query", "[packet][flags]")
{
    Packet p;

    SECTION("SYN")
    {
        REQUIRE_FALSE(p.is_syn());
        p.set_syn();
        REQUIRE(p.is_syn());
        p.clear_syn();
        REQUIRE_FALSE(p.is_syn());
    }

    SECTION("ACK")
    {
        REQUIRE_FALSE(p.is_ack());
        p.set_ack();
        REQUIRE(p.is_ack());
        p.clear_ack();
        REQUIRE_FALSE(p.is_ack());
    }

    SECTION("FIN")
    {
        REQUIRE_FALSE(p.is_fin());
        p.set_fin();
        REQUIRE(p.is_fin());
        p.clear_fin();
        REQUIRE_FALSE(p.is_fin());
    }

    SECTION("RST")
    {
        REQUIRE_FALSE(p.is_rst());
        p.set_rst();
        REQUIRE(p.is_rst());
        p.clear_rst();
        REQUIRE_FALSE(p.is_rst());
    }
}

TEST_CASE("Packet: SYN-ACK combination does not bleed flags", "[packet][flags]")
{
    Packet p;
    p.set_syn();
    p.set_ack();
    REQUIRE(p.is_syn());
    REQUIRE(p.is_ack());
    REQUIRE_FALSE(p.is_fin());
    REQUIRE_FALSE(p.is_rst());

    p.clear_syn();
    REQUIRE_FALSE(p.is_syn());
    REQUIRE(p.is_ack()); // ACK must survive clearing SYN
}

// ---------------------------------------------------------------------------
// SACK HELPERS
// ---------------------------------------------------------------------------

TEST_CASE("Packet: SACK add and query", "[packet][sack]")
{
    Packet p;

    REQUIRE_FALSE(p.has_sack(100));
    REQUIRE(p.add_sack(100));
    REQUIRE(p.has_sack(100));
    REQUIRE_FALSE(p.has_sack(101));
    REQUIRE(p.sack_count == 1);
}

TEST_CASE("Packet: SACK block fills to MAX_SACK_ENTRIES then refuses", "[packet][sack]")
{
    Packet p;
    for (uint8_t i = 0; i < rdt::MAX_SACK_ENTRIES; ++i)
        REQUIRE(p.add_sack(i));

    REQUIRE(p.sack_count == rdt::MAX_SACK_ENTRIES);
    REQUIRE_FALSE(p.add_sack(99)); // full
}

// ---------------------------------------------------------------------------
// FACTORY METHODS
// ---------------------------------------------------------------------------

TEST_CASE("Packet: make_syn factory", "[packet][factory]")
{
    auto p = Packet::make_syn(0xABCD, 42);
    REQUIRE(p.is_syn());
    REQUIRE_FALSE(p.is_ack());
    REQUIRE(p.conn_id == 0xABCDu);
    REQUIRE(p.seq_num == 42u);
}

TEST_CASE("Packet: make_syn_ack factory", "[packet][factory]")
{
    auto p = Packet::make_syn_ack(1, 100, 43);
    REQUIRE(p.is_syn());
    REQUIRE(p.is_ack());
    REQUIRE(p.conn_id == 1u);
    REQUIRE(p.seq_num == 100u);
    REQUIRE(p.ack_num == 43u);
}

TEST_CASE("Packet: make_fin factory", "[packet][factory]")
{
    auto p = Packet::make_fin(7, 200);
    REQUIRE(p.is_fin());
    REQUIRE_FALSE(p.is_syn());
    REQUIRE_FALSE(p.is_ack());
    REQUIRE(p.conn_id == 7u);
    REQUIRE(p.seq_num == 200u);
}

TEST_CASE("Packet: make_rst factory", "[packet][factory]")
{
    auto p = Packet::make_rst(3);
    REQUIRE(p.is_rst());
    REQUIRE_FALSE(p.is_syn());
    REQUIRE(p.conn_id == 3u);
}

// ---------------------------------------------------------------------------
// SERIALISATION / DESERIALISATION ROUND-TRIPS
// ---------------------------------------------------------------------------

TEST_CASE("Packet: minimal header round-trip (no payload, no SACK)", "[packet][serde]")
{
    Packet orig;
    orig.conn_id = 0xDEADBEEF;
    orig.seq_num = 1000;
    orig.ack_num = 999;
    orig.set_syn();
    orig.set_ack();

    auto buf = orig.serialize();
    REQUIRE(buf.size() == rdt::HEADER_SIZE);

    auto maybe = Packet::deserialize(buf.data(), buf.size());
    REQUIRE(maybe.has_value());

    auto &got = *maybe;
    REQUIRE(got.conn_id ==     orig.conn_id);
    REQUIRE(got.seq_num ==     orig.seq_num);
    REQUIRE(got.ack_num ==     orig.ack_num);
    REQUIRE(got.is_syn());
    REQUIRE(got.is_ack());
    REQUIRE_FALSE(got.is_fin());
    REQUIRE_FALSE(got.is_rst());
    REQUIRE(got.payload_len ==  0u);
    REQUIRE(got.sack_count ==   0u);
}

TEST_CASE("Packet: payload round-trip", "[packet][serde]")
{
    Packet orig;
    orig.conn_id = 1;
    orig.seq_num = 5;
    orig.payload = {0x01, 0x02, 0x03, 0xFF};
    orig.payload_len = static_cast<uint16_t>(orig.payload.size());

    auto buf = orig.serialize();
    REQUIRE(buf.size() == rdt::HEADER_SIZE + 4);

    auto maybe = Packet::deserialize(buf.data(), buf.size());
    REQUIRE(maybe.has_value());
    REQUIRE(maybe->payload == orig.payload);
    REQUIRE(maybe->payload_len == 4u);
}

TEST_CASE("Packet: SACK blocks round-trip", "[packet][serde]")
{
    Packet orig;
    orig.conn_id = 2;
    orig.add_sack(10);
    orig.add_sack(20);
    orig.add_sack(30);

    auto buf = orig.serialize();
    // header + 3 SACK entries × 4 bytes
    REQUIRE(buf.size() == rdt::HEADER_SIZE + 3 * sizeof(uint32_t));

    auto maybe = Packet::deserialize(buf.data(), buf.size());
    REQUIRE(maybe.has_value());
    REQUIRE(maybe->sack_count == 3u);
    REQUIRE(maybe->has_sack(10));
    REQUIRE(maybe->has_sack(20));
    REQUIRE(maybe->has_sack(30));
    REQUIRE_FALSE(maybe->has_sack(99));
}

TEST_CASE("Packet: payload + SACK combined round-trip", "[packet][serde]")
{
    Packet orig;
    orig.conn_id = 3;
    orig.seq_num = 77;
    orig.add_sack(5);
    orig.add_sack(6);
    orig.payload = {0xAA, 0xBB};
    orig.payload_len = 2;

    auto buf = orig.serialize();
    REQUIRE(buf.size() == rdt::HEADER_SIZE + 2 * sizeof(uint32_t) + 2);

    auto maybe = Packet::deserialize(buf.data(), buf.size());
    REQUIRE(maybe.has_value());
    REQUIRE(maybe->seq_num == 77u);
    REQUIRE(maybe->sack_count == 2u);
    REQUIRE(maybe->has_sack(5));
    REQUIRE(maybe->has_sack(6));
    REQUIRE(maybe->payload == orig.payload);
}

// ---------------------------------------------------------------------------
// DESERIALISE BOUNDARY / ERROR CASES
// ---------------------------------------------------------------------------

TEST_CASE("Packet: deserialize rejects buffer shorter than header", "[packet][serde][error]")
{
    std::vector<uint8_t> tiny(rdt::HEADER_SIZE - 1, 0);
    REQUIRE_FALSE(Packet::deserialize(tiny.data(), tiny.size()).has_value());
}

TEST_CASE("Packet: deserialize rejects buffer too small for declared sack+payload", "[packet][serde][error]")
{
    // Build a valid packet, then truncate the serialised form by 1 byte
    Packet orig;
    orig.add_sack(1);
    orig.payload = {0x01};
    orig.payload_len = 1;
    auto buf = orig.serialize();
    buf.pop_back(); // truncate one byte

    REQUIRE_FALSE(Packet::deserialize(buf.data(), buf.size()).has_value());
}

// ---------------------------------------------------------------------------
// CHECKSUM
// ---------------------------------------------------------------------------

TEST_CASE("Packet: checksum is non-zero for non-trivial packet", "[packet][checksum]")
{
    Packet p;
    p.conn_id = 0x12345678;
    p.seq_num = 42;
    p.payload = {1, 2, 3};
    p.payload_len = 3;
    auto buf = p.serialize();
    // Checksum sits at bytes 14-15 of the fixed header
    uint16_t cs = static_cast<uint16_t>((buf[14] << 8) | buf[15]);
    REQUIRE(cs != 0);
}

TEST_CASE("Packet: re-serialising a deserialised packet yields identical bytes", "[packet][checksum]")
{
    Packet orig;
    orig.conn_id = 0xCAFE;
    orig.seq_num = 1;
    orig.ack_num = 0;
    orig.set_ack();
    orig.add_sack(7);
    orig.payload = {10, 20, 30};
    orig.payload_len = 3;

    auto buf1 = orig.serialize();
    auto maybe = Packet::deserialize(buf1.data(), buf1.size());
    REQUIRE(maybe.has_value());
    auto buf2 = maybe->serialize();

    REQUIRE(buf1 == buf2);
}
