#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include <functional>

#include "Channel.h"
#include "socket.h"

namespace muduo {

namespace net {

class EventLoop;
class InetAddress;

class Acceptor : Noncopyable {
public:
  typedef std::function<void(int sockfd, const InetAddress &)>
      NewConnectionCallback;

  Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);

  ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback &cb) {
    newConnectionCallback_ = cb;
  }

  void listen();

  bool listening() const {
    return listening_;
  }

private:
  void handleRead();

  EventLoop *           loop_;
  Socket                acceptSocket_;
  Channel               acceptChannel_;
  NewConnectionCallback newConnectionCallback_;
  bool                  listening_;
  int                   idleFd_;
};  // class Acceptor

}  // namespace net

}  // namespace muduo

#endif /* __ACCEPTOR_H__ */
