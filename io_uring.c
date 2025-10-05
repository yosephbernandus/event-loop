#include <asm-generic/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX 1024
#define BACKLOG 5
#define QUEUE_DEPTH 256

enum {
  EVENT_ACCEPT = 0,
  EVENT_READ = 1,
  EVENT_WRITE = 2,
};

typedef struct conn_info {
  int fd;
  int type;
  char buffer[MAX];
} conn_info;

void set_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int setup_listening_socket() {
  int sockfd;
  struct sockaddr_in servaddr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("socket creation failed");
    exit(1);
  }

  int enable = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

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

int main() {

  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);

  int server_fd = setup_listening_socket();
}
