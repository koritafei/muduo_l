#ifndef __TIMER_H__
#define __TIMER_H__

#include "../base/Atomic.h"
#include "../base/TimeStamp.h"
#include "Callbacks.h"

namespace muduo {

namespace net {

class Timer : Noncopyable {
public:
  Timer(TimerCallback cb, TimeStamp when, double interval)
      : callback_(std::move(cb)),
        exprication_(when),
        interval_(interval),
        repeat_(interval > 0.0),
        sequence_(s_numCreated_.incrementAndGet()) {
  }

  void run() const {
    callback_();
  }

  TimeStamp exprication() const {
    return exprication_;
  }

  bool repeat() const {
    return repeat_;
  }

  int64_t sequence() const {
    return sequence_;
  }

  void restart(TimeStamp now);

  static int64_t numCreated() {
    return s_numCreated_.get();
  }

private:
  const TimerCallback callback_;
  TimeStamp           exprication_;
  const double        interval_;
  const bool          repeat_;
  const int64_t       sequence_;

  static AtomicInt64 s_numCreated_;

};  // class Timer

}  // namespace net

}  // namespace muduo

#endif /* __TIMER_H__ */
