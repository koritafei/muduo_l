#include "../EventLoopThread.h"

#include <stdio.h>
#include <unistd.h>

#include "../../base/CountDownLatch.h"
#include "../../base/Thread.h"
#include "../EventLoop.h"

using namespace muduo;
using namespace muduo::net;

void print(EventLoop *p = nullptr) {
  printf("print: pid = %d, tid = %d, loop = %p\n",
         getpid(),
         CurrentThread::tid(),
         p);
}

void quit(EventLoop *p) {
  print(p);
  p->quit();
}

int main() {
  print();

  { EventLoopThread thr1; }

  {
    EventLoopThread thr2;
    EventLoop *     loop = thr2.startLoop();
    loop->runInLoop(std::bind(print, loop));
    CurrentThread::sleepUsec(500 * 1000);
  }
  {
    EventLoopThread thr3;
    EventLoop *     loop = thr3.startLoop();
    loop->runInLoop(std::bind(quit, loop));
    CurrentThread::sleepUsec(500 * 1000);
  }
}
