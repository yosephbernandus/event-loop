#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define PORT 8080
#define MAX 1024
#define SA struct sockaddr
#define MAX_EVENTS 10

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

  // Max 5 clients can wait in queue
  if (listen(sockfd, 5) != 0) {
    printf("Server failed listen\n");
  } else {
    printf("Server listen\n");
  }
  address_len = sizeof(client_addr_info);

  int epfd = epoll_create1(0);
  if (epfd == -1) {
    printf("epoll_create1 failed\n");
    exit(1);
  }
  printf("epoll instance created\n");

  // To not block the request
  if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1) {
    printf("fcntl failed\n");
    exit(1);
  }

  // adding server socket to epoll
  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = sockfd;

  if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
    printf("epoll_ctl failed\n");
    exit(1);
  }

  printf("Server socket added to epoll");

  struct epoll_event events[MAX_EVENTS]; // receive multiple events

  while (1) {
    int nfds = epoll_wait(epfd, events, MAX_EVENTS,
                          -1); // -1 wait forever, 0 for non blocking

    if (nfds == -1) {
      printf("epoll_wait failed\n");
      exit(1);
    }

    for (int i = 0; i < nfds; i++) {
      if (events[i].data.fd == sockfd) {
        connfd = accept(sockfd, (SA *)&client_addr_info, &address_len);
        if (connfd == -1) {
          continue; // EAGAIN - no more connection happen
        }

        printf("new connection accepted\n");

        // non blocking
        fcntl(connfd, F_SETFL, O_NONBLOCK);

        ev.events = EPOLLIN;
        ev.data.fd = connfd;
        epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
      } else {
        // existing socket client
        int client_fd = events[i].data.fd;

        int bytes_received = recv(client_fd, buffer, MAX, 0);
        if (bytes_received <= 0) {
          printf("Client disconnected\n");
          epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, NULL);
          close(client_fd);
        } else {
          printf("Received:  %s\n", buffer);

          char *response = "Hello i'm server";
          send(client_fd, response, strlen(response), 0);
        }
      }
    }
  }
  // while (1) {
  //   connfd = accept(sockfd, (SA *)&client_addr_info, &address_len);
  //   if (connfd < 0) {
  //     printf("Accepted connection failed\n");
  //     continue; // Sip to accept other connection
  //   } else {
  //     printf("Accepted connection success\n");
  //   }
  //
  //   // Buffer not need to set on & because char its same with array means
  //   // pointer default
  //   int bytes_received = recv(connfd, buffer, MAX, 0);
  //   if (bytes_received < 0) {
  //     printf("received failed\n");
  //   } else {
  //     printf("Received: %s\n", buffer);
  //   }
  //
  //   char *response = "Hello from server";
  //   send(connfd, response, strlen(response), 0);
  //   close(connfd);
  // }
}
