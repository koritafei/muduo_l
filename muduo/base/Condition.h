#ifndef __CONDITION_H__
#define __CONDITION_H__

#include <pthread.h>

#include "Mutex.h"
#include "Noncopyable.h"

namespace muduo {

class Condition : Noncopyable {
public:
  explicit Condition(MutexLock& mutex) : mutex_(mutex) {
    MCHECK(pthread_cond_init(&pcond_, nullptr));
  }

  ~Condition() {
    MCHECK(pthread_cond_destroy(&pcond_));
  }

  void wait() {
    MutexLock::UnassignGuard ug(mutex_);
    MCHECK(pthread_cond_wait(&pcond_, mutex_.getPthreadMutex()));
  }

  bool waitForSeconds(double seconds);

  void notify() {
    MCHECK(pthread_cond_signal(&pcond_));
  }

  void notifyAll() {
    MCHECK(pthread_cond_broadcast(&pcond_));
  }

private:
  MutexLock&     mutex_;
  pthread_cond_t pcond_;
};  // class Condition

}  // namespace muduo

#endif /* __CONDITION_H__ */
