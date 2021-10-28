#include "TimerQueue.h"

#include <bits/types/struct_itimerspec.h>
#include <bits/types/struct_timespec.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iterator>

#include "../base/Logging.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

namespace muduo {

namespace net {

namespace detail {

int createTimerfd() {
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

  if (0 > timerfd) {
    LOG_SYSFATAL << "Failed in  timerfd_create";
  }

  return timerfd;
}

struct timespec howMuchTimeromow(TimeStamp when) {
  int64_t microseconds =
      when.microSecondsSinceEpoch() - TimeStamp::now().microSecondsSinceEpoch();

  if (100 > microseconds) {
    microseconds = 100;
  }

  struct timespec ts;
  ts.tv_sec =
      static_cast<time_t>(microseconds / TimeStamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(
      (microseconds % TimeStamp::kMicroSecondsPerSecond) * 1000);
  return ts;
}

void readTimerfd(int timerfd, TimeStamp now) {
  uint64_t howmany;
  ssize_t  n = ::read(timerfd, &howmany, sizeof howmany);
  LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at "
            << now.toString();

  if (n != sizeof howmany) {
    LOG_ERROR << "TimeQueue::handleRead() reads " << n
              << " bytes instead of 8.";
  }
}

void resetTimerfd(int timerfd, TimeStamp expiration) {
  struct itimerspec oldvalue;
  struct itimerspec newvalue;

  memZero(&oldvalue, sizeof oldvalue);
  memZero(&newvalue, sizeof newvalue);

  newvalue.it_value = howMuchTimeromow(expiration);
  int ret           = ::timerfd_settime(timerfd, 0, &oldvalue, &newvalue);

  if (ret) {
    LOG_SYSERR << "timerfd_settime";
  }
}

}  // namespace detail

}  // namespace net

}  // namespace muduo

using namespace muduo;
using namespace muduo::net;
using namespace muduo::net::detail;

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop),
      timerfd_(createTimerfd()),
      timerfdChannel_(loop, timerfd_),
      timers_(),
      callingExpiredTimers_(false) {
  timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
  timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
  timerfdChannel_.disableAll();
  timerfdChannel_.remove();
  ::close(timerfd_);

  for (const Entry &timer : timers_) {
    delete timer.second;
  }
}

TimerId TimerQueue::addTimer(TimerCallback cb,
                             TimeStamp     when,
                             double        interval) {
  Timer *timer = new Timer(std::move(cb), when, interval);

  loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));

  return TimerId(timer, timer->sequence());
}

void TimerQueue::cancel(TimerId timerId) {
  loop_->runInLoop(std::bind(&TimerQueue::cancel, this, timerId));
}

void TimerQueue::addTimerInLoop(Timer *timer) {
  loop_->assertInLoopThread();
  bool earliestChangd = insert(timer);

  if (earliestChangd) {
    resetTimerfd(timerfd_, timer->exprication());
  }
}

void TimerQueue::cancelInLoop(TimerId timerId) {
  loop_->assertInLoopThread();
  assert(timers_.size() == activeTimers_.size());
  ActiveTimer              timer(timerId.timer_, timerId.sequence_);
  ActiveTimerSet::iterator it = activeTimers_.find(timer);

  if (it != activeTimers_.end()) {
    size_t n = timers_.erase(Entry(it->first->exprication(), it->first));
    assert(1 == n);
    (void)n;

    delete it->first;
    activeTimers_.erase(it);
  } else if (callingExpiredTimers_) {
    cancelingTimers_.insert(timer);
  }
  assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::handleRead() {
  loop_->assertInLoopThread();
  TimeStamp now(TimeStamp::now());

  readTimerfd(timerfd_, now);
  std::vector<Entry> expired = getExpired(now);

  callingExpiredTimers_ = true;
  cancelingTimers_.clear();

  for (const Entry &it : expired) {
    it.second->run();
  }

  callingExpiredTimers_ = false;
  reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(TimeStamp now) {
  assert(timers_.size() == activeTimers_.size());
  std::vector<Entry>  expired;
  Entry               sentry(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
  TimerList::iterator end = timers_.lower_bound(sentry);
  assert(end == timers_.end() || now < end->first);
  std::copy(timers_.begin(), end, back_inserter(expired));

  timers_.erase(timers_.begin(), end);

  for (const Entry &it : expired) {
    ActiveTimer timer(it.second, it.second->sequence());
    size_t      n = activeTimers_.erase(timer);

    assert(1 == n);
    (void)n;
  }

  assert(timers_.size() == activeTimers_.size());
  return expired;
}

void TimerQueue::reset(const std::vector<Entry> &expired, TimeStamp now) {
  TimeStamp nextExpire;

  for (const Entry &it : expired) {
    ActiveTimer timer(it.second, it.second->sequence());

    if (it.second->repeat() &&
        cancelingTimers_.find(timer) == cancelingTimers_.end()) {
      it.second->restart(now);
      insert(it.second);
    } else {
      delete it.second;
    }
  }

  if (!timers_.empty()) {
    nextExpire = timers_.begin()->second->exprication();
  }

  if (nextExpire.valid()) {
    resetTimerfd(timerfd_, nextExpire);
  }
}

bool TimerQueue::insert(Timer *timer) {
  loop_->assertInLoopThread();
  assert(timers_.size() == activeTimers_.size());
  bool      earliestChanged = false;
  TimeStamp when            = timer->exprication();

  TimerList::iterator it = timers_.begin();
  if (timers_.end() == it || when < it->first) {
    earliestChanged = true;
  }

  {
    std::pair<TimerList::iterator, bool> result =
        timers_.insert(Entry(when, timer));
    assert(result.second);
    (void)result;
  }
  {
    std::pair<ActiveTimerSet::iterator, bool> result =
        activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
    assert(result.second);
    (void)result;
  }
  assert(timers_.size() == activeTimers_.size());

  return earliestChanged;
}
