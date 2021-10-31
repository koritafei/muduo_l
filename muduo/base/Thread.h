#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>

#include <functional>
#include <memory>

#include "Atomic.h"
#include "CountDownLatch.h"
#include "CurrentThread.h"
#include "Types.h"

namespace muduo {

class Thread : Noncopyable {
public:
  typedef std::function<void()> ThreadFunc;

  explicit Thread(ThreadFunc, const std::string &name = string());

  ~Thread();

  void start();
  int  join();

  bool started() const {
    return started_;
  }

  pid_t tid() const {
    return tid_;
  }

  const std::string &name() const {
    return name_;
  }

  static int numCreated() {
    return numCreated_.get();
  }

private:
  bool        started_;
  bool        joined_;
  pthread_t   pthreadId_;
  pid_t       tid_;
  ThreadFunc  func_;
  std::string name_;

  CountDownLatch latch_;

  static AtomicInt32 numCreated_;

  void setDefaultName();

};  // class Thread

}  // namespace muduo

#endif /* __THREAD_H__ */
