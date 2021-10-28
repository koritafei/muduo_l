#include "socket.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <sys/socket.h>

#include "../base/Logging.h"
#include "InetAddress.h"
#include "SocketsOps.h"

using namespace muduo;
using namespace muduo::net;

Socket::~Socket() {
  sockets::close(socketfd_);
}

bool Socket::getTcpInfo(struct tcp_info* tcpi) const {
  socklen_t len = sizeof(*tcpi);
  memZero(tcpi, len);

  return ::getsockopt(socketfd_, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}

bool Socket::getTcpInfoString(char* buf, int len) const {
  struct tcp_info tcpi;
  bool            ok = getTcpInfo(&tcpi);

  if (ok) {
    snprintf(buf,
             len,
             "unrecovered=%u "
             "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
             "lost=%u retrans=%u rtt=%u rttvar=%u "
             "sshthresh=%u cwnd=%u total_retrans=%u",
             tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
             tcpi.tcpi_rto,          // Retransmit timeout in usec
             tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
             tcpi.tcpi_snd_mss,
             tcpi.tcpi_rcv_mss,
             tcpi.tcpi_lost,     // Lost packets
             tcpi.tcpi_retrans,  // Retransmitted packets out
             tcpi.tcpi_rtt,      // Smoothed round trip time in usec
             tcpi.tcpi_rttvar,   // Medium deviation
             tcpi.tcpi_snd_ssthresh,
             tcpi.tcpi_snd_cwnd,
             tcpi.tcpi_total_retrans);
  }

  return ok;
}

void Socket::bindAddress(const InetAddress& localaddr) {
  sockets::bindOrDie(socketfd_, localaddr.getSockAddr());
}

void Socket::listen() {
  sockets::listenOrDie(socketfd_);
}

int Socket::accept(InetAddress* peeraddr) {
  struct sockaddr_in6 addr;
  memZero(&addr, sizeof addr);

  int connfd = sockets::accept(socketfd_, &addr);
  if (0 <= connfd) {
    peeraddr->setSockAddrInet6(addr);
  }

  return connfd;
}

void Socket::shutdownWrite() {
  sockets::shutdownWrite(socketfd_);
}

void Socket::setTcpNoDelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(socketfd_,
               IPPROTO_TCP,
               TCP_NODELAY,
               &optval,
               static_cast<socklen_t>(sizeof optval));
}

void Socket::setReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(socketfd_,
               SOL_SOCKET,
               SO_REUSEADDR,
               &optval,
               static_cast<socklen_t>(sizeof optval));
}

void Socket::setReusePort(bool on) {
#ifdef SO_REUSEPORT
  int optval = on ? 1 : 0;
  int ret    = ::setsockopt(socketfd_,
                         SOL_SOCKET,
                         SO_REUSEPORT,
                         &optval,
                         static_cast<socklen_t>(sizeof optval));
  if (0 > ret && on) {
    LOG_SYSERR << "SO_REUSEPORT failed.";
  }
#else
  if (on) {
    LOG_ERROR << "SO_REUSEPORT is not supported.";
  }
#endif
}

void Socket::setKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(socketfd_,
               SOL_SOCKET,
               SO_KEEPALIVE,
               &optval,
               static_cast<socklen_t>(sizeof optval));
}
