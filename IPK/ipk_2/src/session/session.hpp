/**
 * @file session.hpp
 * @brief Abstract base class for SenderSession and ReceiverSession.
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef SESSION_HPP
#define SESSION_HPP

#include <atomic>
#include <optional>
#include <span>
#include <vector>

#include "timer.hpp"

#include <sys/socket.h>

#include "rdt_constants.hpp"
#include "connconfig.hpp"

namespace rdt::domain
{

    /**
     * @brief Abstract base for SenderSession and ReceiverSession.
     */
    class Session
    {
    public:
        // Constructor

        explicit Session(ConnConfig config, int sock_fd, const std::atomic<bool> &stop);

        virtual ~Session() = default;

        Session(const Session &) = delete;
        Session &operator=(const Session &) = delete;
        Session(Session &&) = default;
        Session &operator=(Session &&) = default;

        // Public Interfaces

        /**
         * @brief Run the transfer. Blocks until complete, failed, or stopped
         */
        [[nodiscard]] rdt::TransferResult run();

        /**
         * @brief Return the current session state.
         */
        [[nodiscard]] rdt::SessionState state() const noexcept { return state_; }

    protected:
        /**
         * @brief Raw buffer representation of received datagram.
         */
        using RawPacket = std::vector<uint8_t>;

        ConnConfig config_;
        int sock_fd_;
        rdt::SessionState state_;
        uint32_t isn_local_ = 0; // own initial sequence number, randomly generated at construction

        // Cached SYN-ACK / FIN-ACK for server or SYN / FIN for client for retransmission (one at a time)
        std::optional<RawPacket> cached_ctrl_{};
        rdt::utils::Timer ctrl_timer_{};
        uint32_t ctrl_retransmit_ms_ = rdt::RETRANSMIT_MS;

        // Packet built in on_packet_received_, consumed by next_packet_to_send_
        std::optional<RawPacket> pending_send_{};


        // State Machine

        /**
         * @brief Transition to new state and log change
         */
        void change_state_(rdt::SessionState next);

        /**
         * @brief Record that protocol progress has been made, resetting the progress timeout. Call this on every event 
         * that advances the session beyond what was previously known.
         */
        void record_progress_() noexcept { progress_timer_.reset(); }

        /**
         * @brief Validate raw inbound datagram
         */
        [[nodiscard]] bool validate_packet_(const uint8_t *buf, std::size_t len) const;

        /**
         * @brief Compute RFC 1071 checksum over byte span.
         * @return 0x0000 if the data is valid (checksum field included in span).
         */
        [[nodiscard]] static uint16_t checksum_(std::span<const uint8_t> data);

        // Purely virtual hooks
        /**
         * @brief Return the state this session starts in.
         */
        [[nodiscard]] virtual rdt::SessionState initial_state_() const noexcept = 0;

        /**
         * @brief Process a validated inbound packet.
         */
        virtual void on_packet_received_(const uint8_t *buf, std::size_t len, const sockaddr_storage &src) = 0;

        /**
         * @brief Build and return the next packet to transmit, if any.
         */
        [[nodiscard]] virtual std::optional<RawPacket> next_packet_to_send_() = 0;

        /**
         * @brief Return true when the transfer has completed successfully.
         */
        [[nodiscard]] virtual bool is_transfer_complete_() const noexcept = 0;

        void store_ctrl_(RawPacket pkt);

        bool ctrl_timer_elapsed_()const noexcept;

        std::optional<RawPacket> do_ctrl_retransmit_(std::string_view log_prefix);

    private:
        const std::atomic<bool> &stop_;
        rdt::utils::Timer progress_timer_{};

        /// @brief check wether the progress timeout has expired
        [[nodiscard]] bool progress_timed_out_() const noexcept;

        /**
         * @brief Send serialised packet over sock_fd_
         */
        [[nodiscard]] bool send_packet_(const RawPacket &pkt) const;
    };

} // namespace rdt::domain

#endif // SESSION_HPP