import socket
import threading
import time
import sys

# Define ports to open for testing
TCP_OPEN_PORT = 8080
UDP_OPEN_PORT = 9090

def run_tcp_server(port):
    """Binds to a TCP port and listens to simulate an 'open' TCP port."""
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(('0.0.0.0', port))
        s.listen(5)
        print(f"[+] TCP Server listening on port {port} (State: OPEN)")
        while True:
            conn, addr = s.accept()
            # We just close it immediately. The OS already sent the SYN-ACK.
            conn.close() 
    except Exception as e:
        print(f"[-] TCP Server error: {e}")

def run_udp_server(port):
    """Binds to a UDP port to simulate an 'open' UDP port."""
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.bind(('0.0.0.0', port))
        print(f"[+] UDP Server bound to port {port} (State: OPEN)")
        while True:
            data, addr = s.recvfrom(1024)
            # UDP open ports in this assignment don't need to respond, 
            # the OS simply drops the packet without sending ICMP Unreachable.
    except Exception as e:
        print(f"[-] UDP Server error: {e}")

if __name__ == "__main__":
    print("=== L4 Scanner Test Environment ===")
    
    # Start the servers in daemon threads so they exit when main exits
    tcp_thread = threading.Thread(target=run_tcp_server, args=(TCP_OPEN_PORT,), daemon=True)
    udp_thread = threading.Thread(target=run_udp_server, args=(UDP_OPEN_PORT,), daemon=True)
    
    tcp_thread.start()
    udp_thread.start()

    print("\n[!] To test 'CLOSED' ports, scan any port not listed above (e.g., TCP 8081, UDP 9091).")
    print("[!] To test TCP 'FILTERED' state, you will need to configure iptables to drop packets (see instructions below).")
    print("\nPress Ctrl+C to stop the test server...")
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nShutting down dummy server.")
        sys.exit(0)