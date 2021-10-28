#ifndef __ECHO__H__
#define __ECHO__H__

#include "../../muduo/net/TcpServer.h"

class EchoServer {
public:
  EchoServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &addr);

  void start();

private:
  void onConnection(const muduo::net::TcpConnectionPtr &conn);

  void onMessgae(const muduo::net::TcpConnectionPtr &conn,
                 muduo::net::Buffer *                buff,
                 muduo::TimeStamp                    time);

  muduo::net::TcpServer server_;
};  // class EchoServer

#endif /* __ECHO__H__ */
