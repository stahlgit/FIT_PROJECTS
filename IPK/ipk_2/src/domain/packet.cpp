/**
 * @file packet.cpp
 * @brief Packet serialisation, deserialisation, and checksum implementation.
 * @author Peter Stahl (xstahl01)
 */

#include "packet.hpp"
#include "logger.hpp"

#include <netinet/in.h>
#include <cstring>
#include <format>

using Log = rdt::utils::Logger;

namespace rdt::domain
{

    std::vector<uint8_t> Packet::serialize() const
    {
        // Total size: fixed header + SACK block + payload
        std::size_t sack_bytes = sack_count * sizeof(uint32_t);
        std::size_t total = rdt::HEADER_SIZE + sack_bytes + payload.size();

        std::vector<uint8_t> buf(total, 0);
        uint8_t *p = buf.data();

        // Fixed header

        // conn_id (32-bit, network byte order)
        uint32_t net32 = htonl(conn_id);
        std::memcpy(p + OFF_CONN_ID, &net32, 4);

        // seq_num
        net32 = htonl(seq_num);
        std::memcpy(p + OFF_SEQ_NUM, &net32, 4);

        // ack_num
        net32 = htonl(ack_num);
        std::memcpy(p + OFF_ACK_NUM, &net32, 4);

        // flags (1 byte, no endian conversion needed)
        p[OFF_FLAGS] = flags;

        // sack_count - low nibble only (high nibble reserved, stays 0)
        p[OFF_SACK_COUNT] = sack_count & 0x0F;

        // checksum placeholder - filled after full buffer is written
        p[OFF_CHECKSUM] = 0;
        p[OFF_CHECKSUM + 1] = 0;

        // payload_len (16-bit, network byte order)
        uint16_t net16 = htons(static_cast<uint16_t>(payload.size()));
        std::memcpy(p + OFF_PAYLOAD_LEN, &net16, 2);

        // reserved (16-bit, stays 0)
        p[OFF_RESERVED] = 0;
        p[OFF_RESERVED + 1] = 0;

        // Variable SACK block

        uint8_t *sack_ptr = p + OFF_SACK_BLOCK;
        for (uint8_t i = 0; i < sack_count; ++i)
        {
            net32 = htonl(sack_blocks[i]);
            std::memcpy(sack_ptr + i * 4, &net32, 4);
        }

        //  Payload

        if (!payload.empty())
        {
            std::memcpy(p + OFF_SACK_BLOCK + sack_bytes,
                        payload.data(),
                        payload.size());
        }

        // Checksum (RFC 1071 over entire buffer with checksum field = 0)
        uint32_t sum = 0;
        const uint8_t *ptr = buf.data();
        std::size_t len = buf.size();

        while (len > 1)
        {
            uint16_t word{};
            std::memcpy(&word, ptr, 2);
            sum += ntohs(word);
            ptr += 2;
            len -= 2;
        }
        if (len == 1)
        {
            sum += static_cast<uint16_t>(*ptr) << 8;
        }
        while (sum >> 16)
        {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }
        uint16_t cs = htons(static_cast<uint16_t>(~sum));
        std::memcpy(p + OFF_CHECKSUM, &cs, 2);

        return buf;
    }

    std::optional<Packet> Packet::deserialize(const uint8_t *buf, std::size_t len)
    {
        // Minimum size check
        if (len < rdt::HEADER_SIZE)
        {
            Log::instance().debug("packet", "deserialise: too short ({} B)", len);
            return std::nullopt;
        }

        Packet pkt;

        //  Fixed header fields

        uint32_t net32{};
        std::memcpy(&net32, buf + OFF_CONN_ID, 4);
        pkt.conn_id = ntohl(net32);

        std::memcpy(&net32, buf + OFF_SEQ_NUM, 4);
        pkt.seq_num = ntohl(net32);

        std::memcpy(&net32, buf + OFF_ACK_NUM, 4);
        pkt.ack_num = ntohl(net32);

        pkt.flags = buf[OFF_FLAGS];
        pkt.sack_count = buf[OFF_SACK_COUNT] & 0x0F; // low nibble only

        uint16_t net16{};
        std::memcpy(&net16, buf + OFF_CHECKSUM, 2);
        pkt.checksum = ntohs(net16);

        std::memcpy(&net16, buf + OFF_PAYLOAD_LEN, 2);
        pkt.payload_len = ntohs(net16);

        // Validate SACK block fits in buffer
        std::size_t sack_bytes = pkt.sack_count * sizeof(uint32_t);
        std::size_t payload_start = OFF_SACK_BLOCK + sack_bytes;

        if (len < payload_start + pkt.payload_len)
        {
            Log::instance().debug("packet", "deserialise: buffer too small for declared sack_count + payload_len");
            return std::nullopt;
        }

        // SACK block
        for (uint8_t i = 0; i < pkt.sack_count; ++i)
        {
            std::memcpy(&net32, buf + OFF_SACK_BLOCK + i * 4, 4);
            pkt.sack_blocks[i] = ntohl(net32);
        }

        //  Payload
        if (pkt.payload_len > 0)
        {
            pkt.payload.assign(buf + payload_start, buf + payload_start + pkt.payload_len);
        }

        return pkt;
    }

} // namespace rdt::domain