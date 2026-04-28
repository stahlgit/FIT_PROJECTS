# Project 1 - DELTA: L2/L3 Scanner
## 1. Introduction
This document provides a manual for the ipk-l2l3-scanner, a network discovery utility developed in C++ using raw socket programming. This tool performs essential network scanning tasks, including ARP resolution, Neighbor Discovery (NDP), and ICMP pinging for both IPv4 and IPv6 networks.
## 2.  Executive Summary of Theory
This scanner leverages fundamental networking protocols to discover hosts on a local network.
- **ARP (Address Resolution Protocol):** (RFC 826) Operates at the Data Link Layer (Layer 2) and maps IPv4 addresses to MAC addresses. It broadcasts ARP requests to the local network and receives ARP replies from the target hosts.
- **NDP (Neighbor Discovery Protocol):** (RFC 4861) Functions similarly to ARP for IPv6, providing address resolution, router discovery, and other neighbor-related functions. It uses Neighbor Solicitation and Neighbor Advertisement messages.
- **ICMP (Internet Control Message Protocol):** (RFC 792, RFC 4443) Works at the Network Layer (Layer 3) and is used for network diagnostics. The echo request and echo reply messages are used to check host reachability and measure round-trip time (RTT).

Raw sockets are crucial for this application, allowing direct manipulation of network packets at the link and network layers, bypassing the operating system's protocol stack.

## 3. Usage 
#### 3.1 Workflow
1. ```make```
2. ```(sudo) ./ipk-l2l3-scan [-i interface | --interface interface ] {-w timeout | --wait timeout} [-s ipv4-subnet | -s ipv6-subnet | --subnet ipv4-subnet | --subnet ipv6-subnet]```

#### 3.2 Command-Line Options
- ```-i interface | --interface interface```: Specifies the network interface to use (e.g., eth0, wlan0).
- ```-w timeout | --wait timeout```: Sets the timeout in milliseconds (default: 5000ms).
- ```-s subnet | --subnet subnet```: Specifies the IPv4 or IPv6 subnet to scan (e.g., 192.168.1.0/24, fd00::/64). Multiple subnets can be specified using multiple -s options.

## 4. Program Flow
#### 4.1 Argument parsing 
The program parses the command-line arguments to determine the interface, target subnets/hosts and timeout (which is defaultly set to 5000 ms)

#### 4.2 Interface Validation
The program validates the user-specified interface to ensure it exists using system calls.  This prevents errors that could occur if the program attempts to use a non-existent interface.

#### 4.3 Host Expansion
The program expands the provided subnets into individual IP addresses. This allows the program to scan a range of hosts efficiently. The classification of IP addresses as IPv4 or IPv6 is based on the address format, following the standards defined in RFC 791 (IPv4) and RFC 8200 (IPv6).

#### 4.4 Socket Creation
Creates raw sockets for ARP, IPv4 ICMP, IPv6 NDP, and IPv6 ICMP. Each packet is associated with specific protocol family. 

#### 4.5 Socket Configuration
There are used non-blocking sockets allow programs to perform I/O operations without waiting for data to become available. This is crucial for efficient polling using select.The program sets the ICMP and NDP sockets to non-blocking mode using fcntl. This allows the program to monitor multiple sockets simultaneously without blocking.

#### 4.6 Request Sending
Sends ARP requests for IPv4 hosts, NDP requests for IPv6 hosts, and ICMP echo requests for both. These requests are constructed using raw sockets, allowing the program to control the packet headers and payloads.
Structure of each packet is defined by their associated standards:
- ARP (Address Resolution Protocol): (RFC 826) Maps IPv4 addresses to MAC addresses on a local network.
- NDP (Neighbor Discovery Protocol): (RFC 4861) Performs similar functions as ARP for IPv6, including address resolution and router discovery.
- ICMP (Internet Control Message Protocol): (RFC 792, RFC 4443) Used for error reporting and network diagnostics, including the echo request and echo reply messages used in ping.


