#include "Thread.h"

#include <errno.h>
#include <linux/unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <ctime>
#include <type_traits>

#include "Exception.h"
#include "Logging.h"
#include "TimeStamp.h"
namespace muduo {

namespace detail {

pid_t gettid() {
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

void afterFork() {
  muduo::CurrentThread::t_cachedTid  = 0;
  muduo::CurrentThread::t_threadName = "main";
  CurrentThread::tid();
}

class ThreadNameInitializer {
public:
  ThreadNameInitializer() {
    muduo::CurrentThread::t_threadName = "main";
    CurrentThread::tid();
    pthread_atfork(NULL, NULL, &afterFork);
  }
};  // class ThreadNameInitializer

ThreadNameInitializer init;

struct ThreadData {
  typedef muduo::Thread::ThreadFunc ThreadFunc;
  ThreadFunc                        func_;
  std::string                       name_;
  pid_t*                            tid_;
  CountDownLatch*                   latch_;

  ThreadData(ThreadFunc         func,
             const std::string& name,
             pid_t*             tid,
             CountDownLatch*    latch)
      : func_(std::move(func)), name_(name), tid_(tid), latch_(latch) {
  }

  void runThread() {
    *tid_ = muduo::CurrentThread::tid();
    tid_  = NULL;
    latch_->countDown();
    latch_ = NULL;

    muduo::CurrentThread::t_threadName =
        name_.empty() ? "mainThread" : name_.c_str();
    ::prctl(PR_SET_NAME, muduo::CurrentThread::t_threadName);

    try {
      func_();
      muduo::CurrentThread::t_threadName = "finished";
    } catch (const Exception& ex) {
      muduo::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason %s\n", ex.what());
      fprintf(stderr, "stack trace :%s\n", ex.stackTrace());
      abort();
    } catch (const std::exception& ex) {
      muduo::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason %s\n", ex.what());
      abort();
    } catch (...) {
      muduo::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "unkonwn exception caught in Thread %s\n", name_.c_str());
      throw;
    }
  }

};  // struct ThreadData

void* startThread(void* obj) {
  ThreadData* data = static_cast<ThreadData*>(obj);
  data->runThread();
  delete data;
  return nullptr;
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

AtomicInt32 Thread::numCreated_;

Thread::Thread(ThreadFunc func, const std::string& n)
    : started_(false),
      joined_(false),
      pthreadId_(0),
      tid_(0),
      func_(std::move(func)),
      name_(n),
      latch_(1) {
  setDefaultName();
}

Thread::~Thread() {
  if (started_ && !joined_) {
    pthread_detach(pthreadId_);
  }
}

void Thread::setDefaultName() {
  int num = numCreated_.incrementAndGet();
  if (name_.empty()) {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread%d", num);
    name_ = buf;
  }
}

void Thread::start() {
  assert(!started_);
  started_ = true;

  detail::ThreadData* data =
      new detail::ThreadData(func_, name_, &tid_, &latch_);

  if (pthread_create(&pthreadId_, NULL, &detail::startThread, data)) {
    started_ = false;
    delete data;
    LOG_SYSFATAL << "Failed in pthread_create";
  } else {
    latch_.wait();
    assert(tid_ > 0);
  }
}

int Thread::join() {
  assert(started_);
  assert(!joined_);
  joined_ = true;

  return pthread_join(pthreadId_, nullptr);
}

}  // namespace muduo
