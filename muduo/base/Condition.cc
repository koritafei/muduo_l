#include "Condition.h"

#include <asm-generic/errno.h>
#include <errno.h>
#include <pthread.h>

#include <ctime>

bool muduo::Condition::waitForSeconds(double seconds) {
  struct timespec abstime;

  clock_gettime(CLOCK_REALTIME, &abstime);
  const int kNanoSecondsPerSecond = 1000 * 1000;

  int64_t nanoSeconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);

  abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoSeconds) /
                                        kNanoSecondsPerSecond);
  abstime.tv_nsec = static_cast<int64_t>((abstime.tv_nsec + nanoSeconds) %
                                         kNanoSecondsPerSecond);

  MutexLock::UnassignGuard ug(mutex_);

  return ETIMEDOUT ==
         pthread_cond_timedwait(&pcond_, mutex_.getPthreadMutex(), &abstime);
}