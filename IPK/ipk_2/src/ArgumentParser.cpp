#include "ArgumentParser.hpp"
#include <getopt.h>
#include <string>
#include <iostream>
#include <cstdint>
#include <stdexcept>

ArgumentParser::ArgumentParser(int argc, char* argv[])
    : argc(argc), argv(argv) {}

ProgramOptions ArgumentParser::parse() {
    optind = 1;  // Reset getopt global state
    ProgramOptions opts;

    const struct option long_options[] = {
        {"transport", required_argument, nullptr, 't'},
        {"server", required_argument, nullptr, 's'},
        {"port", required_argument, nullptr, 'p'},
        {"udp-timeout", required_argument, nullptr, 'd'},
        {"retries", required_argument, nullptr, 'r'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "t:s:p:d:r:h", long_options, nullptr)) != -1) {
        switch (opt) {
        case 't':
            opts.transport = optarg;
            break;
        case 's':
            opts.server = optarg;
            break;
        case 'p':
            opts.port = static_cast<uint16_t>(std::stoi(optarg));
            break;
        case 'd':
            opts.udpTimeoutMs = static_cast<uint16_t>(std::stoi(optarg));
            break;
        case 'r':
            opts.maxRetries = static_cast<uint8_t>(std::stoi(optarg));
            break;
        case 'h':
            printHelp();
            std::exit(EXIT_SUCCESS);
        default:
            throw std::invalid_argument("Invalid command line argument. Use -h for help.");
        }
    }

    if (opts.transport.empty() || opts.server.empty()) {
        throw std::invalid_argument("Missing required arguments: -t transport, -s server are mandatory!");
    }

    if (opts.transport != "tcp" && opts.transport != "udp") {
        throw std::invalid_argument("Transport must be 'tcp' or 'udp'.");
    }

    if (opts.port == 0 || opts.port > 65535) {
        throw std::invalid_argument("Port must be between 1 and 65535.");
    }

    return opts;
}

void ArgumentParser::printHelp() const {
    std::cout << "Usage:\n"
            << argv[0] << " -t <tcp|udp> -s <server> [-p <port>] [-d <udp-timeout-ms>] [-r <retries>]\n"
            << "\n"
            << "Options:\n"
            << "  -t, --transport      Transport protocol ('tcp' or 'udp')\n"
            << "  -s, --server         Server address or hostname\n"
            << "  -p, --port           Server port (default 4567)\n"
            << "  -d, --udp-timeout    UDP confirm timeout in ms (default 250)\n"
            << "  -r, --retries        UDP max retries (default 3)\n"
            << "  -h, --help           Show this help message\n";
}