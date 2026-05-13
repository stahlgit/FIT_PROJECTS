/**
 * @file timer.hpp
 * @brief Lightweight elapsed-time helper built on std::chrono::steady_clock.
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <cstdint>
#include <limits>

namespace rdt::utils
{

    /**
     * @brief Measures elapsed wall-clock milliseconds since construction or last reset().
     *
     * Starts ticking immediately on construction - no separate start() call needed.
     * For "timer that starts later", wrap in std::optional<Timer> and emplace() when ready.
     *
     * All return values are uint32_t milliseconds for consistency with the
     * protocol constants (RETRANSMIT_MS, MAX_RETRANSMIT_MS) defined in rdt_constants.hpp.
     * elapsed_ms() is clamped to UINT32_MAX so it never overflows on very long runs.
     */
    class Timer
    {
    public:
        ///@brief Starts the timer at construction time 
        Timer() noexcept : start_(std::chrono::steady_clock::now()) {}

        ///@brief Restart from now
        void reset() noexcept { start_ = std::chrono::steady_clock::now(); }

        ///@brief Milliseconds elapsed since construction or last reset(). */
        [[nodiscard]] uint32_t elapsed_ms() const noexcept
        {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start_).count();

            // Clamp to uint32_t range - only relevant after ~49 days of continuous uptime.
            if (ms < 0)
                return 0;
            if (ms > std::numeric_limits<uint32_t>::max())
                return std::numeric_limits<uint32_t>::max();
            return static_cast<uint32_t>(ms);
        }

        ///@brief True if elapsed_ms() >= threshold_ms
        [[nodiscard]] bool has_expired(uint32_t threshold_ms) const noexcept
        {
            return elapsed_ms() >= threshold_ms;
        }

    private:
        std::chrono::steady_clock::time_point start_;
    };

} // namespace rdt::infra

#endif // TIMER_HPP
