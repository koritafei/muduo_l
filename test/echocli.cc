/**
 * @file echocli.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-08-29
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

int main(int argc, char **argv) {
  int sock;
  if (0 > (sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))) {
    ERR_EXIT("socket");
  }

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_port        = htons(8899);
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (0 > connect(sock,
                  reinterpret_cast<struct sockaddr *>(&servaddr),
                  sizeof(servaddr))) {
    ERR_EXIT("connect");
  }

  struct sockaddr_in localaddr;
  socklen_t          addrlen = sizeof(localaddr);
  if (0 > getsockname(sock,
                      reinterpret_cast<struct sockaddr *>(&localaddr),
                      &addrlen)) {
    ERR_EXIT("getsockname");
  }

  std::cout << "ip = " << inet_ntoa(localaddr.sin_addr)
            << " port =  " << ntohs(localaddr.sin_port) << std::endl;

  char sendbuf[1024] = {0};
  char recvbuf[1024] = {0};

  while (NULL != fgets(sendbuf, sizeof(sendbuf), stdin)) {
    write(sock, sendbuf, strlen(sendbuf));
    read(sock, recvbuf, sizeof(recvbuf));

    fputs(recvbuf, stdout);
    memset(sendbuf, 0, sizeof(sendbuf));
    memset(recvbuf, 0, sizeof(recvbuf));
  }

  close(sock);

  return 0;
}
