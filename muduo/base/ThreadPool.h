#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <deque>
#include <vector>

#include "Condition.h"
#include "Mutex.h"
#include "Noncopyable.h"
#include "Thread.h"
#include "Types.h"

namespace muduo {

class ThreadPool : Noncopyable {
public:
  typedef std::function<void()> Task;

  explicit ThreadPool(const std::string nameArg = std::string("ThreadLocal"));
  ~ThreadPool();

  void setMaxQueueSize(int maxSize) {
    maxQueueSize_ = maxSize;
  }

  void setThreadInitCallback(Task &cb) {
    threadInitCallback_ = cb;
  }

  void start(int numThreads);
  void stop();

  const std::string &name() const {
    return name_;
  }

  size_t queueSize() const;

  void run(Task f);

private:
  mutable MutexLock                           mutex_;
  Condition notEmpty_                         GUARDED_BY(mutex_);
  Condition notFull_                          GUARDED_BY(mutex_);
  std::string                                 name_;
  Task                                        threadInitCallback_;
  std::vector<std::unique_ptr<muduo::Thread>> threads_;
  std::deque<Task> queue_                     GUARDED_BY(mutex_);
  size_t                                      maxQueueSize_;
  bool                                        running_;

  bool isFull() const GUARDED_BY(mutex_);

  void runInThread();

  Task take();

};  // class ThreadPool

}  // namespace muduo

#endif /* __THREADPOOL_H__ */
