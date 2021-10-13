#include "CountDownLatch.h"

#include "muduo/base/Mutex.h"

using namespace muduo;

CountDownLatch::CountDownLatch(int count)
    : mutex_(), cond_(mutex_), count_(count) {
}

int CountDownLatch::getCount() const {
  MutexLockGuard lock(mutex_);
  return count_;
}

void CountDownLatch::countDown() {
  MutexLockGuard lock(mutex_);
  --count_;
  if (0 == count_) {
    cond_.notifyAll();
  }
}

void CountDownLatch::wait() {
  MutexLockGuard lock(mutex_);
  while (0 < count_) {
    cond_.wait();
  }
}
