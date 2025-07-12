# Event-Loop

Reproduce an event loop that using epoll like nodejs, eventlet, gevent when handling non blocking process


## Build & Run
```bash
make all
./main
```

## Testing

- Terminal 1: Start server
./main

- Terminal 2: Test multiple clients
```bash
telnet localhost 8080
telnet localhost 8080  # using new terminal
telnet localhost 8080  # using new terminal
```

## Epoll works

```
EPOLL SERVER FLOW:

┌─────────────────────────────────────────────────────────────┐
│                    SERVER PROCESS                           │
│  ┌─────────────┐                                            │
│  │ epoll_wait()│ ←──────────────────────────────────────┐   │
│  │   (sleeps)  │                                        │   │
│  └─────┬───────┘                                        │   │
│        │ "Events ready!"                                │   │
│        ▼                                                │   │
│  ┌─────────────┐                                        │   │
│  │ Event Loop  │                                        │   │
│  │ for(events) │                                        │   │
│  └─────┬───────┘                                        │   │
│        │                                                │   │
│        ▼                                                │   │
│  ┌─────────────┐         ┌─────────────┐                │   │
│  │New Client?  │────Yes──┤  accept()   │                │   │
│  │sockfd event │         │add to epoll │                │   │
│  └─────┬───────┘         └─────────────┘                │   │
│        │No                                              │   │
│        ▼                                                │   │
│  ┌─────────────┐         ┌─────────────┐                │   │
│  │Client Data? │────Yes──┤ recv()/send()│               │   │
│  │client_fd    │         │   response   │               │   │
│  └─────────────┘         └─────────────┘                │   │
│                                   │                     │   │
│                                   └─────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

## Current Implementation

**Concept**
- epoll_create1() - create epoll instance
- epoll_ctl() - add/remove sockets from monitoring
- epoll_wait() - wait for events efficiently
- fcntl(O_NONBLOCK) - make sockets non-blocking
- socket/bind/listen/accept - basic TCP setup
- listen(sockfd, 5) backlog still relevant epoll fast but need to digest and balancing between a connection `for i in {1..20}; do telnet localhost 8080 & done` tested if too fast some connection get refused, can change to 1024

**Features:**
- Multi-client TCP server using epoll for efficient I/O
- Non-blocking sockets with epoll event notification
- Handle multiple connections concurrently in single thread
- Professional event-driven architecture

