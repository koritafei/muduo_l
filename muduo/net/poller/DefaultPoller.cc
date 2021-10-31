#include <stdio.h>

#include "../Poller.h"
#include "EPollPoller.h"
#include "PollPoller.h"

using namespace muduo::net;

Poller* Poller::newDefaultPoller(EventLoop* loop) {
  if (::getenv("MUDUO_USE_POLL")) {
    return new PollPoller(loop);
  } else {
    return new EPollPoller(loop);
  }
}
