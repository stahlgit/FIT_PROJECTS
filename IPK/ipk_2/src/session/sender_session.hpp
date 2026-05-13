/**
 * @file sender_session.hpp
 * @brief SenderSession - client side reliable transfer session
 * @author Peter Stahl (xstahl01)
 * Uses a SendWindow for pipelined, selective-repeat delivery and reads data from a std::istream (file or stdin).
 */

#pragma once
#ifndef SENDER_SESSION_HPP
#define SENDER_SESSION_HPP

#include <iosfwd>
#include <optional>
#include <vector>

#include "session.hpp"
#include "send_window.hpp"

namespace rdt::domain
{

    class SenderSession : public Session //final class - removed because of testing
    {
    public:
        /// @brief Construct sender session
        explicit SenderSession(ConnConfig config, int sock_fd, const std::atomic<bool> &stop, std::istream &input);

    protected:
        /// @brief Returns HANDSHAKE_SYN — sender initiates the three-way handshake.
        [[nodiscard]] rdt::SessionState initial_state_() const noexcept override;

        /// @brief Dispatch inbound packet to the handler for the current state (handshake / data / teardown).
        void on_packet_received_(const uint8_t *buf, std::size_t len, const sockaddr_storage &src) override;

        /// @brief Return the next segment or control packet to transmit, or nullopt if nothing is ready.
        [[nodiscard]] std::optional<RawPacket> next_packet_to_send_() override;

        /// @brief Return true once the peer has acknowledged all data and the TIME_WAIT linger has expired.
        [[nodiscard]] bool is_transfer_complete_() const noexcept override;




    private:
        SendWindow window_;
        std::istream &input_;
        bool input_exhausted_ = false;

        ///@brief Peer's initial sequence number, learned from SYN-ACK during handshake
        uint32_t isn_server_ = 0;

        // TIME_WAIT: started when we enter the linger phase, absent before that
        std::optional<rdt::utils::Timer> time_wait_timer_{};

        /// Read one TYPICAL_PAYLOAD_SIZE chunk from input_, push to window, return packet.
        /// Sets input_exhausted_ on EOF. Returns nullopt if nothing was read.
        [[nodiscard]] std::optional<RawPacket> load_next_segment_();
    };

} // namespace rdt::domain

#endif // SENDER_SESSION_HPP
