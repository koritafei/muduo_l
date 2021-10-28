#include "EventLoop.h"

#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <algorithm>

#include "../base/Logging.h"
#include "../base/Mutex.h"
#include "Channel.h"
#include "Poller.h"
#include "SocketsOps.h"
#include "TimerQueue.h"

using namespace muduo;
using namespace muduo::net;

namespace {

__thread EventLoop* t_loopInThisThread = 0;
const int           kPollTimeMs        = 10000;

int createEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (0 > evtfd) {
    LOG_SYSERR << "Failed in eventfd";
    abort();
  }

  return evtfd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe {
public:
  IgnoreSigPipe() {
    ::signal(SIGPIPE, SIG_IGN);
  }
};  // class IgnoreSigPipe
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;

}  // namespace

EventLoop* EventLoop::getEventLoopOfCurrentLoop() {
  return t_loopInThisThread;
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      iteration_(0),
      threadId_(CurrentThread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      timerQueue_(new TimerQueue(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      currentActiveChannel_(nullptr) {
  LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;

  if (t_loopInThisThread) {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
              << " exists in this thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }

  wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
  LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
            << "destructs in thread " << CurrentThread::tid();
  wakeupChannel_->disableAll();
  wakeupChannel_->remove();
  ::close(wakeupFd_);
  t_loopInThisThread = nullptr;
}

/**
 * @brief 事件循环，不可跨线程调用，只能创建该对象的线程中调用
 * */
void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_    = false;

  LOG_TRACE << "EventLoop " << this << " start looping";

  while (!quit_) {
    activeChannels_.clear();
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
    ++iteration_;

    if (Logger::LogLevel() <= Logger::TRACE) {
      printfActiveChannels();
    }

    eventHandling_ = true;

    for (Channel* channel : activeChannels_) {
      currentActiveChannel_ = channel;
      currentActiveChannel_->handleEvent(pollReturnTime_);
    }

    currentActiveChannel_ = nullptr;
    eventHandling_        = false;
    doPendingFunctors();
  }

  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  if (!isInLoopThread()) {
    wakeup();
  }
}

void EventLoop::runInLoop(Functor cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(std::move(cb));
  }
}

void EventLoop::queueInLoop(Functor cb) {
  {
    MutexLockGuard lock(mutex_);
    pendingFunctor_.push_back(std::move(cb));
  }

  if (!isInLoopThread() || callingPendingFunctors_) {
    wakeup();
  }
}

size_t EventLoop::queueSize() const {
  MutexLockGuard lock(mutex_);
  return pendingFunctor_.size();
}

TimerId EventLoop::runAt(TimeStamp time, TimerCallback cb) {
  return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb) {
  TimeStamp time(addTime(TimeStamp::now(), delay));

  return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb) {
  TimeStamp time(addTime(TimeStamp::now(), interval));

  return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId timerId) {
  return timerQueue_->cancel(timerId);
}

void EventLoop::updateChannel(Channel* channel) {
  assert(channel->ownloop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
  assert(channel->ownloop() == this);
  assertInLoopThread();

  if (eventHandling_) {
    assert(currentActiveChannel_ == channel ||
           std::find(activeChannels_.begin(), activeChannels_.end(), channel) ==
               activeChannels_.end());
  }

  poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel) {
  assert(channel->ownloop() == this);
  assertInLoopThread();

  return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " << CurrentThread::tid();
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t  n   = sockets::write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t  n   = sockets::read(wakeupFd_, &one, sizeof one);

  if (n != sizeof one) {
    LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
    MutexLockGuard lock(mutex_);
    functors.swap(pendingFunctor_);
  }

  for (const Functor& functor : functors) {
    functor();
  }

  callingPendingFunctors_ = false;
}

void EventLoop::printfActiveChannels() const {
  for (const Channel* channel : activeChannels_) {
    LOG_TRACE << "{" << channel->reventsToString() << "}";
  }
}
