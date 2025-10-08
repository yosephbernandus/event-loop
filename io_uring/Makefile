CC = gcc
CFLAGS = -Wall -O2 -g
LDFLAGS_URING = -luring

all: server_epoll server_iouring test_client

server_epoll: server_epoll.c
	$(CC) $(CFLAGS) -o server_epoll server_epoll.c

server_iouring: server_iouring.c
	$(CC) $(CFLAGS) -o server_iouring server_iouring.c $(LDFLAGS_URING)

test_client: test_client.c
	$(CC) $(CFLAGS) -o test_client test_client.c

clean:
	rm -f server_epoll server_iouring test_client

test_epoll: server_epoll test_client
	@echo "Testing epoll server..."
	@./server_epoll &
	@sleep 1
	@./test_client localhost 8080 "Test message"
	@pkill server_epoll

test_iouring: server_iouring test_client
	@echo "Testing io_uring server..."
	@./server_iouring &
	@sleep 1
	@./test_client localhost 8080 "Test message"
	@pkill server_iouring

.PHONY: all clean test_epoll test_iouring
