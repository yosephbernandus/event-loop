#include <asm-generic/socket.h>
#include <fcntl.h>
#include <liburing.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define MAX_BUFFER 1024
#define BACKLOG 128
#define QUEUE_DEPTH 256

enum {
  EVENT_ACCEPT = 0,
  EVENT_READ = 1,
  EVENT_WRITE = 2,
};

typedef struct conn_info {
  int fd;
  int type;
  int buf_size;
  char buffer[MAX_BUFFER];
} conn_info;

void set_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    perror("fcntl F_GETFL");
    return;
  }
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    perror("fcntl F_SETFL");
  }
}

int setup_listening_socket() {
  int sockfd;
  struct sockaddr_in servaddr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("socket creation failed");
    exit(1);
  }

  // Allow port reuse
  int enable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) <
      0) {
    perror("setsockopt SO_REUSEADDR");
    exit(1);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(PORT);

  if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
    perror("socket bind failed");
    exit(1);
  }

  if (listen(sockfd, BACKLOG) != 0) {
    perror("listen failed");
    exit(1);
  }

  printf("Server listening on port %d\n", PORT);
  return sockfd;
}

void accept_request(struct io_uring *ring, int server_fd,
                    struct sockaddr_in *client_addr, socklen_t *client_len) {
  struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
  if (!sqe) {
    fprintf(stderr, "Could not get next available submission queue entry from "
                    "the submission queue\n");
    return;
  }

  conn_info *conn = malloc(sizeof(conn_info));
  if (!conn) {
    perror("malloc");
    return;
  }
  conn->fd = server_fd;
  conn->type = EVENT_ACCEPT;

  // Prepare accept operation
  io_uring_prep_accept(sqe, server_fd, (struct sockaddr *)client_addr,
                       client_len, 0);
  io_uring_sqe_set_data(sqe, conn);
}

void read_request(struct io_uring *ring, int client_fd) {
  struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
  if (!sqe) {
    fprintf(stderr, "Could not get next available submission queue entry from "
                    "the submission queue");
    return;
  }

  conn_info *conn = malloc(sizeof(conn_info));
  if (!conn) {
    perror("malloc");
    return;
  }

  conn->fd = client_fd;
  conn->type = EVENT_READ;
  conn->buf_size = 0;
  memset(conn->buffer, 0, MAX_BUFFER);

  // Prepare read operation
  io_uring_prep_recv(sqe, client_fd, conn->buffer, MAX_BUFFER, 0);
  io_uring_sqe_set_data(sqe, conn);
}

void write_request(struct io_uring *ring, int client_fd, const char *message,
                   int len) {
  struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
  if (!sqe) {
    fprintf(stderr, "Could not get next available submission queue entry from "
                    "the submission queue");
    return;
  }

  conn_info *conn = malloc(sizeof(conn_info));
  if (!conn) {
    perror("malloc");
    return;
  }

  conn->fd = client_fd;
  conn->type = EVENT_WRITE;
  conn->buf_size = len;
  memcpy(conn->buffer, message, len);

  // Prepare write operation
  io_uring_prep_send(sqe, client_fd, conn->buffer, len, 0);
  io_uring_sqe_set_data(sqe, conn);
}

int main(int argc, char *argv[]) {

  struct io_uring ring;
  struct io_uring_cqe *cqe;
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);

  int port = PORT;
  if (argc > 1) {
    port = atoi(argv[1]);
  }

  // Initialize io_uring with queue depth
  if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
    perror("io_uring_queue_init");
    exit(1);
  }
  printf("io_uring initialized with queue depth %d\n", QUEUE_DEPTH);

  int server_fd = setup_listening_socket();
  set_nonblocking(server_fd);

  accept_request(&ring, server_fd, &client_addr, &client_len);

  io_uring_submit(&ring);
  printf("Server running. Press Ctrl+C to stop.\n");

  // Event loop process
  while (1) {
    int ret = io_uring_wait_cqe(&ring, &cqe);
    if (ret < 0) {
      if (ret == -EINTR) {
        continue;
      }

      perror("io_uring_wait_cqe");
      break;
    }

    // Get connection from completion event
    conn_info *conn = (conn_info *)io_uring_cqe_get_data(cqe);
    if (!conn) {
      io_uring_cqe_seen(&ring, cqe);
      continue;
    }

    int result = cqe->res;

    if (result < 0) {
      if (result != -ECONNRESET && result != -EPIPE) {
        fprintf(stderr, "Operation failed on fd=%d: %s\n", conn->fd,
                strerror(-result));
      }
      if (conn->type != EVENT_ACCEPT) {
        close(conn->fd);
      }
      io_uring_cqe_seen(&ring, cqe);
      free(conn);
      continue;
    }

    // Handle different event types
    switch (conn->type) {
    case EVENT_ACCEPT: {
      int client_fd = result;
      printf("New connection accepted, fd=%d\n", client_fd);

      // Set client socket to non-blocking
      set_nonblocking(client_fd);

      // Queue read request for new client
      read_request(&ring, client_fd);

      // Queue another accept to handle next connection
      accept_request(&ring, server_fd, &client_addr, &client_len);

      // submit both operations
      io_uring_submit(&ring);

      break;
    }

    case EVENT_READ: {
      if (result == 0) {
        // client disconnect
        printf("Client disconnected, fd=%d\n", conn->fd);
        close(conn->fd);
      } else {
        // Data received
        conn->buffer[result] = '\0'; // Null terminate
        printf("Received %d bytes from fd=%d: %s\n", result, conn->fd,
               conn->buffer);

        // response
        const char *response = "Hello from io_uring server!\n";
        write_request(&ring, conn->fd, response, strlen(response));
        io_uring_submit(&ring);
      }
      break;
    }

    case EVENT_WRITE: {
      printf("Sent %d bytes to fd=%d\n", result, conn->fd);

      // after writing, queue another read
      read_request(&ring, conn->fd);
      io_uring_submit(&ring);
      break;
    }
    }

    //  Mark completion event as seen
    io_uring_cqe_seen(&ring, cqe);
    free(conn);
  }

  // clean up
  io_uring_queue_exit(&ring);
  close(server_fd);
  return 0;
}
