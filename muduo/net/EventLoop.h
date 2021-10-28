#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__

#include <atomic>
#include <boost/any.hpp>
#include <functional>
#include <memory>
#include <vector>

#include "../base/CurrentThread.h"
#include "../base/Mutex.h"
#include "../base/TimeStamp.h"
#include "Callbacks.h"
#include "TimerId.h"

namespace muduo {

namespace net {

class Channel;
class Poller;
class TimerQueue;

class EventLoop : Noncopyable {
public:
  typedef std::function<void()> Functor;

  EventLoop();
  ~EventLoop();

  void loop();
  void quit();

  TimeStamp pollReturnTime() const {
    return pollReturnTime_;
  }

  int64_t iteration() const {
    return iteration_;
  }

  void runInLoop(Functor cb);

  void queueInLoop(Functor cb);

  size_t queueSize() const;

  TimerId runAt(TimeStamp time, TimerCallback cb);

  TimerId runAfter(double delay, TimerCallback cb);

  TimerId runEvery(double interval, TimerCallback cb);

  void cancel(TimerId timerId);

  void wakeup();

  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);
  bool hasChannel(Channel *channel);

  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }

  bool isInLoopThread() const {
    return CurrentThread::tid() == threadId_;
  }

  bool eventHandling() const {
    return eventHandling_;
  }

  void setContext(const boost::any &context) {
    context_ = context;
  }

  const boost::any getContext() const {
    return context_;
  }

  boost::any *getMutableContext() {
    return &context_;
  }

  static EventLoop *getEventLoopOfCurrentLoop();

private:
  void abortNotInLoopThread();
  void handleRead();
  void doPendingFunctors();

  void printfActiveChannels() const;

  typedef std::vector<Channel *> ChannelList;
  bool                           looping_;
  std::atomic<bool>              quit_;
  bool                           eventHandling_;
  bool                           callingPendingFunctors_;
  int64_t                        iteration_;
  const pid_t                    threadId_;  // 当前线程所属ID
  TimeStamp                      pollReturnTime_;
  std::unique_ptr<Poller>        poller_;
  std::unique_ptr<TimerQueue>    timerQueue_;
  int                            wakeupFd_;
  std::unique_ptr<Channel>       wakeupChannel_;
  boost::any                     context_;
  ChannelList activeChannels_;        // Poller返回的活动通道
  Channel *   currentActiveChannel_;  // 当前正在处理的活动通道

  mutable MutexLock                    mutex_;
  std::vector<Functor> pendingFunctor_ GUARDED_BY(mutex_);
};  // class EventLoop

}  // namespace net

}  // namespace muduo

#endif /* __EVENTLOOP_H__ */
