#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define PORT 8080
#define MAX 1024
#define SA struct sockaddr

int main() {
  int sockfd, connfd;
  unsigned int address_len;
  char buffer[MAX];
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in servaddr, client_addr_info;
  if (sockfd == -1) {
    printf("socket creation failed");
  } else {
    printf("Socket successfully created\n");
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(PORT);

  if (bind(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0) {
    printf("socket bind failed\n");
    exit(0);
  } else {
    printf("Socket bind successfully binded..\n");
  }

  // 5 its maximum number connection
  if (listen(sockfd, 5) != 0) {
    printf("Server failed listen\n");
  } else {
    printf("Server listen\n");
  }
  address_len = sizeof(client_addr_info);

  while (1) {
    connfd = accept(sockfd, (SA *)&client_addr_info, &address_len);
    if (connfd < 0) {
      printf("Accepted connection failed\n");
      continue; // Sip to accept other connection
    } else {
      printf("Accepted connection success\n");
    }

    // Buffer not need to set on & because char its same with array means
    // pointer default
    int bytes_received = recv(connfd, buffer, MAX, 0);
    if (bytes_received < 0) {
      printf("received failed\n");
    } else {
      printf("Received: %s\n", buffer);
    }

    char *response = "Hello from server";
    send(connfd, response, strlen(response), 0);
    close(connfd);
  }
}
