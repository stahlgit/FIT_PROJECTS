/**
 * @file config.hpp
 * @brief Hold configuration settings for the project.
 * @author Peter Stahl (xstahl01)
 */
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <cstdint>

/**
 * @brief Configuration structure to hold settings parsed from command-line arguments.
 */
struct Config{
    static constexpr uint16_t MIN_PORT = 1;
    static constexpr uint16_t MAX_PORT = 65535;

    std::string interface;
    std::string host;
    std::vector<uint16_t> tcp_ports;
    std::vector<uint16_t> udp_ports;
    int timeout_ms = 1000;
    bool show_help = false;
    bool show_interfaces = false;

};

#endif // CONFIG_HPP