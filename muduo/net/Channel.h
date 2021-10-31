#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include <functional>
#include <memory>

#include "../base/Noncopyable.h"
#include "../base/TimeStamp.h"

namespace muduo {

namespace net {

class EventLoop;

class Channel : Noncopyable {
public:
  typedef std::function<void()>          EventCallback;
  typedef std::function<void(TimeStamp)> ReadEventCallback;

  Channel(EventLoop *loop, int fd);
  ~Channel();

  void handleEvent(TimeStamp receieveTime);

  void setReadCallback(ReadEventCallback cb) {
    readCallback_ = std::move(cb);
  }

  void setWriteCallback(EventCallback cb) {
    writeCallback_ = std::move(cb);
  }

  void setCloseCallback(EventCallback cb) {
    closeCallback_ = std::move(cb);
  }

  void setErrorCallback(EventCallback cb) {
    errorCallback_ = std::move(cb);
  }

  void tie(const std::shared_ptr<void> &);

  int fd() const {
    return fd_;
  }

  int events() const {
    return events_;
  }

  void set_revents(int recv) {
    revents_ = recv;
  }

  bool isNoneEvent() const {
    return events_ == kNoneEvent;
  }

  void enableReading() {
    events_ |= kReadEvent;
    update();
  }

  void disableReading() {
    events_ &= ~kReadEvent;
    update();
  }

  void enableWriting() {
    events_ |= kWriteEvent;
    update();
  }

  void disableWriting() {
    events_ &= ~kWriteEvent;
    update();
  }

  void disableAll() {
    events_ = kNoneEvent;
    update();
  }

  bool isWriting() const {
    return events_ & kWriteEvent;
  }

  bool isReading() const {
    return events_ & kReadEvent;
  }

  int index() {
    return index_;
  }

  void set_index(int idx) {
    index_ = idx;
  }

  string eventsToString() const;
  string reventsToString() const;

  void doNotLogHub() {
    logHub_ = false;
  }

  EventLoop *ownloop() {
    return loop_;
  }

  void remove();

private:
  static string eventsToString(int fd, int ev);

  void update();
  void handleEventWithGuard(TimeStamp receieveTime);

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop *loop_;     // 所属EventLoop
  const int  fd_;       // 文件描述符，不负责关闭
  int        events_;   // 关注事件
  int        revents_;  // poll/epoll事件的返回
  int        index_;    // 表示poller事件数组中的序号
  bool       logHub_;

  std::weak_ptr<void> tie_;
  bool                tied_;
  bool                eventHandling_;  // 是否处在事件中
  bool                addedToLoop_;

  ReadEventCallback readCallback_;
  EventCallback     writeCallback_;
  EventCallback     closeCallback_;
  EventCallback     errorCallback_;
};  // class Channel

}  // namespace net

}  // namespace muduo

#endif /* __CHANNEL_H__ */
