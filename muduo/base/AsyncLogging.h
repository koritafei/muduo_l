#ifndef __ASYNCLOGGING__H__
#define __ASYNCLOGGING__H__

#include <atomic>
#include <vector>

#include "BlockingQueue.h"
#include "BoundedBlockingQueue.h"
#include "CountDownLatch.h"
#include "LogStream.h"
#include "Mutex.h"
#include "Thread.h"

namespace muduo {

class AsyncLogging : Noncopyable {
public:
  AsyncLogging(const string& basename, off_t rollSize, int flushInterval = 3);

  ~AsyncLogging() {
    if (running_) {
      stop();
    }
  }

  void append(const char* logline, int len);

  void start() {
    running_ = true;
    thread_.start();
    latch_.wait();
  }

  void stop() NO_THREAD_SAFETY_ANALYSIS {
    running_ = false;
    cond_.notify();
    thread_.join();
  }

private:
  void threadFunc();

  typedef muduo::detail::FixedBuffer<muduo::detail::kLargeBuffer> Buffer;
  typedef std::vector<std::unique_ptr<Buffer>>                    BufferVector;
  typedef BufferVector::value_type                                BufferPtr;

  const int                flushInterval_;
  std::atomic<bool>        running_;
  const string             basename_;
  const off_t              rollSize_;
  muduo::Thread            thread_;
  muduo::CountDownLatch    latch_;
  muduo::MutexLock         mutex_;
  muduo::Condition cond_   GUARDED_BY(mutex_);
  BufferPtr currentBuffer_ GUARDED_BY(mutex_);
  BufferPtr nextBuffer_    GUARDED_BY(mutex_);
  BufferVector buffers_    GUARDED_BY(mutex_);
};

}  // namespace muduo

#endif /* __ASYNCLOGGING__H__ */
