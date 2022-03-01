#include "Thread.h"

#include <linux/prctl.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <cstdlib>

#if __FreeBSD__
#include <pthread_np.h>
#else
#include <linux/unistd.h>
#include <sys/prctl.h>
#endif

namespace muduo {

namespace CurrentThread {

__thread const char* t_threadName = "unnameThread";

}

}  // namespace muduo

namespace {

__thread pid_t t_cachedTid = 0;

#if __FreeBSD__

pid_t getpid() {
  return pthread_getpthreadid_np();
}
#else

#if !__GLIBC_PREREQ(2, 30)
pid_t getpid() {
  return static_cast<pid_t>(::SYS_gettid);
}
#endif

#endif

void afterFork() {
  t_cachedTid                        = gettid();
  muduo::CurrentThread::t_threadName = "main";
}

class ThreadNameInitializer {
public:
  ThreadNameInitializer() {
    muduo::CurrentThread::t_threadName = "main";
    pthread_atfork(NULL, NULL, afterFork);
  }
};

ThreadNameInitializer init;

struct ThreadData {
  typedef muduo::Thread::ThreadFunc ThreadFunc;

  ThreadFunc             func_;
  std::string            name_;
  boost::weak_ptr<pid_t> wkTid_;

  ThreadData(const ThreadFunc&             func,
             const std::string&            name,
             const boost::weak_ptr<pid_t>& tid)
      : func_(func), name_(name), wkTid_(tid) {
  }

  void runThread() {
    pid_t                    tid  = muduo::CurrentThread::tid();
    boost::shared_ptr<pid_t> ptid = wkTid_.lock();

    if (ptid) {
      *ptid = tid;
      ptid.reset();
    }

    if (name_.empty()) {
      muduo::CurrentThread::t_threadName = name_.c_str();
    }

#if __FreeBSD__
    pthread_setname_np(pthread_self(), muduo::CurrentThread::t_threadName);
#else
    ::prctl(PR_SET_NAME, muduo::CurrentThread::t_threadName);
#endif
    func_();
    muduo::CurrentThread::t_threadName = "finished";
  }
};

void* startThread(void* obj) {
  ThreadData* data = static_cast<ThreadData*>(obj);
  data->runThread();
  delete data;
  return NULL;
}

}  // namespace

using namespace muduo;

pid_t CurrentThread::tid() {
  if (0 == t_cachedTid) {
    t_cachedTid = gettid();
  }

  return t_cachedTid;
}

const char* CurrentThread::name() {
  return t_threadName;
}

bool CurrentThread::isMainThread() {
  return tid() == ::getpid();
}

AtomicInt32 Thread::numCreated_;

Thread::Thread(const ThreadFunc& func, const std::string& name)
    : started_(false),
      joined_(false),
      pthreadId_(0),
      tid_(new pid_t(0)),
      func_(func),
      name_(name) {
  numCreated_.increment();
}

Thread::~Thread() {
  if (started_ && !joined_) {
    pthread_detach(pthreadId_);
  }
}

void Thread::start() {
  assert(!started_);
  started_ = true;

  ThreadData* data = new ThreadData(func_, name_, tid_);

  if (pthread_create(&pthreadId_, NULL, &startThread, data)) {
    started_ = false;
    delete data;
    abort();
  }
}

void Thread::join() {
  assert(started_);
  assert(!joined_);
  joined_ = true;
  pthread_join(pthreadId_, NULL);
}