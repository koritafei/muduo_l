#ifndef __TCPCLIENT__H__
#define __TCPCLIENT__H__

#include "../base/Mutex.h"
#include "TcpConnection.h"

namespace muduo {

namespace net {

class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;

class TcpClient : Noncopyable {
public:
  TcpClient(EventLoop*         loop,
            const InetAddress& serverAddr,
            const string&      nameArg);
  ~TcpClient();

  void connect();
  void disconnect();
  void stop();

  TcpConnectionPtr connection() const {
    MutexLockGuard lock(mutex_);
    return connection_;
  }

  EventLoop* getLoop() const {
    return loop_;
  }

  bool retry() const {
    return retry_;
  }

  void enableRetry() {
    retry_ = true;
  }

  const string& name() const {
    return name_;
  }

  void setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
  }

  void setMessageCallback(const MessageCallback& cb) {
    messageCallback_ = cb;
  }

  void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writeCompleteCallback_ = cb;
  }

private:
  void newConnection(int sockfd);

  void removeConnection(const TcpConnectionPtr& conn);

  EventLoop*                   loop_;
  ConnectorPtr                 connector_;
  const string                 name_;
  ConnectionCallback           connectionCallback_;
  MessageCallback              messageCallback_;
  WriteCompleteCallback        writeCompleteCallback_;
  bool                         retry_;
  bool                         connect_;
  int                          nextConnId_;
  mutable MutexLock            mutex_;
  TcpConnectionPtr connection_ GUARDED_BY(mutex_);
};  // class TcpClient

}  // namespace net

}  // namespace muduo

#endif /* __TCPCLIENT__H__ */
