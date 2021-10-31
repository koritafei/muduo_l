#ifndef __POLLPOLLER__H__
#define __POLLPOLLER__H__

#include <vector>

#include "../Poller.h"

struct pollfd;

namespace muduo {

namespace net {

class PollPoller : public Poller {
public:
  PollPoller(EventLoop *loop);
  ~PollPoller() override;

  TimeStamp poll(int timeoutMs, ChannelList *activeChannels) override;
  void      updateChannel(Channel *channel) override;
  void      removeChannel(Channel *channel) override;

private:
  void fillActiveChannels(int operation, ChannelList *activeChannels) const;
  typedef std::vector<struct pollfd> PollFdList;
  PollFdList                         pollFds_;
};  // class PollPoller

}  // namespace net

}  // namespace muduo

#endif /* __POLLPOLLER__H__ */
