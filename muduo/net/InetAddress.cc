#include "InetAddress.h"

#include <bits/stdint-uintn.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstddef>

#include "../base/Logging.h"
#include "Endian.h"
#include "SocketsOps.h"

#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny      = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

using namespace muduo;
using namespace muduo::net;

static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),
              "InetAddr is same size as sockaddr_in6");
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin family offset is 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0,
              "sin6 family offset is 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin port offset is 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6 port offset is 2");

InetAddress::InetAddress(uint16_t portArg, bool loopbackOnly, bool ipv6) {
  static_assert(offsetof(InetAddress, addr6_) == 0, "addr6_ offset 0");
  static_assert(offsetof(InetAddress, addr_) == 0, "addr_ offset 0");

  if (ipv6) {
    memZero(&addr6_, sizeof addr6_);
    addr6_.sin6_family = AF_INET6;
    in6_addr ip        = loopbackOnly ? in6addr_loopback : in6addr_any;
    addr6_.sin6_addr   = ip;
    addr6_.sin6_port   = sockets::hostToNetwork16(portArg);
  } else {
    memZero(&addr_, sizeof(addr_));
    addr_.sin_family      = AF_INET;
    in_addr_t ip          = loopbackOnly ? kInaddrLoopback : kInaddrAny;
    addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
    addr_.sin_port        = sockets::hostToNetwork16(portArg);
  }
}

InetAddress::InetAddress(StringArg ip, uint16_t port, bool ipv6) {
  if (ipv6 || strchr(ip.c_str(), ':')) {
    memZero(&addr6_, sizeof addr6_);
    sockets::fromIpPort(ip.c_str(), port, &addr6_);
  } else {
    memZero(&addr_, sizeof addr_);
    sockets::fromIpPort(ip.c_str(), port, &addr_);
  }
}

string InetAddress::toIpPort() const {
  char buf[64] = "";
  sockets::toIpPort(buf, sizeof buf, getSockAddr());
  return buf;
}

string InetAddress::toIp() const {
  char buf[64] = "";
  sockets::toIp(buf, sizeof buf, getSockAddr());
  return buf;
}

uint32_t InetAddress::ipv4NetEndian() const {
  assert(AF_INET == family());

  return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::port() const {
  return sockets::networkTohost16(portNetEndian());
}

static __thread char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(StringArg hostname, InetAddress *out) {
  assert(out != nullptr);
  struct hostent  hent;
  struct hostent *he     = nullptr;
  int             herrno = 0;

  memZero(&hent, sizeof hent);
  int ret = gethostbyname_r(hostname.c_str(),
                            &hent,
                            t_resolveBuffer,
                            sizeof t_resolveBuffer,
                            &he,
                            &herrno);
  if (ret != 0 && he != nullptr) {
    assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
    out->addr_.sin_addr = *reinterpret_cast<in_addr *>(he->h_addr);
    return true;
  } else {
    if (ret) {
      LOG_SYSERR << "InetAddress::resolve";
    }

    return false;
  }
}

void InetAddress::setScopedId(uint32_t scoped_id) {
  if (AF_INET6 == family()) {
    addr6_.sin6_scope_id = scoped_id;
  }
}
