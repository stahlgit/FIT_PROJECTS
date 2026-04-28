/**
 * @file dns_resolver.hpp
 * @brief Header file for DNS resolver functionality.
 * @author Peter Stahl (xstahl01)
 * adapted from previous coursework https://github.com/stahlgit/FIT_PROJECTS
 */

#ifndef DNS_RESOLVER_HPP
#define DNS_RESOLVER_HPP

#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>

struct ResolvedTarget {
    std::string ip_str;
    sockaddr_storage addr;
    socklen_t addr_len;
    int family;
};

class DnsResolver{
public:
    /**
     * @brief Resolves hostname or IP literal to one or more targets.
     * @param host Hostname or IPv4/IPv6 adress string.
     * @return List of resolved targets
     * @throws std::runtime_error if resolution fails
     */
    std::vector<ResolvedTarget> resolve(const std::string& host) const;
};

#endif // DNS_RESOLVER_HPP