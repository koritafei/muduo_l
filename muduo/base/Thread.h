#ifndef __THREAD__H__
#define __THREAD__H__

#include <boost/core/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/function/function_fwd.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

#include "Atomic.h"

namespace muduo {

class Thread : public boost::noncopyable {
public:
  typedef boost::function<void()> ThreadFunc;

  explicit Thread(const ThreadFunc &, const std::string &name = std::string());

  ~Thread();

  void start();
  void join();

  bool started() const {
    return started_;
  }

  pid_t tid() const {
    return *tid_;
  }

  const std::string name() const {
    return name_;
  }

  static int numCreated() {
    return numCreated_.get();
  }

private:
  bool                     started_;
  bool                     joined_;
  pthread_t                pthreadId_;
  boost::shared_ptr<pid_t> tid_;
  ThreadFunc               func_;
  std::string              name_;

  static AtomicInt32 numCreated_;
};

namespace CurrentThread {
pid_t       tid();
const char *name();
bool        isMainThread();
}  // namespace CurrentThread

}  // namespace muduo

#endif /* __THREAD__H__ */
