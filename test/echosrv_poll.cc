/**
 * @file echosrv_poll.cc
 * @author koritafei (koritafei@gmail.com)
 * @brief poll
 * @version 0.1
 * @date 2021-08-29
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <vector>

#define ERR_EXIT(m)                                                            \
  do {                                                                         \
    perror(m);                                                                 \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

typedef std::vector<struct pollfd> PollfdList;

int main(int argc, char **argv) {
  signal(SIGPIPE, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);  // 避免僵死进程

  int listenfd;

  if (0 > (listenfd = socket(PF_INET,
                             SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                             IPPROTO_TCP))) {
    ERR_EXIT("socket");
  }

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_port        = htons(8899);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  int on = 1;
  if (0 > (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))) {
    ERR_EXIT("setsockopt");
  }

  if (0 > bind(listenfd,
               reinterpret_cast<struct sockaddr *>(&servaddr),
               sizeof(servaddr))) {
    ERR_EXIT("bind");
  }
  if (0 > listen(listenfd, SOMAXCONN)) {
    ERR_EXIT("listen");
  }

  struct pollfd pfd;
  pfd.fd     = listenfd;
  pfd.events = POLLIN;

  PollfdList pfdList;
  pfdList.push_back(pfd);

  int                nready;
  struct sockaddr_in peeraddr;
  socklen_t          peerlen;
  int                connfd;

  while (1) {
    nready = poll(&*pfdList.begin(), pfdList.size(), -1);

    if (-1 == nready) {
      if (EINTR == errno) {
        continue;
      }
      ERR_EXIT("poll");
    }

    if (0 == nready) {
      continue;
    }

    if (pfdList[0].revents & POLLIN) {
      peerlen = sizeof(peeraddr);
      connfd  = ::accept4(listenfd,
                         reinterpret_cast<struct sockaddr *>(&peeraddr),
                         &peerlen,
                         SOCK_NONBLOCK | SOCK_CLOEXEC);
      if (-1 == connfd) {
        ERR_EXIT("accept4");
      }

      pfd.fd      = connfd;
      pfd.events  = POLLIN;
      pfd.revents = 0;
      pfdList.push_back(pfd);

      --nready;

      // 连接成功
      std::cout << "ip = " << inet_ntoa(peeraddr.sin_addr)
                << " port = " << ntohs(peeraddr.sin_port) << " connect success!"
                << std::endl;
    }

    std::cout << "poll fd list size " << pfdList.size() << std::endl;
    std::cout << "poll fd list ready " << nready << std::endl;

    for (PollfdList::iterator it = pfdList.begin() + 1;
         it != pfdList.end() && nready > 0;
         ++it) {
      if (it->revents & POLLIN) {
        --nready;
        connfd            = it->fd;
        char    buf[1024] = {0};
        ssize_t ret       = read(connfd, buf, 1024);
        if (-1 == ret) {
          ERR_EXIT("read");
        }

        if (0 == ret) {
          std::cout << "client close" << std::endl;
          it = pfdList.erase(it);
          --it;

          close(connfd);
          continue;
        }
        std::cout << buf << std::endl;
        write(connfd, buf, sizeof(buf));
      }
    }
  }

  return 0;
}
