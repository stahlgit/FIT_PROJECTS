/**
 * @file send_window.hpp
 * @brief Sliding send window for RDT reliable transport protocol
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef SEND_WINDOW_HPP
#define SEND_WINDOW_HPP

#include <map>
#include <optional>
#include <vector>

#include "rdt_constants.hpp"
#include "timer.hpp"

namespace rdt::domain
{
    class SendWindow
    {
    public:
        /// @brief Metadata for one in-flight segment
        struct SegmentEntry
        {
            std::vector<uint8_t> data;
            rdt::utils::Timer sent_timer{};   // starts on construction, reset on retransmit
            bool sack_marked = false;
            uint32_t retransmit_ms = RETRANSMIT_MS;

            explicit SegmentEntry(std::vector<uint8_t> d) : data(std::move(d)) {}
        };

        explicit SendWindow(uint32_t window_size, uint32_t initial_seq = 0) : window_size_(window_size), base_(initial_seq), next_seq_(initial_seq) {}

        /// @brief statement method if window is full
        [[nodiscard]] bool is_full() const noexcept
        {
            return (next_seq_ - base_) >= window_size_;
        }

        /// @brief statement method if window is empty
        [[nodiscard]] bool is_empty() const noexcept
        {
            return entries_.empty();
        }

        /// @brief current window base (oldest unACK sequence number)
        [[nodiscard]] uint32_t base() const noexcept { return base_; }

        /// @brief next sequence number
        [[nodiscard]] uint32_t next_seq() const noexcept { return next_seq_; }

        /// sending
        /// @brief reacord newly sent segment in window
        void push(uint32_t seq, std::vector<uint8_t> data);

        /// @brief process cumulative ACK
        uint32_t on_ack(uint32_t ack_num);

        /// @brief process sack entry
        bool on_sack(uint32_t seq);

        /// @brief collect segment that need retransmission
        [[nodiscard]] std::vector<std::pair<uint32_t, std::vector<uint8_t>>> retransmit_candidates();

        /**
         * @brief Return one expired segment and reset only its timer (nullopt when none remain)
         * Used by the event loop to drain retransmits one at a time without resetting all timers at once
         */
        [[nodiscard]] std::optional<std::pair<uint32_t, std::vector<uint8_t>>> next_retransmit();

    private:
        uint32_t window_size_;
        uint32_t base_;
        uint32_t next_seq_;
        std::map<uint32_t, SegmentEntry> entries_;
    };
} // namespace rdt::domain

#endif // SEND_WINDOW_HPP