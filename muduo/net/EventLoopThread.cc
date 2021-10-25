#include "EventLoopThread.h"

#include "EventLoop.h"

using namespace muduo;
using namespace muduo::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb,
                                 const string &            name)
    : loop_(nullptr),
      exiting_(false),
      thread_(std::bind(&EventLoopThread::threadFunc, this), name),
      mutex_(),
      cond_(mutex_),
      callback_(cb) {
}

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  if (loop_ != nullptr) {
    loop_->quit();
    thread_.join();
  }
}

EventLoop *EventLoopThread::startLoop() {
  assert(!thread_.started());
  thread_.start();

  EventLoop *loop = nullptr;
  {
    MutexLockGuard lock(mutex_);
    while (nullptr == loop_) {
      cond_.wait();
    }

    loop = loop_;
  }
  return loop;
}

void EventLoopThread::threadFunc() {
  EventLoop loop;

  if (callback_) {
    callback_(&loop);
  }

  {
    MutexLockGuard lock(mutex_);
    loop_ = &loop;
    cond_.notify();
  }
  loop.loop();
  MutexLockGuard lock(mutex_);
  loop_ = nullptr;
}
