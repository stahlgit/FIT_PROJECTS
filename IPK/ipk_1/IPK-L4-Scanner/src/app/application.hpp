/**
 * @file application.hpp
 * @brief Application Entry point class definition
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#include "config.hpp"
#include "dns_resolver.hpp"
#include <vector>
#include <atomic>

class Application
{
public:
    /**
     * @brief Entry point - parses args, validates, resolves and runs the application
     * @param argc Argument count
     * @param argv Argument vector
     * @return Exit code (0 for success, non-zero for errors)
     */
    int run(int argc, char *argv[]);

    /**
     * @brief Gracefully shuts down the application, ensuring all resources are cleaned up
     */
    void shutdown();

private:
    Config config_;
    std::vector<ResolvedTarget> targets_;

    std::atomic<bool> running_{true};

    /**
     * @brief Handles special modes like --help and --list-interfaces, which may exit early
     */
    void handle_special_modes() const;

    /**
     * @brief Validates the configuration for logical consistency and required parameters
     */
    void validate_config() const;

    /**
     * @brief Resolves target hostnames to IP addresses and prepares the list of targets to scan
     */
    void resolve_targets();

    /**
     * @brief Schedules and runs the scans based on the resolved targets and configuration
     */
    void run_scans();
};
