#include "TcpConnection.h"

#include <errno.h>

#include "../base/Logging.h"
#include "../base/WeakCallback.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketsOps.h"

using namespace muduo;
using namespace muduo::net;

void muduo::net::defaultConnectionCallback(const TcpConnectionPtr &conn) {
  LOG_TRACE << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");
}

void muduo::net::defaultMessageCallback(const TcpConnectionPtr &,
                                        Buffer *buf,
                                        TimeStamp) {
  buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop *        loop,
                             const string &     nameArg,
                             int                sockfd,
                             const InetAddress &localAddr,
                             const InetAddress &peerAddr)
    : loop_(CHECK_NOTNULL(loop)),
      name_(nameArg),
      state_(kConnecting),
      reading_(true),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      highWaterMark_(60 * 1024 * 1024) {
  channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, _1));
  channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
  channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
  channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

  LOG_DEBUG << "TcpConnection::ctor [" << name_ << "] at " << this
            << " fd = " << sockfd;
  socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
  LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at " << this
            << " fd=" << channel_->fd() << " state=" << stateToString();
  assert(state_ == kDisconnected);
}

bool TcpConnection::getTcpInfo(struct tcp_info *tcpi) const {
  return socket_->getTcpInfo(tcpi);
}

string TcpConnection::getTcpInfoString() const {
  char buf[1024];
  buf[0] = '\0';
  socket_->getTcpInfoString(buf, sizeof buf);
  return buf;
}

void TcpConnection::send(const void *msg, int len) {
  send(StringPiece(static_cast<const char *>(msg), len));
}

void TcpConnection::send(const StringPiece &message) {
  if (kConnected == state_) {
    if (loop_->isInLoopThread()) {
      sendInLoop(message);
    } else {
      void (TcpConnection::*fp)(const StringPiece &message) =
          &TcpConnection::sendInLoop;
      loop_->runInLoop(std::bind(fp, this, message.as_string()));
    }
  }
}

void TcpConnection::send(Buffer *buff) {
  if (kConnected == state_) {
    if (loop_->isInLoopThread()) {
      sendInLoop(buff->peek(), buff->readableBytes());
      buff->retrieveAll();
    } else {
      void (TcpConnection::*fp)(const StringPiece &) =
          &TcpConnection::sendInLoop;
      loop_->runInLoop(std::bind(fp, this, buff->retrieveAllAsString()));
    }
  }
}

void TcpConnection::sendInLoop(const StringPiece &message) {
  sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void *data, size_t len) {
  loop_->assertInLoopThread();
  ssize_t nwrote     = 0;
  size_t  remaining  = len;
  bool    faultError = false;

  if (kDisconnected == state_) {
    LOG_WARN << "disconnected. give up";
    return;
  }

  if (!channel_->isWriting() || 0 == outputBuffer_.readableBytes()) {
    nwrote = sockets::write(channel_->fd(), data, len);

    if (nwrote >= 0) {
      remaining = len - nwrote;
      if (0 == remaining && writeCompleteCallback_) {
        loop_->queueInLoop(
            std::bind(writeCompleteCallback_, shared_from_this()));
      }
    } else {
      nwrote = 0;

      if (EWOULDBLOCK != errno) {
        LOG_SYSERR << "TcpConnection::sendInLoop";

        if (EPIPE == errno || ECONNRESET == errno) {
          faultError = true;
        }
      }
    }
  }

  assert(remaining <= len);
  if (!faultError && 0 < remaining) {
    size_t oldLen = outputBuffer_.readableBytes();

    if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ &&
        highWaterMarkCakkback_) {
      loop_->queueInLoop(std::bind(highWaterMarkCakkback_,
                                   shared_from_this(),
                                   oldLen + remaining));
    }
    outputBuffer_.append(static_cast<const char *>(data) + nwrote, remaining);

    if (!channel_->isWriting()) {
      channel_->enableWriting();
    }
  }
}

void TcpConnection::shutdown() {
  if (kConnected == state_) {
    setState(kDisConnecting);
    loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
  }
}

