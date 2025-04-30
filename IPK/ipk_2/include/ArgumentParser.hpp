#ifndef ARGUMENTPARSER_HPP
#define ARGUMENTPARSER_HPP

#pragma once

#include <string>
#include <iostream>
#include <cstdint>

/**
 * @brief Structure to hold program options from command line
 */
struct ProgramOptions {
    std::string transport; // "tcp" or "udp"
    std::string server; // IP or host name
    uint16_t port = 4567; // default port
    uint16_t udpTimeoutMs = 250 ; // in miliseconds
    uint8_t maxRetries = 3; // 1 initial + 3 retries
};



class ArgumentParser {
public:
    ArgumentParser(int argc, char* argv[]);
    /**
     * @brief Parse the input arguments
     * @param argc Number of arguments
     * @param argv List of arguments
     * @return ProgramOptions
     */
    ProgramOptions parse();

private:
    int argc;
    char** argv;
    /** @brief prints Help message
     */
    void printHelp() const;
};

#endif //ARGUMENTPARSER_HPP