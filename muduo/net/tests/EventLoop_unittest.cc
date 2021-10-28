#include "../EventLoop.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "../../base/Thread.h"

using namespace muduo;
using namespace muduo::net;

EventLoop *g_loop;

void callback() {
  printf("callback() : pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
  EventLoop anotherLoop;
}

void threadFunc() {
  printf("threadFunc() : pid = %d, tid = %d\n", getpid(), CurrentThread::tid());

  assert(EventLoop::getEventLoopOfCurrentLoop() == nullptr);
  EventLoop loop;
  assert(EventLoop::getEventLoopOfCurrentLoop() == &loop);

  loop.runAfter(1.0, callback);
  loop.loop();
}

int main() {
  printf("main() : pid = %d, tid = %d \n", getpid(), CurrentThread::tid());

  assert(EventLoop::getEventLoopOfCurrentLoop() == nullptr);
  EventLoop loop;
  assert(EventLoop::getEventLoopOfCurrentLoop() == &loop);

  Thread thread(threadFunc);
  thread.start();

  loop.loop();
}
