/**
 * @file dns_resolver.cpp
 * @brief Implementation of the DNS resolver functionality.
 * @author Peter Stahl (xstahl01)
 * adapted from previous coursework https://github.com/stahlgit/FIT_PROJECTS
 */

#include <netdb.h>
#include <stdexcept>
#include <cstring>
#include <arpa/inet.h>

#include "dns_resolver.hpp"
#include "exceptions.hpp"

std::vector<ResolvedTarget> DnsResolver::resolve(const std::string& host) const{
    // Set up hints for getaddrinfo
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_flags = 0; 

    // Perform DNS resolution
    addrinfo* results = nullptr;
    if(int rc = getaddrinfo(host.c_str(), nullptr, &hints, &results); rc != 0){
        throw DnsError("DNS resolution failed for " + host + ": " + gai_strerror(rc));
    }

    // Process results
    std::vector<ResolvedTarget> targets;
    for(addrinfo* ai = results; ai != nullptr; ai = ai->ai_next){
        // skip unsupported families
        if(ai->ai_family != AF_INET && ai->ai_family != AF_INET6){
            continue; 
        }

        ResolvedTarget target;
        target.family = ai->ai_family;
        target.addr_len = ai->ai_addrlen;
        std::memcpy(&target.addr, ai->ai_addr, ai->ai_addrlen);

        char buffer[INET6_ADDRSTRLEN];
        const void* src = (ai->ai_family == AF_INET) 
            ? static_cast<const void*>(&reinterpret_cast<sockaddr_in*>(ai->ai_addr)->sin_addr) 
            : static_cast<const void*>(&reinterpret_cast<sockaddr_in6*>(ai->ai_addr)->sin6_addr);

        if (inet_ntop(ai->ai_family, src, buffer, sizeof(buffer))){
            target.ip_str = buffer;
        }
        targets.push_back(target);
    }

    // Free the results linked list
    freeaddrinfo(results);

    if (targets.empty()){
        throw DnsError("No usable addresses found for '" + host + "'");
    }
    return targets;
}