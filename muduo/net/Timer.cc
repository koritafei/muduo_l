#include "Timer.h"

using namespace muduo;
using namespace muduo::net;

AtomicInt64 Timer::s_numCreated_;

void Timer::restart(TimeStamp now) {
  if (repeat_) {
    exprication_ = addTime(now, interval_);
  } else {
    exprication_ = TimeStamp::invalid();
  }
}
