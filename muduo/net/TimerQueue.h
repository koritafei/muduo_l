#ifndef __TIMERQUEUE_H__
#define __TIMERQUEUE_H__

#include <set>
#include <vector>

#include "../base/Mutex.h"
#include "../base/TimeStamp.h"
#include "Callbacks.h"
#include "Channel.h"

namespace muduo {

namespace net {

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : Noncopyable {
public:
  explicit TimerQueue(EventLoop *loop);
  ~TimerQueue();

  TimerId addTimer(TimerCallback cb, TimeStamp when, double interval);

  void cancel(TimerId timerId);

private:
  typedef std::pair<TimeStamp, Timer *> Entry;
  typedef std::set<Entry>               TimerList;
  typedef std::pair<Timer *, int64_t>   ActiveTimer;
  typedef std::set<ActiveTimer>         ActiveTimerSet;

  void addTimerInLoop(Timer *timer);
  void cancelInLoop(TimerId timerid);

  void handleRead();

  std::vector<Entry> getExpired(TimeStamp now);
  void               reset(const std::vector<Entry> &expired, TimeStamp now);
  bool               insert(Timer *timer);

  EventLoop *    loop_;
  const int      timerfd_;
  Channel        timerfdChannel_;
  TimerList      timers_;
  ActiveTimerSet activeTimers_;
  bool           callingExpiredTimers_;
  ActiveTimerSet cancelingTimers_;
};  // class TimerQueue

}  // namespace net

}  // namespace muduo

#endif /* __TIMERQUEUE_H__ */
