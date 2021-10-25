#ifndef __CONNECTOR__H__
#define __CONNECTOR__H__

#include <functional>
#include <memory>

#include "../base/Logging.h"
#include "InetAddress.h"

namespace muduo {

namespace net {

class Channel;
class EventLoop;

class Connector : Noncopyable, public std::enable_shared_from_this<Connector> {
public:
  typedef std::function<void(int sockfd)> NewConnectionCallback;
  Connector(EventLoop* loop, const InetAddress& serverAddr);
  ~Connector();

  void setNewConnectionCallback(const NewConnectionCallback& cb) {
    newConnectionCallback_ = cb;
  }

  void start();
  void restart();
  void stop();

  const InetAddress& serverAddress() const {
    return serverAddr_;
  }

private:
  enum States { kDisconnected, kConnecting, kConnected };

  static const int kMaxRetryDelayMs  = 30 * 1000;
  static const int kInitRetryDelayMs = 500;

  void setState(States s) {
    state_ = s;
  }

  void startInLoop();
  void stopInLoop();
  void connect();
  void connecting(int sockfd);
  void handleWrite();
  void handleError();
  void retry(int sockfd);
  int  removeAndResetChannel();
  void resetChannel();

  EventLoop*               loop_;
  InetAddress              serverAddr_;
  bool                     connect_;
  States                   state_;
  std::unique_ptr<Channel> channel_;
  NewConnectionCallback    newConnectionCallback_;
  int                      retryDelayMs_;
};  // class Connector

}  // namespace net

}  // namespace muduo

#endif /* __CONNECTOR__H__ */
