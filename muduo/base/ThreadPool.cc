#include "ThreadPool.h"

#include <assert.h>
#include <stdio.h>

#include "Exception.h"
#include "Mutex.h"

using namespace muduo;

ThreadPool::ThreadPool(const std::string nameArg)
    : mutex_(),
      notEmpty_(mutex_),
      notFull_(mutex_),
      name_(nameArg),
      maxQueueSize_(0),
      running_(false) {
}

ThreadPool::~ThreadPool() {
  if (running_) {
    stop();
  }
}

void ThreadPool::start(int numThreads) {
  assert(threads_.empty());
  running_ = true;
  threads_.reserve(numThreads);

  for (int i = 0; i < numThreads; ++i) {
    char buf[32];
    snprintf(buf, sizeof buf, "%d", i);
    threads_.emplace_back(
        new muduo::Thread(std::bind(&ThreadPool::runInThread, this),
                          name_ + buf));
    threads_[i]->start();
  }

  if (0 == numThreads && threadInitCallback_) {
    threadInitCallback_();
  }
}

void ThreadPool::stop() {
  {
    MutexLockGuard lock(mutex_);
    running_ = false;
    notEmpty_.notifyAll();
    notFull_.notifyAll();
  }

  for (const auto &thr : threads_) {
    thr->join();
  }
}

size_t ThreadPool::queueSize() const {
  MutexLockGuard lock(mutex_);
  return queue_.size();
}

void ThreadPool::run(Task task) {
  if (threads_.empty()) {
    task();
  } else {
    MutexLockGuard lock(mutex_);

    while (isFull() && running_) {
      notFull_.wait();
    }

    if (!running_) {
      return;
    }
    assert(!isFull());

    queue_.push_back(std::move(task));
    notEmpty_.notify();
  }
}

ThreadPool::Task ThreadPool::take() {
  MutexLockGuard lock(mutex_);

  while (queue_.empty() && running_) {
    notEmpty_.wait();
  }

  Task task;
  if (!queue_.empty()) {
    task = queue_.front();
    queue_.pop_front();
    if (0 < maxQueueSize_) {
      notFull_.notify();
    }
  }

  return task;
}

bool ThreadPool::isFull() const {
  mutex_.assertLocked();
  return 0 < maxQueueSize_ && queue_.size() >= maxQueueSize_;
}

void ThreadPool::runInThread() {
  try {
    if (threadInitCallback_) {
      threadInitCallback_();
    }

    while (running_) {
      Task task(take());
      if (task) {
        task();
      }
    }

  } catch (const Exception &ex) {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason %s\n", ex.what());
    fprintf(stderr, "stack trace %s\n", ex.stackTrace());
    abort();
  } catch (const std::exception &ex) {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason %s\n", ex.what());
    abort();
  } catch (...) {
    fprintf(stderr,
            "unknown exception caught in ThreadPool %s\n",
            name_.c_str());
    throw;
  }
}
