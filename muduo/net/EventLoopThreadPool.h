#ifndef __EVENTLOOPTHREADPOOL__H__
#define __EVENTLOOPTHREADPOOL__H__

#include <functional>
#include <memory>
#include <vector>

#include "../base/Noncopyable.h"
#include "../base/Types.h"

namespace muduo {

namespace net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : Noncopyable {
public:
  typedef std::function<void(EventLoop *)> ThreadInitCallback;

  EventLoopThreadPool(EventLoop *loop, const string &nameArg);
  ~EventLoopThreadPool();

  void setNumthreads(int numthreads) {
    numThreads_ = numthreads;
  }

  void start(const ThreadInitCallback &cb = ThreadInitCallback());

  EventLoop *getNextLoop();
  EventLoop *getLoopForHash(size_t hashCode);

  std::vector<EventLoop *> getAlLoops();

  bool started() const {
    return started_;
  }

  const string &name() const {
    return name_;
  }

private:
  EventLoop *                                   baseLoop_;
  string                                        name_;
  bool                                          started_;
  int                                           numThreads_;
  int                                           next_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop *>                      loops_;
};  // class EventLoopThreadPool

}  // namespace net

}  // namespace muduo

#endif /* __EVENTLOOPTHREADPOOL__H__ */
