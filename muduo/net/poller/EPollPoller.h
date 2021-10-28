#ifndef __EPOLLPOLLER__H__
#define __EPOLLPOLLER__H__

#include <vector>

#include "../Poller.h"

struct epoll_event;

namespace muduo {

namespace net {

class EPollPoller : public Poller {
public:
  EPollPoller(EventLoop* loop);
  ~EPollPoller() override;

  TimeStamp poll(int timeoutMs, ChannelList* activeChannel) override;
  void      updateChannel(Channel* channel) override;
  void      removeChannel(Channel* channel) override;

private:
  static const int   kInitEventListSize = 16;
  static const char* operationToString(int op);
  void fillActiveChannel(int numThreads, ChannelList* activeChannels) const;
  void update(int operation, Channel* channel);

  typedef std::vector<struct epoll_event> EventList;
  int                                     epollfd_;
  EventList                               events_;
};  // class EPollPoller

}  // namespace net

}  // namespace muduo

#endif /* __EPOLLPOLLER__H__ */
