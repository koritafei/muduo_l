#ifndef __MUTEX__H__
#define __MUTEX__H__

#include <assert.h>
#include <pthread.h>

#include <boost/core/noncopyable.hpp>
#include <boost/noncopyable.hpp>

#include "Thread.h"

namespace muduo {

class MutexLock : public boost::noncopyable {
public:
  MutexLock() : holder_(0) {
    pthread_mutex_init(&mutex_, NULL);
  }

  ~MutexLock() {
    assert(0 == holder_);
    pthread_mutex_destroy(&mutex_);
  }

  bool isLockedByThisThread() {
    return holder_ == CurrentThread::tid();
  }

  void assertLocked() {
    assert(isLockedByThisThread());
  }

  void lock() {
    pthread_mutex_lock(&mutex_);
    holder_ = CurrentThread::tid();
  }

  void unlock() {
    holder_ = 0;
    pthread_mutex_unlock(&mutex_);
  }

  pthread_mutex_t *getPthreadMutex() {
    return &mutex_;
  }

private:
  pthread_mutex_t mutex_;
  pid_t           holder_;
};

class MutexLockGuard : public boost::noncopyable {
public:
  explicit MutexLockGuard(MutexLock &mutex) : mutex_(mutex) {
    mutex_.lock();
  }

  ~MutexLockGuard() {
    mutex_.unlock();
  }

private:
  MutexLock &mutex_;
};

}  // namespace muduo

#define MutexLockGuard(x) error "Missing guard object name"

#endif /* __MUTEX__H__ */
