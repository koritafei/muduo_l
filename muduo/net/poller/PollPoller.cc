#include "PollPoller.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>

#include "../../base/Logging.h"
#include "../../base/Types.h"
#include "../Channel.h"

using namespace muduo;
using namespace muduo::net;

PollPoller::PollPoller(EventLoop *loop) : Poller(loop) {
}

PollPoller::~PollPoller() = default;

TimeStamp PollPoller::poll(int timeoutMs, ChannelList *activeChannels) {
  int       numEvents = ::poll(&*pollFds_.begin(), pollFds_.size(), timeoutMs);
  int       saveErrno = errno;
  TimeStamp now(TimeStamp::now());

  if (0 < numEvents) {
    LOG_TRACE << numEvents << " events happened";
    fillActiveChannels(numEvents, activeChannels);
  } else if (0 == numEvents) {
    LOG_TRACE << " nothing happened";
  } else {
    if (EINTR != saveErrno) {
      errno = saveErrno;
      LOG_SYSERR << "PollPoller::poll()";
    }
  }

  return now;
}

void PollPoller::fillActiveChannels(int          numEvents,
                                    ChannelList *activeChannels) const {
  for (PollFdList::const_iterator pfd = pollFds_.begin();
       pfd != pollFds_.end() && numEvents > 0;
       ++pfd) {
    if (0 < pfd->revents) {
      --numEvents;
      ChannelMap::const_iterator ch = channels_.find(pfd->fd);
      assert(ch != channels_.end());
      Channel *channel = ch->second;
      assert(channel->fd() == pfd->fd);
      channel->set_revents(pfd->revents);

      activeChannels->push_back(channel);
    }
  }
}

void PollPoller::updateChannel(Channel *channel) {
  Poller::assertInLoopThread();
  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();

  if (0 > channel->index()) {
    assert(channels_.find(channel->fd()) == channels_.end());
    struct pollfd pfd;

    pfd.fd      = channel->fd();
    pfd.events  = static_cast<short>(channel->events());
    pfd.revents = 0;

    pollFds_.push_back(pfd);
    int idx = static_cast<int>(pollFds_.size()) - 1;
    channel->set_index(idx);
    channels_[pfd.fd] = channel;
  } else {
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    int idx = channel->index();

    assert(0 <= idx && idx < static_cast<int>(pollFds_.size()));
    struct pollfd &pfd = pollFds_[idx];
    assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
    pfd.fd     = channel->fd();
    pfd.events = static_cast<short>(channel->events());

    pfd.revents = 0;

    if (channel->isNoneEvent()) {
      pfd.fd = -channel->fd() - 1;
    }
  }
}

void PollPoller::removeChannel(Channel *channel) {
  PollPoller::assertInLoopThread();
  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
  assert(channels_.find(channel->fd()) != channels_.end());
  assert(channels_[channel->fd()] == channel);
  assert(channel->isNoneEvent());

  int idx = channel->index();
  assert(0 <= idx && idx < static_cast<int>(pollFds_.size()));
  const struct pollfd &pfd = pollFds_[idx];
  (void)pfd;
  assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
  size_t n = channels_.erase(channel->fd());
  assert(1 == n);
  (void)n;

  if (pollFds_.size() - 1 == implicit_cast<size_t>(idx)) {
    pollFds_.pop_back();
  } else {
    int channelAtEnd = pollFds_.back().fd;
    iter_swap(pollFds_.begin() + idx, pollFds_.end() - 1);

    if (0 > channelAtEnd) {
      channelAtEnd = -channelAtEnd - 1;
    }

    channels_[channelAtEnd]->set_index(idx);
    pollFds_.pop_back();
  }
}
