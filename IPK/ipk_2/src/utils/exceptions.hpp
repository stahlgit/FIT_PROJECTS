/**
 * @file exceptions.hpp
 * @brief Custom exception classes for location specification of error
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <stdexcept>
#include <system_error>
#include "rdt_constants.hpp"

/**
 * inspiration for using system_error: https://stackoverflow.com/questions/3320898/c-alternative-to-perror
 */

namespace rdt
{
    class RdtException : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
        
        // Force all child exceptions to provide an exit code
        [[nodiscard]] virtual ExitCode exit_code() const noexcept = 0;
    };

    class NetworkException : public RdtException
    {
    public:
        // For syscall failures where errno is meaningful. Uses system_category for thread safety.
        NetworkException(const std::string &what, int err_code)
            : RdtException(what + ": " + std::system_category().message(err_code)), err_code_(err_code) {}

        // For higher-level failures where there's no errno
        explicit NetworkException(const std::string &what)
            : RdtException(what), err_code_(0) {}

        [[nodiscard]] int err_code() const noexcept { return err_code_; }
        
        [[nodiscard]] ExitCode exit_code() const noexcept override { 
            return ExitCode::ERR_NETWORK; 
        }

    private:
        int err_code_;
    };

    class DnsException : public RdtException
    {
    public:
        // DNS errors use gai_strerror, which you will format before passing here
        using RdtException::RdtException;

        [[nodiscard]] ExitCode exit_code() const noexcept override { 
            return ExitCode::ERR_DNS; 
        }
    };

    class IoException : public RdtException
    {
    public:
        // For syscall failures (open, read, write) where errno is meaningful.
        IoException(const std::string &what, int err_code)
            : RdtException(what + ": " + std::system_category().message(err_code)), err_code_(err_code) {}

        // For higher-level I/O failures without an errno (e.g., stream state).
        explicit IoException(const std::string &what)
            : RdtException(what), err_code_(0) {}

        [[nodiscard]] int err_code() const noexcept { return err_code_; }

        [[nodiscard]] ExitCode exit_code() const noexcept override
        {
            return ExitCode::ERR_IO;
        }

    private:
        int err_code_;
    };

    class ArgException : public RdtException
    {
    public:
        using RdtException::RdtException;

        [[nodiscard]] ExitCode exit_code() const noexcept override
        {
            return ExitCode::ERR_ARGS;
        }
    };

} // namespace rdt

#endif // EXCEPTIONS_HPP