#include "dns/dns_resolver.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./dns_test <hostname>\n";
        return 1;
    }

    DnsResolver resolver;
    try {
        auto targets = resolver.resolve(argv[1]);
        std::cout << "Resolved " << targets.size() << " target(s) for '"<< argv[1] << "':\n";
        for (const auto& t : targets) {
            std::cout << "  " << t.ip_str << "  (family: " << (t.family == AF_INET ? "IPv4" : "IPv6") << ")\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Resolution failed: " << e.what() << "\n";
        return 1;
    }
    return 0;
}