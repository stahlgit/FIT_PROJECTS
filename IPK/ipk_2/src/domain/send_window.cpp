/**
 * @file send_window.cpp
 * @brief SendWindow implementation
 * @author Peter Stahl (xstahl01)
 */
#include <cassert>
#include <format>

#include "send_window.hpp"
#include "logger.hpp"

using Log = rdt::utils::Logger;

namespace rdt::domain
{
    void SendWindow::push(uint32_t seq, std::vector<uint8_t> data)
    {
        assert(!is_full() && "push() called on full window - check is_full() first");
        entries_.try_emplace(seq, std::move(data)); // try_emplaces constructs SegmentEntry
        next_seq_ = seq + 1;
        // WARN: assumes pushing in order - which it should be, but what if 
    }

    uint32_t SendWindow::on_ack(uint32_t ack_num)
    {
        if (ack_num <= base_)
            return 0; //

        uint32_t newly_acked = 0;

        // ease all entries with seq_num < ack_num
        auto it = entries_.begin();
        while (it != entries_.end() && it->first < ack_num)
        {
            ++newly_acked;
            it = entries_.erase(it);
        }

        base_ = ack_num;

        if (newly_acked > 0)
        {
            Log::instance().debug("send_window", "ACK {} - {} segment(s) cleared, base={}, in_flight={}", ack_num, newly_acked, base(), entries_.size());
        }
        return newly_acked;
    }

    bool SendWindow::on_sack(uint32_t seq)
    {
        auto it = entries_.find(seq);
        if (it == entries_.end())
            return false; // already acked or never send
        if (it->second.sack_marked)
            return false; // duplicate SACK
        it->second.sack_marked = true;
        Log::instance().debug("send_window","SACK marked seq={}", seq);
        return true;
    }

    std::vector<std::pair<uint32_t, std::vector<uint8_t>>> SendWindow::retransmit_candidates()
    {
        std::vector<std::pair<uint32_t, std::vector<uint8_t>>> result;

        for (auto &[seq, entry] : entries_)
        {
            if (entry.sack_marked) continue;

            if (entry.sent_timer.has_expired(entry.retransmit_ms))
            {
                result.emplace_back(seq, entry.data);
                entry.sent_timer.reset();
                entry.retransmit_ms = std::min(
                    entry.retransmit_ms * rdt::BACKOFF_MULTIPLIER,
                    rdt::MAX_RETRANSMIT_MS);
                Log::instance().debug("send_window", std::format("retransmit seq={}, next_interval={} ms ", seq, entry.retransmit_ms));
            }
        }

        return result;
    }

    std::optional<std::pair<uint32_t, std::vector<uint8_t>>> SendWindow::next_retransmit()
    {
        for (auto &[seq, entry] : entries_)
        {
            if (entry.sack_marked) continue;
            if (entry.sent_timer.has_expired(entry.retransmit_ms))
            {
                entry.sent_timer.reset();
                entry.retransmit_ms = std::min(entry.retransmit_ms * rdt::BACKOFF_MULTIPLIER, rdt::MAX_RETRANSMIT_MS);
                Log::instance().debug("send_window", "retransmit seq={}, next_interval={} ms", seq, entry.retransmit_ms);
                return {{seq, entry.data}};
            }
        }
        return std::nullopt;
    }
}