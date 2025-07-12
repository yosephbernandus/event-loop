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

- Terminal 2: Test client
telnet localhost 8080

## Step 1
Basic TCP that accepts connections
socket() - create server socket
bind() - bind to port 8080
listen() - start listening
accept() - wait for clients 
recv() - read data 
send() - send response 