void TcpConnection::shutdownInLoop() {
  loop_->assertInLoopThread();

  if (!channel_->isWriting()) {
    socket_->shutdownWrite();
  }
}

void TcpConnection::forceClose() {
  if (kConnected == state_ || kDisConnecting == state_) {
    setState(kDisConnecting);
    loop_->queueInLoop(
        std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
  }
}

void TcpConnection::forceCloseWithDelay(double seconds) {
  if (kConnected == state_ || kDisConnecting == state_) {
    setState(kDisConnecting);
    loop_->runAfter(
        seconds,
        makeWeakCallback(shared_from_this(), &TcpConnection::forceClose));
  }
}

void TcpConnection::forceCloseInLoop() {
  loop_->assertInLoopThread();

  if (kDisconnected == state_ || kDisConnecting == state_) {
    handleClose();
  }
}

const char *TcpConnection::stateToString() const {
  switch (state_) {
    case kConnected:
      return "kConnected";
    case kConnecting:
      return "KConnecting";
    case kDisconnected:
      return "kDisConnected";
    case kDisConnecting:
      return "kDisConnecting";
    default:
      return "unknown state";
  }
}

void TcpConnection::setTcpNoDelay(bool on) {
  socket_->setTcpNoDelay(on);
}

void TcpConnection::startRead() {
  loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop() {
  loop_->assertInLoopThread();

  if (!reading_ || !channel_->isReading()) {
    channel_->enableReading();
    reading_ = true;
  }
}

void TcpConnection::stopRead() {
  loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop() {
  loop_->assertInLoopThread();

  if (reading_ || channel_->isReading()) {
    channel_->disableReading();
    reading_ = false;
  }
}

void TcpConnection::connectEstablished() {
  loop_->assertInLoopThread();
  assert(kConnecting == state_);
  setState(kConnected);
  channel_->tie(shared_from_this());
  channel_->enableReading();

  connectionCallback_(shared_from_this());
}

void TcpConnection::connectDistory() {
  loop_->assertInLoopThread();

  if (kConnected == state_) {
    setState(kDisconnected);
    channel_->disableAll();

    connectionCallback_(shared_from_this());
  }

  channel_->remove();
}

void TcpConnection::handleRead(TimeStamp receieveTime) {
  loop_->assertInLoopThread();
  int     savedErrno = 0;
  ssize_t n          = inputBuffer_.readFd(channel_->fd(), &savedErrno);

  if (0 < n) {
    messageCallback_(shared_from_this(), &inputBuffer_, receieveTime);
  } else if (0 == n) {
    handleClose();
  } else {
    errno = savedErrno;
    LOG_SYSERR << "TcpConnection::handleRead";
    handleError();
  }
}

void TcpConnection::handleWrite() {
  loop_->assertInLoopThread();

  if (channel_->isWriting()) {
    ssize_t n = sockets::write(channel_->fd(),
                               outputBuffer_.peek(),
                               outputBuffer_.readableBytes());

    if (0 < n) {
      outputBuffer_.retrieve(0);

      if (0 == outputBuffer_.readableBytes()) {
        channel_->disableWriting();

        if (writeCompleteCallback_) {
          loop_->queueInLoop(
              std::bind(writeCompleteCallback_, shared_from_this()));
        }

        if (kDisConnecting == state_) {
          shutdownInLoop();
        }
      }

    } else {
      LOG_SYSERR << "TcpConnect::handleWrite";
    }
  } else {
    LOG_TRACE << "Connect fd = " << channel_->fd()
              << "is down, no more writing.";
  }
}

void TcpConnection::handleClose() {
  loop_->assertInLoopThread();
  LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();

  assert(kConnected == state_ || kDisConnecting == state_);
  setState(kDisconnected);
  channel_->disableAll();

  TcpConnectionPtr guardThis(shared_from_this());
  connectionCallback_(guardThis);
  closeCallback_(guardThis);
}

void TcpConnection::handleError() {
  int err = sockets::getSocketError(channel_->fd());
  LOG_ERROR << "TcpConnection::handleError [" << name_
            << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}
