#ifndef __COUNTDOWNLATCH_H__
#define __COUNTDOWNLATCH_H__

#include "Condition.h"
#include "Mutex.h"
#include "Noncopyable.h"

namespace muduo {

class CountDownLatch : Noncopyable {
public:
  explicit CountDownLatch(int count);

  void wait();
  void countDown();
  int  getCount() const;

private:
  mutable MutexLock mutex_;
  Condition cond_   GUARDED_BY(mutex_);
  int count_        GUARDED_BY(mutex_);
};  // class CountDownLatch

}  // namespace muduo

#endif /* __COUNTDOWNLATCH_H__ */
