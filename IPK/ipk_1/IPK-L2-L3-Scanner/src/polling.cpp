#include "include/polling.h"
#include <algorithm>

void poll_sockets(int arp_sock, int icmp_sock, int icmpv6_sock, int ndp_sock,
                  vector<ARPRequest> &arp_requests, vector<ICMPRequest> &icmp_requests,
                  vector<NDPRequest> &ndp_requests, int timeout)
{
    // Set of file descriptors to monitor for readability
    fd_set read_fds;
    // Record the starting time for the timeout calculation
    auto start = chrono::steady_clock::now();

    while (true)
    {
        // Initialize the set of file descriptors to be empty
        FD_ZERO(&read_fds);
        // add each socket to the set
        FD_SET(arp_sock, &read_fds);
        FD_SET(icmp_sock, &read_fds);
        FD_SET(icmpv6_sock, &read_fds);
        FD_SET(ndp_sock, &read_fds);
        // structure to specify the timeout for all the select calls
        timeval tv{};
        // Current time
        auto now = chrono::steady_clock::now();
        int remaining = timeout - chrono::duration_cast<chrono::milliseconds>(now - start).count();
        if (remaining <= 0)
        {
            break; // Exiting loop due to timeout
        }

        tv.tv_sec = remaining / 1000;           // seconds
        tv.tv_usec = (remaining % 1000) * 1000; // microseconds
        // Determine the maximum file descriptor value
        int max_fd = max({arp_sock, icmp_sock, icmpv6_sock, ndp_sock}) + 1;
        // Call the select system call to wait for activity on any of the specified sockets
        int rv = select(max_fd, &read_fds, nullptr, nullptr, &tv);
        if (rv < 0)
        {
            // If an error occurred (e.g., interrupted by a signal), print an error message
            perror("select");
            break;
        }
        else if (rv == 0)
        {
            break; // Timeout
        }

        // Process responses based on which sockets are ready for reading
        if (FD_ISSET(arp_sock, &read_fds))
            process_arp_replies(arp_sock, arp_requests);
        if (FD_ISSET(ndp_sock, &read_fds))
            process_ndp_replies(ndp_sock, ndp_requests);
        if (FD_ISSET(icmp_sock, &read_fds))
            process_icmp_replies(icmp_sock, icmp_requests);
        if (FD_ISSET(icmpv6_sock, &read_fds))
            process_icmpv6_replies(icmpv6_sock, icmp_requests);
    }
}
