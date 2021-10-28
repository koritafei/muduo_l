#ifndef __TIMERID_H__
#define __TIMERID_H__

#include <bits/stdint-intn.h>

#include "../base/Copyable.h"

namespace muduo {

namespace net {

class Timer;

class TimerId : public muduo::Copyable {
public:
  TimerId() : timer_(nullptr), sequence_(0) {
  }

  TimerId(Timer* timer, int64_t sequence) : timer_(timer), sequence_(sequence) {
  }

  friend class TimerQueue;

private:
  Timer*  timer_;
  int64_t sequence_;
};  // class TimerId

}  // namespace net

}  // namespace muduo

#endif /* __TIMERID_H__ */
