/**
 * @file rdt_constants.hpp
 * @brief Central definitions for all protocol constants, enums, and magic-number-free values.
 *        Every numeric literal used in the protocol MUST be referenced from here.
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef RDT_CONSTANTS_HPP
#define RDT_CONSTANTS_HPP

#include <cstddef>
#include <cstdint>

namespace rdt
{
    /**
     * PROTOCOL LIMITS
     */

    /// @brief Maximum UDP payload size (assignment hard limit)
    inline constexpr std::size_t MAX_UDP_PAYLOAD = 1200;

    /// @brief Fixed header size in bytes (conn_id + seq_num + ack_num + flags + sack_count + checksum + payload_len + reserved)
    inline constexpr std::size_t HEADER_SIZE = 20;

    /// @brief Maximum number of SACK entries in one packet (4-bit sack_count field)
    inline constexpr uint8_t MAX_SACK_ENTRIES = 15;

    /// @brief maximum SACK block size in bytes (15 * 4 bytes each )
    inline constexpr std::size_t MAX_SACK_BLOCK_SIZE = MAX_SACK_ENTRIES * sizeof(uint32_t);

    /// @brief maximum payload bytes in single PDU (worst is 15 SACK entries present)
    inline constexpr std::size_t MAX_PAYLOAD_SIZE = MAX_UDP_PAYLOAD - HEADER_SIZE - MAX_SACK_BLOCK_SIZE;

    /// @brief Payload size with no SACK entries present
    inline constexpr std::size_t TYPICAL_PAYLOAD_SIZE = MAX_UDP_PAYLOAD - HEADER_SIZE;

    /**
     * SLIDING WINDOW
     */

    /** NOTE: i used here uin32_t for all even tho it is overkill in many cases to 
     * avoid arithmetic promotion and keep platform consistency 
     */
    ///@brief Base retransmission interval in miliseconds (internal, finer than progress timeout)
    inline constexpr uint32_t RETRANSMIT_MS = 100;

    ///@brief maximum retransmit interval in milisecond
    inline constexpr uint32_t MAX_RETRANSMIT_MS = 3000;

    ///@brief multiplier applied to retransmit interval on each consecutive timeout
    inline constexpr uint32_t BACKOFF_MULTIPLIER = 2;

    /**
     * PACKET FLAGS
     */
    /**
     * bit layout | SIN | ACK | FIN | RST | SACK | rsv | rsv | rsv
     */
    enum class Flag : uint8_t {
        SYN = 0x80,
        ACK = 0x40,
        FIN = 0X20,
        RST = 0x10,
    };
    
    // support for SYN-ACK etc. 
    inline constexpr Flag operator|(Flag a, Flag b) {
        return static_cast<Flag>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }
    inline constexpr bool operator&(uint8_t flags, Flag flag) {
        return (flags & static_cast<uint8_t>(flag)) != 0;
    }

    /**
     * SESSION STATES
     */

    enum class SessionState : uint8_t {
        // client side
        IDLE,           // Initial state before any action
        SYN_SENT,       // SYN transmitted, waiting for SYN-ACK
        ESTABLISHED,    // Handshake complete, about to send data
        TRANSFERRING,   // Actively sending DATA segments
        FIN_WAIT,       // FIN sent, waiting for FIN-ACK
        TIME_WAIT,      // Final ACK sent, lingering for possible FIN-ACK retransmit

        // server side
        LISTENING,      // Waiting for a valid SYN
        SYN_RCVD,       // SYN received, SYN-ACK sent, waiting for ACK
        RECEIVING,      // Actively receiving DATA segments
        FIN_RCVD,       // FIN received, FIN-ACK sent, waiting for final ACK

        CLOSED,         // Session ended successfully
        ERROR,          // Session ended with failure (timeout, RST, malformed packet)
    };
    
    enum class TransferResult : uint8_t {
        SUCCESS = 0,
        FAILURE = 1,
        TIMEOUT = 2,
    };

    enum class AppMode : uint8_t {
        CLIENT,
        SERVER
    };

    /**
     * EXIT CODES
     */

    enum class ExitCode : uint8_t {
        OK = 0,
        ERR_ARGS = 1,
        ERR_NETWORK = 2, 
        ERR_DNS = 3,
        ERR_TIMEOUT = 4,
        ERR_PROTOCOL = 5,
        ERR_IO = 6,
        ERR_INTERNAL = 99
    };

} // namespace rdt

#endif // RDT_CONSTANTS_HPP