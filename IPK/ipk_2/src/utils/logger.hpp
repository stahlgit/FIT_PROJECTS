/**
 * @file logger.hpp
 * @brief Global Logger Instance
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <format>
#include <string>
#include <string_view>
#include <cstdint>

namespace rdt::utils
{

    /**
     * @brief Singleton logger that writes structured messages to stderr.
     *
     * All messages follow the format:
     *   [LEVEL][component] message
     *
     * Usage:
     * 
     * @code
        Logger::instance().info("session", "handshake complete");
        Logger::instance().debug("sender", "ACK={}", n);
    @endcode
    */
    class Logger
    {
    public:
        enum class Level : uint8_t
        {
            DEBUG = 0,
            INFO  = 1,
            WARN  = 2,
            ERROR = 3,
        };


        ///@brief Returns the process-wide logger instance 
        static Logger &instance() noexcept;

        Logger(const Logger &) = delete;
        Logger &operator=(const Logger &) = delete;


        ///@brief Enable or disable all output. Default: enabled
        void set_enabled(bool on) noexcept { enabled_ = on; }

        ///@brief Only emit messages at or above lvl. Default: DEBUG (emit all)
        void set_min_level(Level lvl) noexcept { min_level_ = lvl; }

        [[nodiscard]] bool is_enabled() const noexcept { return enabled_; }
        [[nodiscard]] Level min_level() const noexcept { return min_level_; }


        void debug(std::string_view component, std::string_view msg) const;
        void info (std::string_view component, std::string_view msg) const;
        void warn (std::string_view component, std::string_view msg) const;
        void error(std::string_view component, std::string_view msg) const;

        /// @cite https://stackoverflow.com/questions/66778684/how-to-overload-a-function-taking-variable-number-of-arguments

        template<typename Arg, typename... Args>
        void debug(std::string_view component, std::format_string<Arg, Args...> fmt, Arg&& arg, Args&&... args) const
        {
            emit_(Level::DEBUG, component, std::format(fmt, std::forward<Arg>(arg), std::forward<Args>(args)...));
        }

        template<typename Arg, typename... Args>
        void info(std::string_view component, std::format_string<Arg, Args...> fmt, Arg&& arg, Args&&... args) const
        {
            emit_(Level::INFO, component, std::format(fmt, std::forward<Arg>(arg), std::forward<Args>(args)...));
        }

        template<typename Arg, typename... Args>
        void warn(std::string_view component, std::format_string<Arg, Args...> fmt, Arg&& arg, Args&&... args) const
        {
            emit_(Level::WARN, component, std::format(fmt, std::forward<Arg>(arg), std::forward<Args>(args)...));
        }

        template<typename Arg, typename... Args>
        void error(std::string_view component, std::format_string<Arg, Args...> fmt, Arg&& arg, Args&&... args) const
        {
            emit_(Level::ERROR, component, std::format(fmt, std::forward<Arg>(arg), std::forward<Args>(args)...));
        }

        ///@brief Generic entry point
        void log(Level lvl, std::string_view component, std::string_view msg) const;

        template<typename Arg, typename... Args>
        void log(Level lvl, std::string_view component, std::format_string<Arg, Args...> fmt, Arg&& arg, Args&&... args) const
        {
            emit_(lvl, component, std::format(fmt, std::forward<Arg>(arg), std::forward<Args>(args)...));
        }

    private:
        Logger() = default;

        bool enabled_ = true;
        Level min_level_ = Level::DEBUG;

        void emit_(Level lvl, std::string_view component, std::string_view msg) const;
        static std::string_view level_str_(Level lvl) noexcept;
    };

} // namespace rdt::infra

#endif // LOGGER_HPP
