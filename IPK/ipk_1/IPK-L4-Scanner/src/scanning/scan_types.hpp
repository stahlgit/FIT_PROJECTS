/**
 * @file scan_types.hpp
 * @brief Common types and enums for port scanning tasks and results.
 * @author Peter Stahl (xstahl01)
 */
#pragma once

#include <cstdint>
#include <string>

#include "dns_resolver.hpp"

enum class Protocol { TCP, UDP };

enum class PortState { OPEN, CLOSED, FILTERED };

struct ScanTask {
    ResolvedTarget target;
    uint16_t port;
    Protocol protocol;
};

struct ScanResult {
    std::string ip_str;
    uint16_t port;
    Protocol protocol;
    PortState state;
};

inline const char* proto_str(Protocol p) {
    return p == Protocol::TCP ? "tcp" : "udp";
}

inline const char* state_str(PortState s) {
    switch (s) {
        using enum PortState;
        case OPEN:     return "open";
        case CLOSED:   return "closed";
        case FILTERED: return "filtered";
    }
    return "unknown";
}