#include "EPollPoller.h"

#include <assert.h>
#include <errno.h>
#include <muduo/net/Channel.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "../../base/Logging.h"
#include "../Channel.h"

using namespace muduo;
using namespace muduo::net;

// are expected to be the same.
static_assert(EPOLLIN == POLLIN, "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI, "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT, "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR, "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP, "epoll uses same flag values as poll");

namespace {
const int kNew     = -1;
const int kAdded   = 1;
const int kDeleted = 2;
}  // namespace

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop),
      epollfd_(::epoll_create(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
  if (0 > epollfd_) {
    LOG_SYSFATAL << "EPollPoller::EPollPoller";
  }
}

EPollPoller::~EPollPoller() {
  ::close(epollfd_);
}

TimeStamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannel) {
  LOG_TRACE << "fd total count " << channels_.size();
  int       numEvents  = ::epoll_wait(epollfd_,
                               &*events_.begin(),
                               static_cast<int>(events_.size()),
                               timeoutMs);
  int       savedErrno = errno;
  TimeStamp now(TimeStamp::now());

  if (numEvents > 0) {
    LOG_TRACE << numEvents << " events happened";
    fillActiveChannel(numEvents, activeChannel);
    if (implicit_cast<size_t>(numEvents) == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  } else if (0 == numEvents) {
    LOG_TRACE << "nothing happened";
  } else {
    if (EINTR != savedErrno) {
      errno = savedErrno;
      LOG_SYSERR << "EPollPoller::poll()";
    }
  }

  return now;
}

void EPollPoller::fillActiveChannel(int          numEvents,
                                    ChannelList *activeChannels) const {
  assert(implicit_cast<size_t>(numEvents) <= events_.size());
  for (int i = 0; i < numEvents; i++) {
    Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
#ifndef NDEBUG
    int                        fd = channel->fd();
    ChannelMap::const_iterator it = channels_.find(fd);
    assert(it != channels_.end());
    assert(it->second == channel);
#endif
    channel->set_revents(events_[i].events);
    activeChannels->push_back(channel);
  }
}

void EPollPoller::updateChannel(Channel *channel) {
  Poller::assertInLoopThread();
  const int index = channel->index();
  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events()
            << " index = " << index;
  if (kNew == index || kDeleted == index) {
    int fd = channel->fd();
    if (kNew == index) {
      assert(channels_.find(fd) == channels_.end());
      channels_[fd] = channel;

    } else {
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
    }

    channel->set_index(kAdded);
    update(EPOLL_CTL_ADD, channel);
  } else {
    int fd = channel->fd();
    (void)fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(kAdded == index);

    if (channel->isNoneEvent()) {
      update(EPOLL_CTL_DEL, channel);
      channel->set_index(kDeleted);
    } else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EPollPoller::removeChannel(Channel *channel) {
  Poller::assertInLoopThread();
  int fd = channel->fd();
  LOG_TRACE << "fd = " << fd;

  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->isNoneEvent());
  int index = channel->index();
  assert(kAdded == index || kDeleted == index);
  size_t n = channels_.erase(fd);
  (void)n;
  assert(1 == n);

  if (kAdded == index) {
    update(EPOLL_CTL_DEL, channel);
  }

  channel->set_index(kNew);
}

void EPollPoller::update(int operation, Channel *channel) {
  struct epoll_event event;
  memZero(&event, sizeof event);
  event.events   = channel->events();
  event.data.ptr = channel;
  int fd         = channel->fd();

  LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
            << " fd = " << fd << " event = { " << channel->eventsToString()
            << " }";
  if (0 > ::epoll_ctl(epollfd_, operation, fd, &event)) {
    if (EPOLL_CTL_DEL == operation) {
      LOG_SYSERR << "epoll_ctl op =" << operationToString(operation)
                 << " fd =" << fd;
    } else {
      LOG_SYSFATAL << "epoll_ctl op =" << operationToString(operation)
                   << " fd =" << fd;
    }
  }
}

const char *EPollPoller::operationToString(int op) {
  switch (op) {
    case EPOLL_CTL_ADD:
      return "ADD";
    case EPOLL_CTL_DEL:
      return "DEL";
    case EPOLL_CTL_MOD:
      return "MOD";
    default:
      assert(false && "ERROR op");
      return "Unknown Operation";
  }
}
