/**
 * @file application.cpp
 * @brief Implementation of the Application class
 * @author Peter Stahl (xstahl01)
 */
#include <stdexcept>
#include <ifaddrs.h>
#include <unordered_set>

#include "application.hpp"
#include "arg_parser.hpp"
#include "scan_scheduler.hpp"
#include "exceptions.hpp"
#include <iostream>



int Application::run(int argc, char* argv[]) {
    try {
        ArgParser parser;
        config_ = parser.parse_arguments(argc, argv);

        handle_special_modes();  // may exit(0) early
        validate_config();
        resolve_targets();
        run_scans();

    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << "\n";
        ArgParser{}.print_help(argv[0]);
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void Application::shutdown() {
    running_ = false;
}

void Application::handle_special_modes() const {
    if(config_.show_help) {
        ArgParser{}.print_help("ipk-L4-scan");
        std::exit(EXIT_SUCCESS);
    }
    
    if (config_.show_interfaces) {
        ifaddrs* ifas = nullptr;
        if (getifaddrs(&ifas) == -1)
            throw InterfaceError("Could not list network interfaces");

        std::unordered_set<std::string> names;
        for (ifaddrs* ifa = ifas; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_name && ifa->ifa_addr) {
                names.emplace(ifa->ifa_name);
            }
        }
        for (const auto& name : names) {
            std::cout << name << "\n";
        }

        freeifaddrs(ifas);
        std::exit(EXIT_SUCCESS);
    }
}

void Application::validate_config() const {
    if (config_.host.empty())
        throw std::invalid_argument("No host specified");
    if (config_.interface.empty())
        throw std::invalid_argument("No interface specified (-i)");
    if (config_.tcp_ports.empty() && config_.udp_ports.empty())
        throw std::invalid_argument("No ports specified (-t or -u)");
}

void Application::resolve_targets() {
    DnsResolver resolver;
    targets_ = resolver.resolve(config_.host);
}

void Application::run_scans() {
    ScanScheduler scheduler(running_, config_.timeout_ms, config_.interface);

    for (const auto& target : targets_) {
        for (uint16_t port : config_.tcp_ports)
            scheduler.enqueue({target, port, Protocol::TCP});
        for (uint16_t port : config_.udp_ports)
            scheduler.enqueue({target, port, Protocol::UDP});
    }

    scheduler.wait();
}