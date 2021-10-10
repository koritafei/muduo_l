#include "Thread.h"

#include <errno.h>
#include <linux/unistd.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <ctime>
#include <type_traits>

#include "TimeStamp.h"

namespace muduo {

namespace detail {

pid_t gettid() {
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

void afterFork() {
  muduo::CurrentThread::t_cachedTid = 0;
  muduo::CurrentThread::t_theadName = "main";
  CurrentThread::tid();
}

}  // namespace detail

void CurrentThread::cacheTid() {
  if (0 == t_cachedTid) {
    t_cachedTid = detail::gettid();
    t_tidStringLength =
        snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
  }
}

bool CurrentThread::isMainThread() {
  return tid() == ::getpid();
}

void CurrentThread::sleepUsec(int64_t usec) {
  struct timespec ts = {0, 0};

  ts.tv_sec = static_cast<time_t>(usec / TimeStamp::kMicroSecondsPerSecond);
  ts.tv_nsec =
      static_cast<long>(usec % TimeStamp::kMicroSecondsPerSecond * 1000);
  ::nanosleep(&ts, nullptr);
}

}  // namespace muduo
