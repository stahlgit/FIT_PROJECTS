/**
 * @file arg_parser.cpp
 * @brief Implementation of the argument parser for command-line options.
 * @author Peter Stahl (xstahl01)
 * adapted from previous coursework https://github.com/stahlgit/FIT_PROJECTS
 */

#include "arg_parser.hpp"
#include "exceptions.hpp"

#include <getopt.h>
#include <iostream>
#include <stdexcept>
#include <sstream>


Config ArgParser::parse_arguments(int argc, char* argv[]) {
    Config config;

    // special case: only -i argument, return early 
    if (argc == 2 && std::string(argv[1]) == "-i") {
        config.show_interfaces = true;
        return config;
    }

    static struct option long_opts[] = {
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    };

    int opt;

    optind = 1; 

    while ((opt = getopt_long(argc, argv, "hi:t:u:w:", long_opts, nullptr)) != -1) {
        switch (opt) {
            case 'h':
                config.show_help = true;
                return config;          // no need to parse further
            case 'i':
                config.interface = optarg;
                break;
            case 't':
                config.tcp_ports = parse_ports_(optarg);
                break;
            case 'u':
                config.udp_ports = parse_ports_(optarg);
                break;
            case 'w':
                config.timeout_ms = std::stoi(optarg);
                break;
            default:
                throw std::invalid_argument("Unknown argument. Use -h for help.");
        }
    }

    for (int i = optind; i < argc; ++i) {
    if (config.host.empty()) {
        config.host = argv[i];
    } else {
        throw std::invalid_argument(
            std::string("Unexpected argument: ") + argv[i]);
    }
}

    return config;
}

void ArgParser::print_help(const char* prog_name) const {
    std::cout << "Usage:\n" 
        << "  " << prog_name << " -i INTERFACE [-t PORTS] [-u PORTS] HOST [-w TIMEOUT]\n"
        << "  " << prog_name << " -i\n"
        << "  " << prog_name << " -h | --help\n\n"
        << "Options:\n" 
        << "  -i INTERFACE   Network interface to use\n"
        << "  -t PORTS       TCP ports (e.g. 22,80 or 1-1024)\n"
        << "  -u PORTS       UDP ports (e.g. 53,67 or 1-1024)\n"
        << "  -w TIMEOUT     Timeout in milliseconds (default: 1000)\n"
        << "  -h, --help     Show this help message\n";
}

// PRIVATE METHODS

std::vector<uint16_t> ArgParser::parse_ports_(const std::string& s) {
    std::vector<uint16_t> ports;
    std::stringstream ss(s);
    std::string token;

    auto validate_port = [](int p) {
        if (p < Config::MIN_PORT || p > Config::MAX_PORT) {
            throw std::invalid_argument("Port out of range (" + 
                                        std::to_string(Config::MIN_PORT) + "-" + 
                                        std::to_string(Config::MAX_PORT) + "): " + 
                                        std::to_string(p));
        }
    };

    while (std::getline(ss, token, ',')) {
        auto dash = token.find('-');
        if (dash != std::string::npos) {
            int lo = std::stoi(token.substr(0, dash));
            int hi = std::stoi(token.substr(dash + 1));
            
            validate_port(lo);
            validate_port(hi);

            if (lo > hi) {
                throw std::invalid_argument("Invalid port range (start > end): " + token);
            }

            for (int p = lo; p <= hi; ++p) ports.push_back(static_cast<uint16_t>(p));
        } else {
            int port = std::stoi(token);
            validate_port(port);
            ports.push_back(static_cast<uint16_t>(port));
        }
    }
    return ports;
}
