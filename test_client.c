#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX 1024

int main(int argc, char *argv[]) {
  int sockfd;
  struct sockaddr_in servaddr;
  char buffer[MAX];

  if (argc < 4) {
    printf("Usage: %s <host> <port> <message>\n", argv[0]);
    printf("Example: %s localhost 8080 \"Hello Server\"\n", argv[0]);
    exit(0);
  }

  const char *host = argv[1];
  int port = atoi(argv[2]);
  const char *message = argv[3];

  // Create socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    printf("socket creation failed\n");
    exit(1);
  }

  // Resolve hostname
  struct hostent *server = gethostbyname(host);
  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(1);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  memcpy(&servaddr.sin_addr.s_addr, server->h_addr, server->h_length);
  servaddr.sin_port = htons(port);

  // Connect to server
  if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
    printf("connection with the server failed\n");
    exit(1);
  }
  printf("Connected to %s:%d\n", host, port);

  // Send message
  send(sockfd, message, strlen(message), 0);
  printf("Sent: %s\n", message);

  // Receive response
  memset(buffer, 0, MAX);
  int bytes_received = recv(sockfd, buffer, MAX - 1, 0);
  if (bytes_received > 0) {
    buffer[bytes_received] = '\0';
    printf("Received: %s\n", buffer);
  }

  close(sockfd);
  return 0;
}
