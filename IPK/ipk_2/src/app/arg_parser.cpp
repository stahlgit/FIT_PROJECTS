/**
 * @file arg_parser.cpp
 * @brief Implementation of argument parser
 * @author Peter Stahl (xstahl01)
 */

#include "arg_parser.hpp"
#include "exceptions.hpp"

#include <array>
#include <cstdlib>
#include <getopt.h>
#include <iostream>

namespace rdt::app {

void ArgParser::print_usage_(std::string_view prog)
{
    std::cout << "Usage:\n"
              << "  " << prog << " -s -p PORT [-a ADDRESS] [-o OUTPUT] [-w TIMEOUT] [-h|--help]\n"
              << "  " << prog << " -c -a HOST -p PORT [-i INPUT] [-w TIMEOUT] [-h|--help]\n\n"
              << "Options:\n"
              << "  -s             Server (receiver) mode\n"
              << "  -c             Client (sender) mode\n"
              << "  -p PORT        UDP port number (1-65535)\n"
              << "  -a ADDRESS     Server: local bind address; Client: destination host\n"
              << "  -i INPUT       Input file for client (default: stdin; '-' = stdin)\n"
              << "  -o OUTPUT      Output file for server (default: stdout; '-' = stdout)\n"
              << "  -w TIMEOUT     Max seconds without protocol progress (default: 1)\n"
              << "  -h, --help     Show this help and exit\n";
}

rdt::AppMode ArgParser::detect_mode_(std::span<char *> args)
{
    bool has_s = false;
    bool has_c = false;
    for (std::size_t i = 1; i < args.size(); i++) {
        std::string_view arg(args[i]);
        if (arg == "-s")      has_s = true;
        else if (arg == "-c") has_c = true;
    }
    if (has_s && has_c)
        throw rdt::ArgException("Cannot specify both -s and -c.");
    if (!has_s && !has_c)
        throw rdt::ArgException("Must specify either -s (server) or -c (client).");
    return has_s ? rdt::AppMode::SERVER : rdt::AppMode::CLIENT;
}

void ArgParser::apply_option_(ConnConfig &cfg, int opt, const char *optarg_val)
{
    switch (opt) {
        case 'p': {
            int p = std::atoi(optarg_val);
            if (p < ConnConfig::MIN_PORT || p > ConnConfig::MAX_PORT)
                throw rdt::ArgException("Port must be between 1 and 65535.");
            cfg.port = static_cast<uint16_t>(p);
            break;
        }
        case 'a':
            if (cfg.mode == rdt::AppMode::CLIENT)
                cfg.host = optarg_val;
            else
                cfg.bind_adress = optarg_val;
            break;
        case 'i':
            if (cfg.mode == rdt::AppMode::SERVER)
                throw rdt::ArgException("-i is not valid in server mode.");
            cfg.input_path = optarg_val;
            break;
        case 'o':
            if (cfg.mode == rdt::AppMode::CLIENT)
                throw rdt::ArgException("-o is not valid in client mode.");
            cfg.output_path = optarg_val;
            break;
        case 'w': {
            int t = std::atoi(optarg_val);
            if (t <= 0)
                throw rdt::ArgException("Timeout must be a positive integer.");
            cfg.timeout_s = static_cast<uint32_t>(t);
            break;
        }
        default:
            throw rdt::ArgException("Unknown argument. Use -h for help.");
    }
}

void ArgParser::validate_client_(const ConnConfig &cfg)
{
    if (cfg.host.empty())
        throw rdt::ArgException("Client mode requires -a HOST.");
    if (cfg.port == 0)
        throw rdt::ArgException("Client mode requires -p PORT.");
}

void ArgParser::validate_server_(const ConnConfig &cfg)
{
    if (cfg.port == 0)
        throw rdt::ArgException("Server mode requires -p PORT.");
}

ConnConfig ArgParser::parse(int argc, char *argv[])
{
    // Handle -h/--help before mode detection so it works without a mode flag
    for (int i = 1; i < argc; i++) {
        std::string_view arg(argv[i]);
        if (arg == "-h" || arg == "--help") {
            print_usage_(argv[0]);
            std::exit(static_cast<int>(rdt::ExitCode::OK));
        }
    }

    ConnConfig config;
    config.mode = detect_mode_(std::span<char *>(argv, static_cast<std::size_t>(argc)));

    static const std::array<struct option, 2> long_opts = {{
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    }};

    optind = 1;
    int opt;
    while ((opt = getopt_long(argc, argv, "hscp:a:i:o:w:", long_opts.data(), nullptr)) != -1) {
        if (opt == 'h') {
            print_usage_(argv[0]);
            std::exit(static_cast<int>(rdt::ExitCode::OK));
        }
        if (opt == 's' || opt == 'c') continue; // already consumed by detect_mode_
        apply_option_(config, opt, optarg);
    }

    if (optind < argc)
        throw rdt::ArgException(std::string("Unexpected argument: ") + argv[optind]);

    if (config.mode == rdt::AppMode::CLIENT)
        validate_client_(config);
    else
        validate_server_(config);

    // Normalize '-' to empty string (means stdin/stdout)
    if (config.input_path == "-")  config.input_path.clear();
    if (config.output_path == "-") config.output_path.clear();

    return config;
}

} // namespace rdt::app
