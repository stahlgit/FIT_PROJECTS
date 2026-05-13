/**
 * @file receive_buffer.hpp
 * @brief Reorder buffer for RDT transport receiver (server)
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef RECEIVE_BUFFER_HPP
#define RECEIVE_BUFFER_HPP
#include <cstdint>
#include <vector>
#include <map>
namespace rdt::domain
{
    class ReceiveBuffer
    {
    public:
        enum class InsertResult
        {
            NEW,
            DUPLICATE,
            INVALID
        };
        struct DeliveredSegment
        {
            uint32_t seq;
            std::vector<uint8_t> data;
        };
        explicit ReceiveBuffer(uint32_t window_size, uint32_t initial_seq) : window_size_(window_size), next_expected_(initial_seq) {}

        /**
         * @brief Current ACK value
         */
        [[nodiscard]] uint32_t ack_num() const noexcept { return next_expected_; }

        /**
         * @brief Sequence number of buffered out of order segments
         */
        [[nodiscard]] std::vector<uint32_t> sack_entries() const;

        /**
         * @brief statement method if buffer is empty
         */
        [[nodiscard]] bool is_empty() const noexcept { return buffer_.empty(); }

        /**
         * @brief Attempt to insert incoming segment
         */
        [[nodiscard]] InsertResult insert(uint32_t seq, std::vector<uint8_t> data);

        /**
         * @brief Extract all segments from next_expected_ 
         */
        [[nodiscard]] std::vector<DeliveredSegment> drain();
    private:
        uint32_t window_size_;
        uint32_t next_expected_;
        std::map<uint32_t, std::vector<uint8_t>> buffer_;
    };
} // rdt::domain

#endif // RECEIVE_BUFFER_HPP