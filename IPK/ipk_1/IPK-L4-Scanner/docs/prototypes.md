### Prototype
#### High level prototype
```mermaid
flowchart TD

    CLI[CLI Parser] --> Config[Config Object]
    Config --> Resolver[DNS Resolver]
    Resolver --> Targets[IP Address List]

    Targets --> Scheduler[Scan Scheduler]

    Scheduler --> TCP[TCP Scanner]
    Scheduler --> UDP[UDP Scanner]

    TCP --> Sender[Packet Sender]
    UDP --> Sender

    Sender --> Network[\Network\]

    Network --> Receiver[Packet Receiver libpcap]

    Receiver --> Evaluator[Response Evaluator]

    Evaluator --> Results[Results Aggregator]

    Results --> Output[Output Formatter]

    Output --> STDOUT[stdout]
```


#### Flow Prototype
```mermaid
flowchart TD

    Start([Start Scan Task]) --> Send[Send Packet]

    Send --> Wait[Wait for Response]

    Wait --> Response{Response Received?}

    Response -->|Yes| Type{Response Type}

    Type -->|SYN-ACK| Open[Mark OPEN]
    Type -->|RST| Closed[Mark CLOSED]
    Type -->|ICMP Unreachable| Closed

    Response -->|No| Retry{Retry Already Done?}

    Retry -->|No| Resend[Send Again]
    Resend --> Wait

    Retry -->|Yes| Filtered[Mark FILTERED]

    Open --> End([Done])
    Closed --> End
    Filtered --> End
```

### Architecture concepts 
#### CLI Parser
*NOTE: Separation of Concerns: CLI Parser only checks valid syntax - no DNS resolution inside!*
```mermaid
flowchart LR

Config["`**Config**
    interface
    tcp_ports
    udp_ports
    host
    timeout_ms
    ...`"]

Arg["`**ArgParser**`"]-->|produces|Config-->
DNS["`**DNS Resolver**`"]
```

#### DNS Resolver
```mermaid
flowchart TD

config[config.host]-->DNS["DnsResolver::resolve()"]
DNS-->|failure|runtime_error
DNS-->|success|iterate[Iterate addrinfo list]-->
int_top-->vector[vector of ResolvedTarget]

```

#### ScanScheduler
```mermaid
flowchart TD

Application-->ScanScheduler
ScanScheduler-->|TCP tasks|TcpScanner
ScanScheduler-->|UDP tasks|UdpScanner
ScanScheduler-->...
TcpScanner-->SG[SocketGuard]
UdpScanner-->SG2[SocketGuard]
SG-->Results
SG2-->Results
```

### System Architecture Overview
```mermaid
flowchart TD
    CLI[CLI Parser] --> Config[Config Object]
    Config --> Resolver[DNS Resolver]
    Resolver --> Targets[Resolved Targets IPv4/IPv6]

    Targets --> Scheduler[Scan Scheduler]

    subgraph "Concurrency Engine"
        Scheduler --> |Spawns| Pool[Worker Thread Pool]
        Scheduler --> |Starts| Listener[Pcap Listener Thread]
        
        Pool --> |Registers Promise| Tracker[(Task Tracker)]
        Pool --> |Sends Packet| NetOut[Raw Sockets]
        
        NetOut --> Network((Network))
        Network --> |Captures Responses| Listener
        
        Listener --> |Resolves Promise| Tracker
        Tracker -.-> |Wakes| Pool
    end

    Pool --> Output[Output Formatter]
    Output --> STDOUT[stdout]
```


```mermaid
flowchart TD
ScanScheduler --> PcapListener
Worker --> |register_task| TaskTracker
PcapListener --> |resolve_task| TaskTracker
TaskTracker -.-> |future.get| Worker
```
