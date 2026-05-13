/**
 * @file receiver_session.cpp
 * @brief ReceiverSession implementation - server-side state machine.
 * @author Peter Stahl (xstahl01)
 */

#include "receiver_session.hpp"
#include "packet.hpp"
#include "logger.hpp"

#include <ostream>
#include <sys/socket.h>

using Log = rdt::utils::Logger;


namespace rdt::domain
{
    ReceiverSession::ReceiverSession(ConnConfig config, int sock_fd, const std::atomic<bool> &stop, std::ostream &output)
        : Session(std::move(config), sock_fd, stop), buffer_(config_.window_size, 0), output_(output)
    {
        state_ = initial_state_(); // vtable is set here; safe to call virtual
    }

    rdt::SessionState ReceiverSession::initial_state_() const noexcept
    {
        return rdt::SessionState::LISTENING;
    }

    void ReceiverSession::on_packet_received_(const uint8_t *buf, std::size_t len, const sockaddr_storage &src)
    {
        auto opt = Packet::deserialize(buf, len);
        if (!opt.has_value())
            return;
        const auto &pkt = *opt;

        switch (state_)
        {
        case rdt::SessionState::LISTENING:
        {
            if (!pkt.is_syn() || pkt.is_ack())
                break; // must be plain SYN

            // Adopt client's conn_id for all future packets
            config_.conn_id = pkt.conn_id;

            // Connect the UDP socket to the client so ::send() works in base class
            if (::connect(sock_fd_, reinterpret_cast<const sockaddr *>(&src), sizeof(src)) < 0)
            {
                Log::instance().error("receiver", "connect() failed - cannot reach client");
                break;
            }

            record_progress_();

            // SYN-ACK: our seq = isn_s_, ack = client's ISN
            auto synack = Packet::make_syn_ack(config_.conn_id, isn_local_, pkt.seq_num);
            auto raw = synack.serialize();
            store_ctrl_(raw);
            pending_send_ = raw;

            change_state_(rdt::SessionState::SYN_RCVD);
            Log::instance().info("receiver", "SYN received conn_id={} isn_s", config_.conn_id, isn_local_);
            break;
        }

        case rdt::SessionState::SYN_RCVD:
        {
            if (pkt.is_syn() || pkt.is_fin() || pkt.is_rst())
                break;

            // Explicit ACK (pure control, no payload) - verify ack_num
            if (pkt.is_ack() && pkt.payload_len == 0) {
                if (pkt.ack_num != isn_local_) {
                    Log::instance().debug("receiver", "ACK ack_num={} expected isn_s= ",pkt.ack_num, isn_local_);
                    break;
                }
                record_progress_();
                cached_ctrl_.reset();
                pending_send_.reset();
                change_state_(rdt::SessionState::ESTABLISHED);
                Log::instance().info("receiver", "handshake complete");
                break;
            }

            // Data packet: SYN-ACK was received (client is already sending data),
            // but the client's ACK was lost. Accept this as an implicit handshake
            // completion and fall through to buffer the data.
            if (pkt.payload_len > 0) {
                record_progress_();
                cached_ctrl_.reset();
                pending_send_.reset();
                change_state_(rdt::SessionState::ESTABLISHED);
                Log::instance().info("receiver", "handshake complete (implicit - client ACK lost)");
                [[fallthrough]];
            } else {
                break;
            }
        }

        case rdt::SessionState::ESTABLISHED:
        case rdt::SessionState::RECEIVING:
        {
            // FIN takes priority over stray data
            if (pkt.is_fin() && !pkt.is_syn())
            {
                // Drain whatever is still in the buffer before closing
                auto segments = buffer_.drain();
                flush_to_output_(segments);

                record_progress_();

                auto finack = Packet::make_fin_ack(config_.conn_id, isn_local_, buffer_.ack_num());
                auto raw = finack.serialize();
                store_ctrl_(raw);
                pending_send_ = raw;

                change_state_(rdt::SessionState::FIN_RCVD);
                Log::instance().info("receiver", "FIN received, sending FIN-ACK");
                break;
            }

            if (pkt.payload_len == 0)
                break; // pure ACK or empty packet, nothing to buffer

            if (state_ == rdt::SessionState::ESTABLISHED)
                change_state_(rdt::SessionState::RECEIVING);

            auto result = buffer_.insert(pkt.seq_num, pkt.payload);

            if (result == ReceiveBuffer::InsertResult::DUPLICATE)
            {
                // Always send ACK so sender knows we have this gap covered
                pending_send_ = Packet::make_ack_sack(config_.conn_id, buffer_.ack_num(), buffer_.sack_entries()).serialize();
                Log::instance().debug("receiver", "duplicate seq={}, re-ACKing", pkt.seq_num);
                break;
            }

            if (result == ReceiveBuffer::InsertResult::NEW)
            {
                record_progress_();
                auto segments = buffer_.drain();
                flush_to_output_(segments);
                pending_send_ = Packet::make_ack_sack(config_.conn_id, buffer_.ack_num(), buffer_.sack_entries()).serialize();
            }

            break;
        }

        case rdt::SessionState::FIN_RCVD:
        {
            // Expect the client's final ACK
            if (!pkt.is_ack() || pkt.is_syn() || pkt.is_fin())
                break;

            record_progress_();
            cached_ctrl_.reset();
            pending_send_.reset();

            change_state_(rdt::SessionState::CLOSED);
            Log::instance().info("receiver", "final ACK received, session closed");
            break;
        }

        default:
            break;
        }
    }

    std::optional<Session::RawPacket> ReceiverSession::next_packet_to_send_()
    {
        using S = rdt::SessionState;

        // Return any reactively built packet first (ACK, SYN-ACK, FIN-ACK)
        if (pending_send_.has_value())
        {
            auto pkt = std::move(*pending_send_);
            pending_send_.reset();
            return pkt;
        }

        // Retransmit SYN-ACK (SYN_RCVD) or FIN-ACK (FIN_RCVD) on timer
        if ((state_ == S::SYN_RCVD || state_ == S::FIN_RCVD) &&
            cached_ctrl_.has_value() && ctrl_timer_elapsed_())
        {
            return do_ctrl_retransmit_("[receiver] ctrl retransmit");
        }

        return std::nullopt;
    }

    bool ReceiverSession::is_transfer_complete_() const noexcept
    {
        return state_ == rdt::SessionState::CLOSED;
    }

    void ReceiverSession::flush_to_output_(const std::vector<ReceiveBuffer::DeliveredSegment> &segments)
    {
        for (const auto &seg : segments)
        {
            output_.write(reinterpret_cast<const char *>(seg.data.data()), static_cast<std::streamsize>(seg.data.size()));
        }
        if (!segments.empty())
            output_.flush();
    }

} // namespace rdt::domain
