/**
 * @file receive_buffer.cpp
 * @brief Implementation of Receive Buffer
 * @author Peter Stahl (xstahl01)
 */
#include "receive_buffer.hpp"
#include "rdt_constants.hpp"
#include "logger.hpp"

#include <format>

using Log = rdt::utils::Logger;

namespace rdt::domain
{
    ReceiveBuffer::InsertResult ReceiveBuffer::insert(uint32_t seq, std::vector<uint8_t> data)
    {
        using enum rdt::domain::ReceiveBuffer::InsertResult;
        if (seq < next_expected_)
        {
            Log::instance().debug("recv_buffer", "duplicate (already delivered) seq={}, expected={}", seq, next_expected_);
            return DUPLICATE;
        }
        if (buffer_.contains(seq))
        {
            Log::instance().debug("recv_buffer", "duplicate (already buffered) seq={}", seq);
            
            return DUPLICATE;
        }
        if (seq >= next_expected_ + window_size_)
        {
            Log::instance().debug("recv_buffer","invalid (outside window) seq={}, window=[{}, {}]", seq, next_expected_, next_expected_ + window_size_);
            return INVALID;
        }

        buffer_.try_emplace(seq, data);
        Log::instance().debug("recv_buffer", "buffered seq={}, buffered_count={}", seq, buffer_.size());
        return NEW;
    }

    std::vector<ReceiveBuffer::DeliveredSegment> ReceiveBuffer::drain()
    {
        std::vector<DeliveredSegment> delivered;

        // Walk from next_expected_ forward while entries are contiguous
        while (true)
        {
            auto it = buffer_.find(next_expected_);
            if (it == buffer_.end())
                break; // Gap - can't deliver further

            delivered.emplace_back(next_expected_, std::move(it->second));
            buffer_.erase(it);
            ++next_expected_;
        }

        if (!delivered.empty())
        {
            Log::instance().debug("recv_buffer", "drained {} + segment(s), next_expected={}, buffered_remaining={}", delivered.size(), next_expected_, buffer_.size());
        }

        return delivered;
    }

    std::vector<uint32_t> ReceiveBuffer::sack_entries() const
    {
        std::vector<uint32_t> entries;
        entries.reserve(std::min(
            static_cast<std::size_t>(rdt::MAX_SACK_ENTRIES),
            buffer_.size()));

        for (const auto &[seq, _] : buffer_)
        {
            if (entries.size() >= rdt::MAX_SACK_ENTRIES) break; // to fit in 1 packet
            entries.push_back(seq);
        }

        return entries;
    }

}