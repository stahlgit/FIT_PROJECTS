/**
 * @file sender_session.cpp
 * @brief SenderSession implementation - client-side state machine.
 * @author Peter Stahl (xstahl01)
 */

#include "sender_session.hpp"
#include "packet.hpp"
#include "logger.hpp"

#include <istream>

using Log = rdt::utils::Logger;

namespace rdt::domain
{

    SenderSession::SenderSession(ConnConfig config, int sock_fd, const std::atomic<bool> &stop, std::istream &input)
        : Session(std::move(config), sock_fd, stop), window_(config_.window_size, 0), input_(input)
    {
        state_ = initial_state_(); // vtable is set here; safe to call virtual
    }

    rdt::SessionState SenderSession::initial_state_() const noexcept
    {
        return rdt::SessionState::IDLE;
    }

    void SenderSession::on_packet_received_(const uint8_t *buf, std::size_t len, const sockaddr_storage & /*src*/)
    {
        auto opt = Packet::deserialize(buf, len);
        if (!opt.has_value())
            return;
        const auto &pkt = *opt;

        switch (state_)
        {
        case rdt::SessionState::SYN_SENT:
        {
            if (!pkt.is_syn() || !pkt.is_ack())
                break; // drop anything that is not SYN-ACK

            // Validate: server must ack our ISN
            if (pkt.ack_num != isn_local_)
            {
                Log::instance().debug("sender", "SYN-ACK has wrong ack_num={}, expected={}", pkt.ack_num, isn_local_);
                break;
            }

            isn_server_ = pkt.seq_num;
            record_progress_();
            cached_ctrl_.reset();

            // ACK the server SYN-ACK
            auto ack = Packet::make_ack(config_.conn_id, isn_server_);
            pending_send_ = ack.serialize();

            change_state_(rdt::SessionState::ESTABLISHED);
            Log::instance().info("sender", "SYN-ACK received, isn_s=", isn_server_);
            break;
        }

        case rdt::SessionState::TRANSFERRING:
        {
            // Drop SYN-ACK duplicates - their ack_num is the ISN, not a data offset,
            // and feeding it into on_ack() would corrupt the window base.
            if (!pkt.is_ack() || pkt.is_syn())
                break;

            uint32_t newly_acked = window_.on_ack(pkt.ack_num);

            for (uint8_t i = 0; i < pkt.sack_count; ++i)
                window_.on_sack(pkt.sack_blocks[i]);

            if (newly_acked > 0)
                record_progress_();

            Log::instance().debug("sender", "ACK ack={} sack_count={}", pkt.ack_num, static_cast<int>(pkt.sack_count));
            break;
        }

        case rdt::SessionState::FIN_WAIT:
        {
            if (!pkt.is_fin() || !pkt.is_ack())
                break;

            record_progress_();
            cached_ctrl_.reset();

            // Send final ACK acknowledging server's FIN-ACK
            auto ack = Packet::make_ack(config_.conn_id, pkt.seq_num);
            pending_send_ = ack.serialize();

            time_wait_timer_.emplace();
            change_state_(rdt::SessionState::TIME_WAIT);
            Log::instance().info("sender", "FIN-ACK received, entering TIME_WAIT");
            break;
        }

        case rdt::SessionState::TIME_WAIT:
        {
            // Server is retransmitting FIN-ACK - re-send our ACK
            if (pkt.is_fin() && pkt.is_ack())
            {
                record_progress_();
                auto ack = Packet::make_ack(config_.conn_id, pkt.seq_num);
                pending_send_ = ack.serialize();
                Log::instance().debug("sender", "FIN-ACK retransmit in TIME_WAIT, re-ACKing");
            }
            break;
        }

        default:
            break;
        }
    }

