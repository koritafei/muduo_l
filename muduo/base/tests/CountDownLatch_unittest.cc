#include "../CountDownLatch.h"

#include <stdio.h>

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <string>

#include "../CurrentThread.h"
#include "../Thread.h"
#include "../TimeStamp.h"

using namespace muduo;

class Test {
public:
  Test(int numThreads) : latch_(1), threads_(numThreads) {
    for (int i = 0; i < numThreads; ++i) {
      char name[32];
      snprintf(name, sizeof name, "work thread %d", i);
      threads_.push_back(new muduo::Thread(boost::bind(&Test::threadFunc, this),
                                           muduo::string(name)));
    }
    std::for_each(threads_.begin(),
                  threads_.end(),
                  boost::bind(&Thread::start, _1));
  }

  void run() {
    latch_.countDown();
  }

  void joinAll() {
    std::for_each(threads_.begin(),
                  threads_.end(),
                  boost::bind(&Thread::join, _1));
  }

private:
  CountDownLatch                   latch_;
  boost::ptr_vector<muduo::Thread> threads_;

  void threadFunc() {
    latch_.wait();
    printf("tid = %d, %s now = %s started\n",
           CurrentThread::tid(),
           CurrentThread::name(),
           TimeStamp::now().toString().c_str());
    printf("tid = %d, %s now = %s stopped\n",
           CurrentThread::tid(),
           CurrentThread::name(),
           TimeStamp::now().toString().c_str());
  }
};

int main() {
  printf("pid = %d, tid = %d\n", ::getpid(), CurrentThread::tid());
  Test t(3);
  sleep(3);

  printf("pid = %d, tid = %d, %s running...\n",
         ::getpid(),
         CurrentThread::tid(),
         CurrentThread::name());
  t.run();
  t.joinAll();

  printf("number of created threads %d\n", Thread::numCreated());
  return 0;
}
