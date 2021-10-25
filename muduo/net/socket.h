#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "../base/Noncopyable.h"

struct tcp_info;

namespace muduo {

namespace net {

class InetAddress;

class Socket : Noncopyable {
public:
  explicit Socket(int socketfd) : socketfd_(socketfd) {
  }

  ~Socket();

  int fd() const {
    return socketfd_;
  }

  bool getTcpInfo(struct tcp_info *) const;
  bool getTcpInfoString(char *buf, int len) const;

  void bindAddress(const InetAddress &localaddr);

  void listen();
  int  accept(InetAddress *peeraddr);
  void shutdownWrite();
  void setTcpNoDelay(bool on);
  void setReuseAddr(bool on);
  void setReusePort(bool on);

  void setKeepAlive(bool on);

private:
  const int socketfd_;
};  // class Socket

}  // namespace net

}  // namespace muduo

#endif /* __SOCKET_H__ */
