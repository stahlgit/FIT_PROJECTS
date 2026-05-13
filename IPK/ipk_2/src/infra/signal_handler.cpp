/**
 * @file signal_handler.cpp
 * @brief Implementation of SignalHandler.
 * @author Peter Stahl (xstahl01)
 */

#include "signal_handler.hpp"
#include "exceptions.hpp"

#include <cerrno>
#include <system_error>

namespace rdt::infra
{

    // Definition of the static member.
    std::atomic<bool> *SignalHandler::stop_ptr_ = nullptr;

    SignalHandler::SignalHandler(std::atomic<bool> &stop)
    {
        stop_ptr_ = &stop;

        struct sigaction sa{};
        sa.sa_handler = on_signal_;
        ::sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0; // no SA_RESTART - want EINTR to wake poll() immediately

        if (::sigaction(SIGINT, &sa, &old_sigint_) != 0)
        {
            stop_ptr_ = nullptr;
            throw std::system_error(errno, std::system_category(), "sigaction SIGINT");
        }

        if (::sigaction(SIGTERM, &sa, &old_sigterm_) != 0)
        {
            // Roll back SIGINT before throwing.
            ::sigaction(SIGINT, &old_sigint_, nullptr);
            stop_ptr_ = nullptr;
            throw std::system_error(errno, std::system_category(), "sigaction SIGTERM");
        }
    }

    SignalHandler::~SignalHandler()
    {
        ::sigaction(SIGINT,  &old_sigint_,  nullptr);
        ::sigaction(SIGTERM, &old_sigterm_, nullptr);
        stop_ptr_ = nullptr;
    }

    void SignalHandler::on_signal_(int /*sig*/) noexcept
    {
        if (stop_ptr_ != nullptr)
            stop_ptr_->store(true, std::memory_order_relaxed);
    }

} // namespace rdt::infra