    std::optional<Session::RawPacket> SenderSession::next_packet_to_send_()
    {
        using S = rdt::SessionState;

        switch (state_)
        {
        case S::IDLE:
        {
            // First action: send SYN with our ISN
            Packet syn = Packet::make_syn(config_.conn_id, isn_local_);
            auto raw = syn.serialize();
            store_ctrl_(raw);
            change_state_(S::SYN_SENT);
            Log::instance().info("sender", "SYN sent conn_id={} isn_c={}", config_.conn_id, isn_local_);
            return raw;
        }

        case S::SYN_SENT:
        {
            if (cached_ctrl_.has_value() && ctrl_timer_elapsed_())
            {
                return do_ctrl_retransmit_("[sender] SYN retransmit");
            }
            return std::nullopt;
        }

        case S::ESTABLISHED:
        {
            // Deliver the pending ACK (response to SYN-ACK), then start transfer
            if (pending_send_.has_value())
            {
                auto pkt = std::move(*pending_send_);
                pending_send_.reset();
                change_state_(S::TRANSFERRING);
                return pkt;
            }
            return std::nullopt;
        }

        case S::TRANSFERRING:
        {
            // Priority 1: deliver any pending ACK (shouldn't normally queue here)
            if (pending_send_.has_value())
            {
                auto pkt = std::move(*pending_send_);
                pending_send_.reset();
                return pkt;
            }

            // Priority 2: retransmit one expired segment (resets only that segment's timer)
            if (auto candidate = window_.next_retransmit(); candidate.has_value())
            {
                auto const &[seq, data] = *candidate;
                return Packet::make_data(config_.conn_id, seq, data).serialize();
            }

            // Priority 3: send next new segment if window has room
            if (!window_.is_full() && !input_exhausted_)
                return load_next_segment_();

            // Priority 4: all data acked and input done → teardown
            if (window_.is_empty() && input_exhausted_)
            {
                Packet fin = Packet::make_fin(config_.conn_id, window_.next_seq());
                auto raw = fin.serialize();
                store_ctrl_(raw);
                change_state_(S::FIN_WAIT);
                Log::instance().info("sender", "FIN sent seq={}", window_.next_seq() - 1);
                return raw;
            }

            return std::nullopt; // window full, waiting for ACKs
        }

        case S::FIN_WAIT:
        {
            if (cached_ctrl_.has_value() && ctrl_timer_elapsed_())
            {
                return do_ctrl_retransmit_("[sender] FIN retransmit");
            }
            return std::nullopt;
        }

        case S::TIME_WAIT:
        {
            // Send final ACK if queued (or re-send on FIN-ACK retransmit)
            if (pending_send_.has_value())
            {
                auto pkt = std::move(*pending_send_);
                pending_send_.reset();
                return pkt;
            }
            return std::nullopt;
        }

        default:
            return std::nullopt;
        }
    }

    bool SenderSession::is_transfer_complete_() const noexcept
    {
        if (state_ != rdt::SessionState::TIME_WAIT)
            return false;
        if (!time_wait_timer_.has_value())
            return false;
        return time_wait_timer_->has_expired(2u * rdt::MAX_RETRANSMIT_MS);
    }

    std::optional<Session::RawPacket> SenderSession::load_next_segment_()
    {
        std::vector<uint8_t> chunk(rdt::TYPICAL_PAYLOAD_SIZE);
        input_.read(reinterpret_cast<char *>(chunk.data()),
                    static_cast<std::streamsize>(rdt::TYPICAL_PAYLOAD_SIZE));
        std::streamsize n = input_.gcount();

        if (n == 0)
        {
            input_exhausted_ = true;
            return std::nullopt;
        }

        chunk.resize(static_cast<std::size_t>(n));

        if (input_.eof())
            input_exhausted_ = true;

        uint32_t seq = window_.next_seq();
        auto raw = Packet::make_data(config_.conn_id, seq, chunk).serialize();
        window_.push(seq, std::move(chunk));

        Log::instance().debug("sender", "DATA seq={} len={}", seq, n);
        return raw;
    }
} // namespace rdt::domain
