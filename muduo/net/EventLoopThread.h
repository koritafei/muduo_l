#ifndef __EVENTLOOPTHREAD__H__
#define __EVENTLOOPTHREAD__H__

#include "../base/Condition.h"
#include "../base/Mutex.h"
#include "../base/Thread.h"

namespace muduo {

namespace net {

class EventLoop;

class EventLoopThread : Noncopyable {
public:
  typedef std::function<void(EventLoop *)> ThreadInitCallback;

  EventLoopThread(const ThreadInitCallback &cb   = ThreadInitCallback(),
                  const string &            name = string());
  ~EventLoopThread();

  EventLoop *startLoop();

private:
  void threadFunc();

  MutexLock          mutex_;
  EventLoop *loop_   GUARDED_BY(mutex_);
  bool               exiting_;
  Thread             thread_;
  Condition cond_    GUARDED_BY(mutex_);
  ThreadInitCallback callback_;
};

}  // namespace net

}  // namespace muduo

#endif /* __EVENTLOOPTHREAD__H__ */
