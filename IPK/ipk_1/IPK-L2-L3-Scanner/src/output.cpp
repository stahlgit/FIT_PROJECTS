#include "include/output.h"
#include <algorithm>
//circular import//circular import

/**
 * @brief Helping function to replace all ':' with '-' in a MAC address
 * @param mac MAC address to be corrected
 * @return Corrected MAC address
 */
string correct_mac(const string& mac) {
    string mac_copy = mac; // Create a mutable copy of the input string
    replace(mac_copy.begin(), mac_copy.end(), ':', '-');
    return mac_copy;
}

void print_output(vector<string>& ipv4_hosts, vector<string>& ipv6_hosts, vector<ARPRequest>& arp_requests, 
        vector<ICMPRequest>& icmp_requests, vector<NDPRequest>& ndp_requests, NetworkInterface& iface){
    for (const auto& host : ipv4_hosts) {
        // ARP status
        string arp_status = "FAIL";
        string mac = "";
        
        if (host == iface.ipv4) {
            arp_status = "OK";
            mac = " (" + correct_mac(iface.mac) + ")";
        } else {
            auto arp_it = find_if(arp_requests.begin(), arp_requests.end(),
                [&](const ARPRequest& r) { return r.host == host; });
            
            if (arp_it != arp_requests.end() && arp_it->responded) {
                arp_status = "OK";
                mac = " (" + correct_mac(arp_it->mac) + ")";
            }
        }

        // ICMP status
        string icmp_status = "FAIL";
        auto icmp_it = find_if(icmp_requests.begin(), icmp_requests.end(),
            [&](const ICMPRequest& r) { return r.host == host && !r.is_ipv6; });
        
        if (icmp_it == icmp_requests.end()) {
            icmp_status = "OK";
        }

        cout << host << " arp " << arp_status << mac 
            << ", icmpv4 " << icmp_status << endl;
    }
    for(const auto& host: ipv6_hosts){
        // NDP status
        string ndp_status = "FAIL";
        string mac = "";
        if(host == iface.ipv6){
            ndp_status = "OK";
            mac = " (" + correct_mac(iface.mac) + ")";
        } else {
            auto ndp_it = find_if(ndp_requests.begin(), ndp_requests.end(),
                [&](const NDPRequest& r) { return r.host == host && r.responded; });
            if (ndp_it != ndp_requests.end()) {
                ndp_status = "OK";
                mac = " (" + correct_mac(ndp_it->mac) + ")";
            }
        }
        // ICMPv6 status
        string icmp_status = "FAIL";
        auto icmp_it = find_if(icmp_requests.begin(), icmp_requests.end(),
            [&](const ICMPRequest& r) { return r.host == host && r.is_ipv6; });
        if (icmp_it == icmp_requests.end()) {
            icmp_status = "OK";
        }

        cout << host << " ndp " << ndp_status << mac 
            << ", icmpv6 " << icmp_status << endl;
    }
}