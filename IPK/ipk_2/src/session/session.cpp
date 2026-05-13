/**
 * @file session.cpp
 * @brief Implementation of the Session base class event loop and shared logic.
 * @author Peter Stahl (xstahl01)
 */

#include "session.hpp"
#include "logger.hpp"

#include <poll.h>
#include <random>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <numeric>
#include <stdexcept>
#include <netinet/in.h>
#include <span>

using Log = rdt::utils::Logger;

namespace rdt::domain
{

    Session::Session(ConnConfig config, int sock_fd, const std::atomic<bool> &stop)
        : config_(std::move(config)), sock_fd_(sock_fd), state_(rdt::SessionState::IDLE), stop_(stop)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        isn_local_ = std::uniform_int_distribution<uint32_t>{}(gen);
    }

    rdt::TransferResult Session::run()
    {
        // Buffer for incoming datagrams - max UDP payload
        std::vector<uint8_t> recv_buf(rdt::MAX_UDP_PAYLOAD);

        while (true)
        {
            // stop flag (sigtem / sigint)
            if (stop_.load(std::memory_order_relaxed))
            {
                Log::instance().warn("session", "stop requested by signal");
                change_state_(rdt::SessionState::ERROR);
                return rdt::TransferResult::FAILURE;
            }

            // progress timeout - skipped in TIME_WAIT which has its own completion timer
            if (state_ != rdt::SessionState::TIME_WAIT && progress_timed_out_())
            {
                Log::instance().warn("session", "progress timeout expired ({}s)", config_.timeout_s);
                change_state_(rdt::SessionState::ERROR);
                return rdt::TransferResult::TIMEOUT;
            }

            // poll for incoming data
            // Timeout = RETRANSMIT_INTERVAL so we wake up to retransmiteven when no packet arrives.
            pollfd pfd{};
            pfd.fd = sock_fd_;
            pfd.events = POLLIN;

            int ready = ::poll(&pfd, 1, static_cast<int>(rdt::RETRANSMIT_MS));

            if (ready < 0)
            {
                if (errno == EINTR)
                    continue; // signal interrupted poll, loop again
                Log::instance().error("session", "poll error: " + std::string(std::strerror(errno)));
                change_state_(rdt::SessionState::ERROR);
                return rdt::TransferResult::FAILURE;
            }

            // receive and process incoming packet
            if (ready > 0 && (pfd.revents & POLLIN))
            {
                sockaddr_storage src{};
                socklen_t src_len = sizeof(src);

                ssize_t n = ::recvfrom(sock_fd_, recv_buf.data(), recv_buf.size(), 0, reinterpret_cast<sockaddr *>(&src), &src_len);

                if (n < 0)
                {
                    Log::instance().error("session", "recvfrom error: " + std::string(std::strerror(errno)));
                    continue; // non-fatal, try again
                }

                auto len = static_cast<std::size_t>(n);

                // Validate before dispatching - invalid packets are silently dropped
                if (validate_packet_(recv_buf.data(), len))
                {
                    on_packet_received_(recv_buf.data(), len, src);
                }
            }

            // drain everything sendable: fill window, flush all pending retransmits
            while (auto pkt = next_packet_to_send_())
            {
                (void)send_packet_(*pkt);
            }

            // check completion
            if (is_transfer_complete_())
            {
                change_state_(rdt::SessionState::CLOSED);
                return rdt::TransferResult::SUCCESS;
            }
        }
    }

    void Session::change_state_(rdt::SessionState next)
    {
        Log::instance().info("session", "state: {} --> {}", static_cast<int>(state_), static_cast<int>(next));
        state_ = next;
    }

    bool Session::validate_packet_(const uint8_t *buf, std::size_t len) const
    {
        // Minimum size
        if (len < rdt::HEADER_SIZE)
        {
            Log::instance().debug("session", "dropped: too short ({} B)", len);
            return false;
        }

        // Server skips this during LISTENING state (conn_id not yet known), all other states must match
        if (state_ != rdt::SessionState::LISTENING)
        {
            uint32_t pkt_conn_id{};
            std::memcpy(&pkt_conn_id, buf, sizeof(uint32_t));
            pkt_conn_id = ntohl(pkt_conn_id);

            if (pkt_conn_id != config_.conn_id)
            {
                Log::instance().debug("session", "dropped: conn_id mismatch");
                return false;
            }
        }

        // RFC 1071 checksum over the entire PDU (header + payload (valid packet produces 0x0000 when the checksum field is included)
        if (checksum_({buf, len}) != 0x0000)
        {
            Log::instance().debug("session", "dropped: checksum failed");
            return false;
        }

        // payload_len field is at byte offset 16 (after conn_id, seq, ack, flags, sack_count, checksum)
        uint16_t payload_len{};
        std::memcpy(&payload_len, buf + 16, sizeof(uint16_t));
        payload_len = ntohs(payload_len);

        // sack_count is at byte offset 13 (high nibble of byte 13 used for flags, low 4 bits for sack_count - or separate byte depending on layout)
        // Here: flags at byte 12, sack_count at byte 13
        uint8_t sack_count = buf[13] & 0x0F; // low nibble

        if (std::size_t expected_min = rdt::HEADER_SIZE + sack_count * sizeof(uint32_t) + payload_len; len < expected_min)
        {
            Log::instance().debug("session", "dropped: payload_len inconsistent");
            return false;
        }

        return true;
    }

    uint16_t Session::checksum_(std::span<const uint8_t> data)
    {
        uint32_t sum = 0;
        const uint8_t *ptr = data.data();
        std::size_t len = data.size();

        // Sum 16-bit words (big-endian)
        while (len > 1)
        {
            uint16_t word{};
            std::memcpy(&word, ptr, sizeof(word));
            sum += ntohs(word);
            ptr += 2;
            len -= 2;
        }

        // Pad odd byte
        if (len == 1)
        {
            sum += static_cast<uint16_t>(*ptr) << 8;
        }

        // Fold carries
        while (sum >> 16)
        {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }

        return static_cast<uint16_t>(~sum);
    }

    bool Session::progress_timed_out_() const noexcept
    {
        return progress_timer_.has_expired(config_.timeout_s * 1000u);
    }

    bool Session::send_packet_(const RawPacket &pkt) const
    {
        if (ssize_t sent = ::send(sock_fd_, pkt.data(), pkt.size(), 0); sent < 0)
        {
            Log::instance().error("session", "send error: " + std::string(std::strerror(errno)));
            return false;
        }
        return true;
    }

    void Session::store_ctrl_(RawPacket pkt)
    {
        cached_ctrl_ = std::move(pkt);
        ctrl_timer_.reset();
        ctrl_retransmit_ms_ = rdt::RETRANSMIT_MS;
    }

    bool Session::ctrl_timer_elapsed_() const noexcept
    {
        return ctrl_timer_.has_expired(ctrl_retransmit_ms_);
    }

    std::optional<Session::RawPacket> Session::do_ctrl_retransmit_(std::string_view log_prefix)
    {
        ctrl_retransmit_ms_ = std::min(ctrl_retransmit_ms_ * rdt::BACKOFF_MULTIPLIER, rdt::MAX_RETRANSMIT_MS);
        ctrl_timer_.reset();

        Log::instance().debug("session", "{} (interval = {}ms)", std::string(log_prefix), ctrl_retransmit_ms_);

        return cached_ctrl_;
    }

} // namespace rdt::domain