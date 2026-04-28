#include "include/arp.h"
#include "include/console_utils.h"
#include "include/icmp.h"
#include "include/icmpv6.h"
#include "include/ndp.h"
#include "include/network_utils.h"
#include "include/output.h"
#include "include/polling.h"

#include <algorithm>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    // Parsing command-line arguments
    ProgramOptions options = parse_arguments(argc, argv);
    // Print input arguments or default values (useful for debugging)
    // print_input_arguments(options);
    print_scanning_ranges(options.subnets);

    // Get active network interfaces
    vector<NetworkInterface> interfaces = get_active_interfaces();
    // Check if an interface was specified and if it is valid
    if (!options.interface.empty() && !any_of(interfaces.begin(), interfaces.end(),
                                              [&](const NetworkInterface &ni)
                                              { return ni.name == options.interface; }))
    {
        cerr << "Invalid interface. Available interfaces:\n";
        print_interfaces(interfaces);
        return EXIT_FAILURE;
    }

    // split subnets into IPv4 and IPv6 lists
    vector<string> ipv4_hosts, ipv6_hosts;
    // Expand subnets into individual host addresses
    for (const auto &subnet : options.subnets)
    {
        auto hosts = expand_subnet(subnet);
        for (const auto &host : hosts)
        {
            if (host.find(':') == string::npos)
            {
                ipv4_hosts.push_back(host);
            }
            else
            {
                ipv6_hosts.push_back(host);
            }
        }
    }

    // Create sockets
    // IPv4
    int arp_sock = create_arp_socket(options.interface);
    int icmp_sock = create_icmp_socket();
    // IPv6
    int ndp_sock = create_ndp_socket();
    int icmpv6_sock = create_icmpv6_socket();

    // check if sockets were created successfully
    if (icmp_sock < 0 || icmpv6_sock < 0 || ndp_sock < 0 || arp_sock < 0)
    {
        print_socket_error(arp_sock, icmp_sock, ndp_sock, icmpv6_sock);
        return EXIT_FAILURE;
    }

    // configure sockets (non-blocking, TTL/hop-limit)
    fcntl(icmp_sock, F_SETFL, O_NONBLOCK);
    fcntl(icmpv6_sock, F_SETFL, O_NONBLOCK);
    fcntl(ndp_sock, F_SETFL, O_NONBLOCK);

    NetworkInterface selected_interface;

    // Get the information of the selected network interface (MAC, IPv4, IPv6)
    for (const auto &iface : interfaces)
    {
        if (iface.name == options.interface)
        {
            selected_interface = iface;
            break;
        }
    }
    // Vectors to store the requests senf for each protocol
    vector<ARPRequest> arp_requests;
    vector<NDPRequest> ndp_requests;
    vector<ICMPRequest> icmp_requests;

    uint16_t sequence = 1;

    // send ARP requests
    for (const auto &host : ipv4_hosts)
    {
        send_arp_request(arp_sock, host, selected_interface.mac, selected_interface.ipv4, options);
        arp_requests.push_back({host, chrono::steady_clock::now()});
    }
    // send NDP requests
    for (const auto &host : ipv6_hosts)
    {
        send_ndp_request(ndp_sock, host, selected_interface.mac, selected_interface.ipv6, options);
        ndp_requests.push_back({host, chrono::steady_clock::now()});
    }
    // send ICMP requests
    for (const auto &host : ipv4_hosts)
    {
        send_icmp_request(icmp_sock, host, sequence);
        icmp_requests.push_back({host, sequence++, chrono::steady_clock::now(), false});
    }
    // send ICMPv6 requests
    for (const auto &host : ipv6_hosts)
    {
        send_icmpv6_request(icmpv6_sock, host, sequence);
        icmp_requests.push_back({host, sequence++, chrono::steady_clock::now(), true});
    }
    // Main polling loop
    // Wait for replies from the network using the poll_sockets function
    poll_sockets(arp_sock, icmp_sock, icmpv6_sock, ndp_sock, arp_requests, icmp_requests, ndp_requests, options.timeout);

    // Output results
    print_output(ipv4_hosts, ipv6_hosts, arp_requests, icmp_requests, ndp_requests, selected_interface);

    close(arp_sock);
    close(icmp_sock);
    close(icmpv6_sock);
    close(ndp_sock);

    return EXIT_SUCCESS;
}
