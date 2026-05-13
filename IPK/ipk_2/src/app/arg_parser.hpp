/**
 * @file arg_parser.hpp
 * @brief Argument parser for command-line options.
 * @author Peter Stahl (xstahl01)
 */

#ifndef ARG_PARSER_HPP
#define ARG_PARSER_HPP

#include "connconfig.hpp"
#include <span>
#include <string_view>

namespace rdt::app
{
    class ArgParser
    {
    public:
        /// @brief Main entry point to parsing arguments 
        [[nodiscard]] static ConnConfig parse(int argc, char *argv[]);

    private:
        /// @brief prints usage
        static void print_usage_(std::string_view prog);

        /// @brief detects AppMode (Client or Server)
        static rdt::AppMode detect_mode_(std::span<char *> args);

        /// @brief switch function which applies CLI arguments to App Configuration
        static void apply_option_(ConnConfig &cfg, int opt, const char *optarg_val);

        /// @brief Check valid arguments for client side
        static void validate_client_(const ConnConfig &cfg);

        /// @brief Check valid arguments for server side
        static void validate_server_(const ConnConfig &cfg);
    };
}

#endif // ARG_PARSER_HPP
