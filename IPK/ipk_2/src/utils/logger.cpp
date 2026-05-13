/**
 * @file logger.cpp
 * @brief Implementation of Logger.
 * @author Peter Stahl (xstahl01)
 */

#include "logger.hpp"

#include <iostream>

namespace rdt::utils
{

    Logger &Logger::instance() noexcept
    {
        static Logger inst;
        return inst;
    }

    void Logger::debug(std::string_view component, std::string_view msg) const
    {
        emit_(Level::DEBUG, component, msg);
    }

    void Logger::info(std::string_view component, std::string_view msg) const
    {
        emit_(Level::INFO, component, msg);
    }

    void Logger::warn(std::string_view component, std::string_view msg) const
    {
        emit_(Level::WARN, component, msg);
    }

    void Logger::error(std::string_view component, std::string_view msg) const
    {
        emit_(Level::ERROR, component, msg);
    }

    void Logger::log(Level lvl, std::string_view component, std::string_view msg) const
    {
        emit_(lvl, component, msg);
    }


    void Logger::emit_(Level lvl, std::string_view component, std::string_view msg) const
    {
        if (!enabled_ || static_cast<uint8_t>(lvl) < static_cast<uint8_t>(min_level_))
            return;

        std::cerr << '[' << level_str_(lvl) << "][" << component << "] " << msg << '\n';
    }

    std::string_view Logger::level_str_(Level lvl) noexcept
    {
        switch (lvl)
        {
            using enum rdt::utils::Logger::Level;
            case DEBUG: return "DEBUG";
            case INFO:  return "INFO ";
            case WARN:  return "WARN ";
            case ERROR: return "ERROR";
        }
        return "?????";
    }

} // namespace rdt::infra
