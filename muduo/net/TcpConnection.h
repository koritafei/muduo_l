#ifndef __TCPCONNECTION__H__
#define __TCPCONNECTION__H__

#include <boost/any.hpp>
#include <memory>

#include "../base/Noncopyable.h"
#include "../base/StringPiece.h"
#include "../base/Types.h"
#include "Buffer.h"
#include "Callbacks.h"
#include "InetAddress.h"

struct tcp_info;

namespace muduo {

namespace net {

class Channel;
class EventLoop;
class Socket;

class TcpConnection : Noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:
  TcpConnection(EventLoop*         loop,
                const string&      name,
                int                sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
  ~TcpConnection();

  EventLoop* getLoop() const {
    return loop_;
  }

  const string& name() const {
    return name_;
  }

  const InetAddress& localAddress() const {
    return localAddr_;
  }

  const InetAddress& peerAddress() const {
    return peerAddr_;
  }

  bool connected() const {
    return state_ == kConnected;
  }

  bool disconencted() const {
    return state_ == kDisconnected;
  }

  bool   getTcpInfo(struct tcp_info*) const;
  string getTcpInfoString() const;

  void send(const void* message, int len);
  void send(const StringPiece& message);
  // void send(Buffer&& message);
  void send(Buffer* message);

  void shutdown();
  // void shutdownAndForceCloseAfter(double seconds);
  void forceClose();
  void forceCloseWithDelay(double seconds);

  void setTcpNoDelay(bool on);

  void startRead();
  void stopRead();
  bool isReading() const {
    return reading_;
  }

  void setContext(const boost::any& context) {
    context_ = context;
  }

  const boost::any& getContext() const {
    return context_;
  }

  boost::any* getMultiContext() {
    return &context_;
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

  void setHighWaterMarkCallback(const HighWaterMarkCallback& cb,
                                size_t                       highWaterMark) {
    highWaterMarkCakkback_ = cb;
    highWaterMark_         = highWaterMark;
  }

  void setCloseCallback(const CloseCallback& cb) {
    closeCallback_ = cb;
  }

  Buffer* inputBuffer() {
    return &inputBuffer_;
  }

  Buffer* outputBuffer() {
    return &outputBuffer_;
  }

  void connectEstablished();
  void connectDistory();

private:
  enum statusE { kDisconnected, kConnecting, kConnected, kDisConnecting };

  void handleRead(TimeStamp receiveTime);
  void handleWrite();
  void handleError();
  void handleClose();

  void sendInLoop(const StringPiece& meesage);
  void sendInLoop(const void* message, size_t len);

  void shutdownInLoop();
  void forceCloseInLoop();

  void setState(statusE s) {
    state_ = s;
  }

  const char* stateToString() const;

  void startReadInLoop();
  void stopReadInLoop();

  EventLoop*               loop_;
  const string             name_;
  statusE                  state_;
  bool                     reading_;
  std::unique_ptr<Socket>  socket_;
  std::unique_ptr<Channel> channel_;
  const InetAddress        localAddr_;
  const InetAddress        peerAddr_;
  ConnectionCallback       connectionCallback_;
  MessageCallback          messageCallback_;
  WriteCompleteCallback    writeCompleteCallback_;
  HighWaterMarkCallback    highWaterMarkCakkback_;
  CloseCallback            closeCallback_;
  size_t                   highWaterMark_;
  Buffer                   inputBuffer_;
  Buffer                   outputBuffer_;
  boost::any               context_;
};  // class TcpConnection

}  // namespace net

}  // namespace muduo

#endif /* __TCPCONNECTION__H__ */
