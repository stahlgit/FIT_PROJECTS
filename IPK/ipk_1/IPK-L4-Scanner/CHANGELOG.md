# CHANGELOG
## [2026-04-03]
### Summary of implemented functionality
- TCP SYN scanning (IPv4 and IPv6) — SYN-only, no full handshake
- UDP scanning (IPv4 and IPv6) — ICMP port-unreachable detection
- Concurrent scanning via worker thread pool with shared PcapListener
- DNS resolution supporting dual-stack (IPv4 + IPv6) targets
- Configurable timeout per port scan (`-w`)
- Interface listing (`-i` alone)
- RAII resource management throughout (SocketGuard, TaskGuard)
- Automated test suite via Catch2 (`make test`, no root required)
 
### Known Limitations
- Mixed port format (e.g. `-t 22,25-30,35`) is not supported — the assignment explicitly permits this omission
- UDP has no `filtered` state — no ICMP port-unreachable reply is treated as `open`, per RFC 768 and Nmap convention
- IPv6 scanning requires the selected interface to have an IPv6 address assigned
- Fragmented IP responses are not handled


---

## [2026-04-02]
### Added 
- **[74d9435d]** - IP family to SocketGuard contruction 
- **[a744f315]** - RAII (Resource Acquisition Is Initialization) Task Tracker Guard
- **[7c26dc21]** - Caching interface addresses

### Changed 
- **[f42d1be7]** - pick_port method to more thread-safe and lock-free variant

---
## [2026-03-31]
### Added 
- **[93de0ee9]** - IPv6 support
- **[996e1c43]** - wider link layer offset support

### Removed 
- **[68b908a3]** - Removed Logger Utility - unnecessary for this use case

--- 
## [2026-03-28]
### Added
- **[6dfac3f1]** - Shared pcap listeners for scanners 
### Changed
- **[378a0c32]** - Abstracted packet creation to Scanner Base Class
### Fixed
- **[05db6a67]** - CLI Interface Listing

---
## [2026-03-27]
### Changed
- **[0dd0a195]** - Refactored Scanner Base with optimized checksum

---
## [2026-03-26]
### Added
- **[37903e71]** - Scanner Base Class
- **[1626a7d1]** - TCP Scanner
- **[c50030fe]** - UDP Scanner
- **[b92ded4a]** - Scanning Scheduler connection to Scanners
- **[a24199e7]** - Interface Discovery

---
## [2026-03-24]
### Added
- **[39885bd1]** - Scanning Scheduler
- **[0895783c]** - RAII Socket Guard

---
## [2026-03-23]
### Added
- **[f0ad20d8]** - Logger utility
- **[e5fd96f1]** - Main Application Class
- **[3aeeca55]** - Port Validaton 
- **[6ca4ca56]** - Running Signal Handler

---
## [2026-03-22]
### Added
- **[5d2b3b29]** - Argument Parser, Basic Configuration structure
- **[f727d020]** - DNS Resolver Implementation
