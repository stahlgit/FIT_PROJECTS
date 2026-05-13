/**
 * @file packet.hpp
 * @brief RDT protocol packet - header definition, serialization and deserialization
 * @author Peter Stahl (xstahl01)
 *
 * Packet is pure value type in domain layer. It has no knowledge of sockets, sessions or IO.
 */

#pragma once
#ifndef PACKET_HPP
#define PACKET_HPP

#include <cstdint>
#include <array>
#include <span>
#include <vector>
#include <optional>

#include "rdt_constants.hpp"

namespace rdt::domain
{
    /**
     * @brief Memory representation of RDT PDU
     */
    struct Packet
    {
        uint32_t conn_id = 0;
        uint32_t seq_num = 0;
        uint32_t ack_num = 0;
        uint8_t flags = 0;
        uint8_t sack_count = 0;
        uint16_t checksum = 0;
        uint16_t payload_len = 0;

        ///@brief SAK block
        std::array<uint32_t, rdt::MAX_SACK_ENTRIES> sack_blocks{};

        ///@brief payload bytes
        std::vector<uint8_t> payload;

        // FLAG HELPERS

        [[nodiscard]] bool is_syn() const noexcept
        {
            return flags & static_cast<uint8_t>(rdt::Flag::SYN);
        }
        [[nodiscard]] bool is_ack() const noexcept
        {
            return flags & static_cast<uint8_t>(rdt::Flag::ACK);
        }
        [[nodiscard]] bool is_fin() const noexcept
        {
            return flags & static_cast<uint8_t>(rdt::Flag::FIN);
        }
        [[nodiscard]] bool is_rst() const noexcept
        {
            return flags & static_cast<uint8_t>(rdt::Flag::RST);
        }

        void set_syn() noexcept { flags |= static_cast<uint8_t>(rdt::Flag::SYN); }
        void set_ack() noexcept { flags |= static_cast<uint8_t>(rdt::Flag::ACK); }
        void set_fin() noexcept { flags |= static_cast<uint8_t>(rdt::Flag::FIN); }
        void set_rst() noexcept { flags |= static_cast<uint8_t>(rdt::Flag::RST); }

        void clear_syn() noexcept { flags &= ~static_cast<uint8_t>(rdt::Flag::SYN); }
        void clear_ack() noexcept { flags &= ~static_cast<uint8_t>(rdt::Flag::ACK); }
        void clear_fin() noexcept { flags &= ~static_cast<uint8_t>(rdt::Flag::FIN); }
        void clear_rst() noexcept { flags &= ~static_cast<uint8_t>(rdt::Flag::RST); }

        // SACK HELPERS
        /**
         * @brief Add segment number to SACK block
         * @return true if added, false if block is full
         */
        bool add_sack(uint32_t seg) noexcept
        {
            if (sack_count >= rdt::MAX_SACK_ENTRIES)
                return false;
            sack_blocks[sack_count++] = seg;
            return true;
        }

        [[nodiscard]] bool has_sack(uint32_t seg) const noexcept
        {
            for (uint8_t i = 0; i < sack_count; ++i)
            {
                if (sack_blocks[i] == seg)
                    return true;
            }
            return false;
        }

        // no need to update or delete i believe

        // Factory helpers for common packet types
        [[nodiscard]] static Packet make_syn(uint32_t conn_id, uint32_t isn)
        {
            Packet p;
            p.conn_id = conn_id;
            p.seq_num = isn;
            p.set_syn();
            return p;
        }

        [[nodiscard]] static Packet make_ack(uint32_t conn_id, uint32_t ack){
            Packet p;
            p.conn_id = conn_id;
            p.ack_num = ack;
            p.set_ack();
            return p;
        }

        [[nodiscard]] static Packet make_syn_ack(uint32_t conn_id, uint32_t isn, uint32_t ack)
        {
            Packet p;
            p.conn_id = conn_id;
            p.seq_num = isn;
            p.ack_num = ack;
            p.set_syn();
            p.set_ack();
            return p;
        }

        [[nodiscard]] static Packet make_fin(uint32_t conn_id, uint32_t isn)
        {
            Packet p;
            p.conn_id = conn_id;
            p.seq_num = isn;
            p.set_fin();
            return p;
        }

        [[nodiscard]] static Packet make_fin_ack(uint32_t conn_id, uint32_t isn, uint32_t ack){
            Packet p;
            p.conn_id = conn_id;
            p.seq_num = isn;
            p.ack_num = ack;
            p.set_fin();
            p.set_ack();
            return p;
        }

        [[nodiscard]] static Packet make_rst(uint32_t conn_id)
        {
            Packet p;
            p.conn_id = conn_id;
            p.set_rst();
            return p;
        }

        [[nodiscard]] static Packet make_data(uint32_t conn_id, uint32_t seq, std::vector<uint8_t> data)
        {
            Packet p;
            p.conn_id = conn_id;
            p.seq_num = seq;
            p.payload_len = static_cast<uint16_t>(data.size());
            p.payload = std::move(data);
            return p;
        }

        [[nodiscard]] static Packet make_ack_sack(uint32_t conn_id, uint32_t ack, std::span<const uint32_t> sack_entries)
        {
            Packet p = make_ack(conn_id, ack);
            for (uint32_t seq : sack_entries)
                if (!p.add_sack(seq)) break; // full sack 
            return p;
        }

        // Serialization
        [[nodiscard]] std::vector<uint8_t> serialize() const;

        // Deserialize
        [[nodiscard]] static std::optional<Packet> deserialize(const uint8_t *buf, std::size_t len);

    private:
        // Offsets... idk 
        static constexpr std::size_t OFF_CONN_ID = 0;
        static constexpr std::size_t OFF_SEQ_NUM = 4;
        static constexpr std::size_t OFF_ACK_NUM = 8;
        static constexpr std::size_t OFF_FLAGS = 12;
        static constexpr std::size_t OFF_SACK_COUNT = 13;
        static constexpr std::size_t OFF_CHECKSUM = 14;
        static constexpr std::size_t OFF_PAYLOAD_LEN = 16;
        static constexpr std::size_t OFF_RESERVED = 18;
        static constexpr std::size_t OFF_SACK_BLOCK = 20; ///< Start of variable SACK block
    };
}

#endif // PACKET_HPP
