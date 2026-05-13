/**
 * @file receiver_session.hpp
 * @brief ReceiverSession - server side reliable transfer session.
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef RECEIVER_SESSION_HPP
#define RECEIVER_SESSION_HPP

#include <iosfwd>
#include <optional>
#include <vector>

#include "session.hpp"
#include "receive_buffer.hpp"

namespace rdt::domain
{

    class ReceiverSession : public Session
    {
    public:
        /// @brief Construct a receiver session
        explicit ReceiverSession(ConnConfig config, int sock_fd,const std::atomic<bool> &stop,std::ostream &output);

    protected:
        /// @brief Returns HANDSHAKE_SYN_WAIT — receiver waits for the client's opening SYN.
        [[nodiscard]] rdt::SessionState initial_state_() const noexcept override;

        /// @brief Dispatch inbound packet to the handler for the current state (handshake / data / teardown).
        void on_packet_received_(const uint8_t *buf, std::size_t len, const sockaddr_storage &src) override;

        /// @brief Return a cumulative ACK (with SACK blocks) or control packet to transmit, or nullopt if nothing is ready.
        [[nodiscard]] std::optional<RawPacket> next_packet_to_send_() override;

        /// @brief Return true once the FIN-ACK exchange is complete and all data has been flushed to output.
        [[nodiscard]] bool is_transfer_complete_() const noexcept override;

    private:
        ReceiveBuffer buffer_;
        std::ostream &output_;


        /// Write delivered segments to output_ in order.
        void flush_to_output_(const std::vector<ReceiveBuffer::DeliveredSegment> &segments);

    };

} // namespace rdt::domain

#endif // RECEIVER_SESSION_HPP
