/**
 * @file arg_parser.hpp
 * @brief Argument parser for command-line options.
 * @author Peter Stahl (xstahl01)
 */

#ifndef ARG_PARSER_HPP
#define ARG_PARSER_HPP

#include "config.hpp"

class ArgParser {
public:
    /**
     * @brief Parse command-line arguments and return a Config object.
     * @param argc Argument count.
     * @param argv Argument vector.
     * @return Config object populated with parsed settings.
     * @throws std::invalid_argument if invalid argument is encountered.
     */
    Config parse_arguments(int argc, char* argv[]);

    /**
     * @brief Print help message to the console.
     * @param prog_name Program name.
     */
    void print_help(const char* prog_name) const;
private:
    /**
     * @brief Parse a comma-separated list of ports from a string.
     * @param s Input string containing comma-separated ports.
     * @return Vector of parsed port numbers.
     */
    static std::vector<uint16_t> parse_ports_(const std::string& s);
};

#endif // ARG_PARSER_HPP