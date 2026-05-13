/**
 * @file signal_handler.hpp
 * @brief RAII installer for SIGINT / SIGTERM → std::atomic<bool> stop flag.
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef SIGNAL_HANDLER_HPP
#define SIGNAL_HANDLER_HPP

#include <atomic>
#include <csignal>

namespace rdt::infra
{

    /**
     * @brief Sets SIGINT and SIGTERM handlers on construction, restores the previous dispositions on destruction.
     *
     * SA_RESTART is intentionally NOT set so that the signal delivery interrupts poll() with EINTR, 
     * letting the event loop see the stop flag on the very next iteration
     */
    class SignalHandler
    {
    public:
        /**
         * @brief Install SIGINT and SIGTERM handlers that set stop = true
         * @throws std::system_error if sigaction() fails
         */
        explicit SignalHandler(std::atomic<bool> &stop);

        /** @brief Restore previous signal dispositions. */
        ~SignalHandler();

        SignalHandler(const SignalHandler &) = delete;
        SignalHandler &operator=(const SignalHandler &) = delete;
        SignalHandler(SignalHandler &&) = delete;
        SignalHandler &operator=(SignalHandler &&) = delete;

    private:
        struct sigaction old_sigint_{};
        struct sigaction old_sigterm_{};

        // Process-wide pointer - only one SignalHandler instance may exist at time
        static std::atomic<bool> *stop_ptr_;

        // Async-signal-safe: single relaxed store on bool-sized atomic
        static void on_signal_(int sig) noexcept;
    };

} // namespace rdt::infra

#endif // SIGNAL_HANDLER_HPP
