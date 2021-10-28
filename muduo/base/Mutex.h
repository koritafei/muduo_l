#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <assert.h>
#include <pthread.h>

#include "CurrentThread.h"
#include "Noncopyable.h"

#if defined(__clang__) && !defined(SWIG)
#define THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)
#endif

#define CAPABILITY(x)     THREAD_ANNOTATION_ATTRIBUTE__(capability(x))
#define SCOPED_CAPABILITY THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define GUARDED_BY(x)    THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))
#define PT_GUARDED_BY(x) THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define ACQUIRED_BEFOR(...)                                                    \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))
#define ACQUIRED_AFTER(...)                                                    \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define REQUIRES(...)                                                          \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))
#define REQUIRED_SHARED(...)                                                   \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(x))

#define ACQUIRE(...)                                                           \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))
#define ACQUIRE_SHARED(...)                                                    \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define RELEASE(...)                                                           \
  THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))
#define RELEASE_SHARED(...)                                                    \
  THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define TRY_ACQUIRE(...)                                                       \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))
#define TRY_ACQUIRE_SHARED(...)                                                \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability_(__VA_ARGS__))

#define EXCLUDES(...) THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define ASSERT_CAPABILITY(x) THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))
#define ASSER_SHARED_CAPABILITY(x)                                             \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define RETURN_CAPABILITY(x) THREAD_ANNOTATION_ATTRIBUTE__(lock_return(x))

#define NO_THREAD_SAFETY_ANALYSIS                                              \
  THREAD_ANNOTATION_ATTRIBUTE__(no_return_safety_analisis)

#define MCHECK(ret)                                                            \
  ({                                                                           \
    __typeof__(ret) errnum = (ret);                                            \
    if (__builtin_expect(errnum != 0, 0)) {                                    \
      __assert_perror_fail(errnum, __FILE__, __LINE__, __func__);              \
    }                                                                          \
  })

namespace muduo {

class CAPABILITY("mutex") MutexLock : Noncopyable {
public:
  MutexLock() : holder_(0) {
    MCHECK(pthread_mutex_init(&mutex_, nullptr));
  }

  ~MutexLock() {
    assert(0 == holder_);
    MCHECK(pthread_mutex_destroy(&mutex_));
  }

  bool isLockedByThisThread() const {
    return holder_ == CurrentThread::tid();
  }

  void assertLocked() const ASSERT_CAPABILITY(this) {
    assert(isLockedByThisThread());
  }

  void lock() ACQUIRE() {
    MCHECK(pthread_mutex_lock(&mutex_));
    assignHolder();
  }

  void unlock() RELEASE() {
    unassignHolder();
    MCHECK(pthread_mutex_unlock(&mutex_));
  }

  pthread_mutex_t *getPthreadMutex() {
    return &mutex_;
  }

private:
  friend class Condition;
  pthread_mutex_t mutex_;
  pid_t           holder_;

  class UnassignGuard : Noncopyable {
  public:
    explicit UnassignGuard(MutexLock &owner) : owner_(owner) {
      owner_.unassignHolder();
    }

    ~UnassignGuard() {
      owner_.assignHolder();
    }

  private:
    MutexLock &owner_;
  };  // class

  void unassignHolder() {
    holder_ = 0;
  }

  void assignHolder() {
    holder_ = CurrentThread::tid();
  }

};  // class MutexLock

class SCOPED_CAPABILITY MutexLockGuard : Noncopyable {
public:
  explicit MutexLockGuard(MutexLock &mutex) ACQUIRE(mutex) : mutex_(mutex) {
    mutex_.lock();
  }

  ~MutexLockGuard() {
    mutex_.unlock();
  }

private:
  MutexLock &mutex_;
};  // class MutexLockGuard

}  // namespace muduo

#endif /* __MUTEX_H__ */
