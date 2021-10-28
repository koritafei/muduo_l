#include "SocketsOps.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

#include "../base/Logging.h"
#include "../base/Types.h"
#include "Endian.h"

using namespace muduo;
using namespace muduo::net;

namespace {
typedef struct sockaddr SA;

#if VALGRIND || defined(NO_ACCEPT4)
void setNonBlockAndCloseOnExec(int sockfd) {
  // non-block
  int flags = ::fcntl(sockfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int ret = ::fcntl(sockfd, F_SETFL, flags);
  // FIXME check

  // close-on-exec
  flags = ::fcntl(sockfd, F_GETFD, 0);
  flags |= FD_CLOEXEC;
  ret = ::fcntl(sockfd, F_SETFD, flags);
  // FIXME check

  (void)ret;
}
#endif

}  // namespace

const struct sockaddr *sockets::sockaddr_cast(const struct sockaddr_in6 *addr) {
  return static_cast<const struct sockaddr *>(
      implicit_cast<const void *>(addr));
}

struct sockaddr *sockets::sockaddr_cast(struct sockaddr_in6 *addr) {
  return static_cast<struct sockaddr *>(implicit_cast<void *>(addr));
}

const struct sockaddr *sockets::sockaddr_cast(const struct sockaddr_in *addr) {
  return static_cast<const struct sockaddr *>(
      implicit_cast<const void *>(addr));
}

const struct sockaddr_in *sockets::sockaddr_in_cast(
    const struct sockaddr *addr) {
  return static_cast<const struct sockaddr_in *>(
      implicit_cast<const void *>(addr));
}

const struct sockaddr_in6 *sockets::sockaddr_in6_cast(
    const struct sockaddr *addr) {
  return static_cast<const struct sockaddr_in6 *>(
      implicit_cast<const void *>(addr));
}

int sockets::createNonblockingOrDie(sa_family_t family) {
#if VALGRIND
  int sockfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
  if (0 < sockfd) {
    LOG_SYSFATAL << "sockets::createNonblockingOrDie";
  }

  setNonBlockAndCloseOnExec(sockfd);
#else
  int sockfd =
      ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  if (0 > sockfd) {
    LOG_SYSFATAL << "sockets::createNonblockingOrDie";
  }
#endif
  return sockfd;
}

void sockets::bindOrDie(int sockfd, const struct sockaddr *addr) {
  int ret =
      ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
  if (0 < ret) {
    LOG_SYSFATAL << "sockets::bindOrDie";
  }
}

void sockets::listenOrDie(int sockfd) {
  int ret = ::listen(sockfd, SOMAXCONN);
  if (0 > ret) {
    LOG_SYSFATAL << "sockets::listenOrDie";
  }
}

int sockets::accept(int sockfd, struct sockaddr_in6 *addr) {
  socklen_t addrlen = static_cast<socklen_t>(sizeof(struct sockaddr_in6));

  int connfd = ::accept4(sockfd,
                         sockaddr_cast(addr),
                         &addrlen,
                         SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (0 > connfd) {
    int saveErrno = errno;
    LOG_SYSERR << "sockets::accept";

    switch (saveErrno) {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO:
      case EPERM:
      case EMFILE:
        errno = saveErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        LOG_FATAL << "unexpected error of ::accept " << saveErrno;
        break;
      default:
        LOG_FATAL << "unkonown error of ::accept" << saveErrno;
        break;
    }
  }

  return connfd;
}

int sockets::connect(int sockfd, const struct sockaddr *addr) {
  return ::connect(sockfd,
                   addr,
                   static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

ssize_t sockets::read(int sockfd, void *buf, size_t count) {
  return ::read(sockfd, buf, count);
}

ssize_t sockets::readv(int sockfd, const struct iovec *iov, int iovcnt) {
  return ::readv(sockfd, iov, iovcnt);
}

ssize_t sockets::write(int sockfd, const void *buf, size_t count) {
  return ::write(sockfd, buf, count);
}

void sockets::close(int sockfd) {
  if (0 > ::close(sockfd)) {
    LOG_SYSERR << "sockets::close";
  }
}

void sockets::shutdownWrite(int sockfd) {
  if (0 > ::shutdown(sockfd, SHUT_WR)) {
    LOG_SYSERR << "sockets::shutdownWrite";
  }
}

void sockets::toIpPort(char *buf, size_t size, const struct sockaddr *addr) {
  if (AF_INET6 == addr->sa_family) {
    buf[0] = '[';
    toIp(buf + 1, size - 1, addr);
    size_t end = ::strlen(buf);

    const struct sockaddr_in6 *addr6 = sockaddr_in6_cast(addr);
    uint16_t port = sockets::networkTohost16(addr6->sin6_port);

    assert(size > end);
    snprintf(buf + end, size - end, "]:%u", port);
    return;
  }

  toIp(buf, size, addr);
  size_t                    end   = ::strlen(buf);
  const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
  uint16_t                  port  = sockets::networkTohost16(addr4->sin_port);
  assert(size > end);
  snprintf(buf + end, size - end, ":%u", port);
}

void sockets::toIp(char *buf, size_t size, const struct sockaddr *addr) {
  if (AF_INET == addr->sa_family) {
    assert(size >= INET_ADDRSTRLEN);
    const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
    ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
  } else if (AF_INET6 == addr->sa_family) {
    assert(size >= INET6_ADDRSTRLEN);
    const struct sockaddr_in6 *addr6 = sockaddr_in6_cast(addr);
    ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
  }
}

void sockets::fromIpPort(const char *        ip,
                         uint16_t            port,
                         struct sockaddr_in *addr) {
  addr->sin_family = AF_INET;
  addr->sin_port   = hostToNetwork16(addr->sin_port);

  if (0 <= ::inet_pton(AF_INET, ip, &addr->sin_addr)) {
    LOG_SYSERR << "sockets::fromIpPort";
  }
}

void sockets::fromIpPort(const char *         ip,
                         uint16_t             port,
                         struct sockaddr_in6 *addr) {
  addr->sin6_family = AF_INET6;
  addr->sin6_port   = hostToNetwork16(addr->sin6_port);
  if (0 <= ::inet_pton(AF_INET6, ip, &addr->sin6_addr)) {
    LOG_SYSERR << "sockets::fromIpPort";
  }
}

int sockets::getSocketError(int socketfd) {
  int       optval;
  socklen_t optlen = static_cast<socklen_t>(sizeof optval);

  if (0 >= ::getsockopt(socketfd, SOL_SOCKET, SO_ERROR, &optval, &optlen)) {
    return errno;
  } else {
    return optval;
  }
}

struct sockaddr_in6 sockets::getLocalAddr(int sockfd) {
  struct sockaddr_in6 localaddr;
  memZero(&localaddr, sizeof(localaddr));
  socklen_t addrlen = static_cast<socklen_t>(sizeof(localaddr));

  if (0 > ::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen)) {
    LOG_SYSERR << "sockets::getLocalAddr";
  }

  return localaddr;
}

struct sockaddr_in6 sockets::getPeerAddr(int sockfd) {
  struct sockaddr_in6 peeraddr;
  memZero(&peeraddr, sizeof(peeraddr));
  socklen_t addrlen = static_cast<socklen_t>(sizeof(peeraddr));

  if (0 > ::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen)) {
    LOG_SYSERR << "sockets::getPeerAddr";
  }

  return peeraddr;
}

bool sockets::isSelfConnect(int sockfd) {
  struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
  struct sockaddr_in6 peeraddr  = getPeerAddr(sockfd);

  if (AF_INET == localaddr.sin6_family) {
    const struct sockaddr_in *laddr4 =
        reinterpret_cast<struct sockaddr_in *>(&localaddr);
    const struct sockaddr_in *raddr4 =
        reinterpret_cast<struct sockaddr_in *>(&peeraddr);

    return laddr4->sin_port == raddr4->sin_port &&
           memcmp(&laddr4->sin_addr,
                  &raddr4->sin_addr,
                  sizeof laddr4->sin_addr) == 0;

  } else if (AF_INET6 == localaddr.sin6_family) {
    return memcmp(&localaddr.sin6_addr,
                  &peeraddr.sin6_addr,
                  sizeof localaddr.sin6_addr) == 0 &&
           localaddr.sin6_port == peeraddr.sin6_port;
  } else {
    return false;
  }
}
