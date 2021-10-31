#include "../EventLoopThreadPool.h"

#include <stdio.h>
#include <unistd.h>

#include "../../base/Thread.h"
#include "../EventLoop.h"

using namespace muduo;
using namespace muduo::net;

void print(EventLoop *loop = nullptr) {
  printf("main() : pid = %d, tid = %d, loop = %p\n",
         getpid(),
         CurrentThread::tid(),
         loop);
}

void init(EventLoop *loop) {
  printf("init()： pid = %d, tid = %d, loop = %p\n",
         getpid(),
         CurrentThread::tid(),
         loop);
}

int main() {
  print();

  EventLoop loop;
  loop.runAfter(11, std::bind(&EventLoop::quit, &loop));

  {
    printf("Single thread %p\n", &loop);
    EventLoopThreadPool model(&loop, "single");
    model.setNumthreads(0);
    model.start();
    assert(model.getNextLoop() == &loop);
    assert(model.getNextLoop() == &loop);
    assert(model.getNextLoop() == &loop);
  }

  {
    printf("Another thread\n");
    EventLoopThreadPool model(&loop, "another");
    model.setNumthreads(1);
    model.start(init);
    EventLoop *nextLoop = model.getNextLoop();
    nextLoop->runAfter(2, std::bind(print, nextLoop));
    assert(nextLoop != &loop);
    assert(nextLoop == model.getNextLoop());
    assert(nextLoop == model.getNextLoop());
  }

  {
    printf("Three Threads\n");
    EventLoopThreadPool model(&loop, "three");
    model.setNumthreads(3);
    model.start(init);
    EventLoop *nextLoop = model.getNextLoop();
    nextLoop->runInLoop(std::bind(print, nextLoop));
    assert(nextLoop != &loop);
    assert(nextLoop != model.getNextLoop());
    assert(nextLoop != model.getNextLoop());
    assert(nextLoop == model.getNextLoop());
  }

  loop.loop();
}
