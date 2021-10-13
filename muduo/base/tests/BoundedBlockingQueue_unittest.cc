#include "../BoundedBlockingQueue.h"

#include <stdio.h>
#include <unistd.h>

#include <memory>
#include <string>
#include <vector>

#include "../CountDownLatch.h"
#include "../Thread.h"

class Test {
public:
  Test(int numThreads) : queue_(20), latch_(numThreads) {
    for (int i = 0; i < numThreads; i++) {
      char name[32];
      snprintf(name, sizeof name, "work thread %d", i);
      threads_.emplace_back(
          new muduo::Thread(std::bind(&Test::threadFunc, this),
                            muduo::string(name)));
    }

    for (auto &thr : threads_) {
      thr->start();
    }
  }

  void run(int times) {
    printf("wait for count down latch\n");
    latch_.wait();
    printf("all threads started");

    for (int i = 0; i < times; i++) {
      char buf[32];
      snprintf(buf, sizeof buf, "hello %d", i);
      queue_.put(buf);
      printf("tid = %d, put buf = %s, size = %zd\n",
             muduo::CurrentThread::tid(),
             buf,
             queue_.size());
    }
  }

  void joinAll() {
    for (size_t i = 0; i < threads_.size(); ++i) {
      queue_.put("stopped");
    }

    for (auto &thr : threads_) {
      thr->join();
    }
  }

private:
  muduo::BoundedBlockingQueue<std::string>    queue_;
  muduo::CountDownLatch                       latch_;
  std::vector<std::unique_ptr<muduo::Thread>> threads_;

  void threadFunc() {
    printf("tid = %d, %s started \n",
           muduo::CurrentThread::tid(),
           muduo::CurrentThread::name());
    latch_.countDown();
    bool running = true;
    while (running) {
      std::string data(queue_.take());
      printf("tid = %d, get data = %s, size = %zd\n",
             muduo::CurrentThread::tid(),
             data.c_str(),
             queue_.size());

      running = (data != "stopped");
    }

    printf("tid = %d, %s stoped\n",
           muduo::CurrentThread::tid(),
           muduo::CurrentThread::name());
  }
};

void testMove() {
  muduo::BoundedBlockingQueue<std::unique_ptr<int>> queue(10);
  queue.put(std::unique_ptr<int>(new int(42)));
  std::unique_ptr<int> x = queue.take();
  printf("took %d\n", *x);
  *x = 123;
  queue.put(std::move(x));
  std::unique_ptr<int> y = queue.take();
  printf("took %d\n", *y);
}

int main() {
  printf("pid=%d, tid = %d, %s",
         ::getpid(),
         muduo::CurrentThread::tid(),
         muduo::CurrentThread::name());
  Test t(5);
  t.run(100);
  t.joinAll();

  testMove();

  return 0;
}
