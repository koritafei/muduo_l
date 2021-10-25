#ifndef __POLLER_H__
#define __POLLER_H__

#include <map>
#include <vector>

#include "../base/TimeStamp.h"
#include "EventLoop.h"

namespace muduo {

namespace net {

class Channel;

class Poller : Noncopyable {
public:
  typedef std::vector<Channel *> ChannelList;

  Poller(EventLoop *loop);
  virtual ~Poller();

  virtual TimeStamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
  virtual void      updateChannel(Channel *channel)                  = 0;
  virtual void      removeChannel(Channel *channel)                  = 0;
  virtual bool      hasChannel(Channel *channel) const;

  static Poller *newDefaultPoller(EventLoop *loop);

  void assertInLoopThread() const {
    ownerLoop_->assertInLoopThread();
  }

protected:
  typedef std::map<int, Channel *> ChannelMap;
  ChannelMap                       channels_;

private:
  EventLoop *ownerLoop_;
};  // class Poller

}  // namespace net

}  // namespace muduo

#endif /* __POLLER_H__ */