#### 4.7 Polling
The select system call is used to monitor multiple file descriptors for readiness. It allows the program to wait for incoming data on any of the sockets without blocking, what is an efficient way to handle multiple sockets concurrently.


#### 4.8 Response Processing
The program parses the received packets to extract the relevant information. ARP replies contain the MAC address of the target host, NDP replies contain the link-layer address, and ICMP echo replies indicate reachability and provide the round-trip time (RTT).


#### 4.9 Result Output
Prints the final results, indicating which hosts responded and their associated information.

#### 4.10 Socket Closing
 Closes all the created sockets.

## 5. UML Diagram

```Mermaid
sequenceDiagram
    participant UserInput
    participant Main
    participant parse_argument()
    participant get_active_interface()
    participant expand_subnet()
    participant ARPsocket
    participant ICMPsocket
    participant NDPsocket
    participant ICMPv6socket

    UserInput->>Main: arguments
    Main->>parse_argument(): arguments
    parse_argument()->>Main: ProgramOptions

    get_active_interface()->>Main: vector<NetworkInterface> (checking interface)
    Main->>expand_subnet(): ProgramOptions.subnets
    expand_subnet()->>Main: vector<string> of all IP addresses

    ARPsocket->>Main:create_arp_socket()
    ICMPsocket->>Main:create_icmp_socket()
    NDPsocket->>Main:create_ndp_socket()
    ICMPv6socket->>Main:create_icmpv6_socket()
    Main->>Main:configure non-blocking for each socket

    Main->>ARPsocket:send_arp_requests()
    Main->>NDPsocket:send_ndp_requests()
    Main->>ICMPsocket:send_icmp_requests()
    Main->>ICMPv6socket:send_icmpv6_requests()

    ARPsocket->>Main:poll_sockets():process_arp_replies()
    NDPsocket->>Main:poll_sockets():process_ndp_replis()
    ICMPsocket->>Main:poll_sockets():process_icmp_replies()
    ICMPv6socket->>Main:poll_sockets():process_icmpv6_replies()

    Main->>Output:print_output()

    Main->>Main:close() each socket

```

## 6. Testing 
Testing of this program was bit unconventional since I didn't want to ping some IP ranges that are not mine. I came up with solution to use Docker Networking. I created relatively simple network with couple sleeping containers and one which was running my program. 
This was very beneficial for me since i could permanently set addresses of each container. I was not pinging anyone and I could look into this communication of the network using Wireshark. 
#### 6.1 Testing Methodology:
- **Docker Compose Setup:** A Docker Compose file was created to define a test network with multiple Alpine containers.
- **Container Configuration:** Each container was assigned a static IPv4 and IPv6 address, enabling predictable testing.
- **Scanner Execution:** The ipk-l2l3-scanner was executed within a dedicated container, targeting the IP addresses of the other containers.
- **Wireshark Monitoring:** Wireshark was used to capture and analyze the network traffic, verifying the correctness of ARP, NDP, and ICMP communications.

## 7. Bibliography
Plummer, D. C. "An Ethernet Address Resolution Protocol." RFC 826, November 1982. [Online]. Available: doi:10.17487/RFC0826

Narten, T., Nordmark, E., Simpson, W., and H. Soliman. "Neighbor Discovery for IP version 6 (IPv6)." RFC 4861, September 2007. [Online]. Available: doi:10.17487/RFC4861

Postel, J. "Internet Control Message Protocol." RFC 792, September 1981. [Online]. Available: doi:10.17487/RFC0792

Conta, A., and S. Deering. "Internet Control Message Protocol (ICMPv6) for the Internet Protocol Version 6 (IPv6) Specification." RFC 4443, March 2006. [Online]. Available: doi:10.17487/RFC4443

IBM Corporation. "select() — z/OS Communications Server: IP Sockets Application Programming Interface Guide and Reference." IBM Documentation, 2021. [Online]. Available: https://www.ibm.com/docs/en/zos/2.5.0?topic=calls-select






















































<span style="color: #f0f0f0; font-size: 1px;">but actually I've read it all on wikipedia</span>