#include <unistd.h>

#include "../../muduo/base/Logging.h"
#include "../../muduo/net/EventLoop.h"
#include "echo.h"

int main() {
  LOG_INFO << "pid = " << getpid();
  muduo::net::EventLoop   loop;
  muduo::net::InetAddress listenAddr(2007);
  EchoServer              server(&loop, listenAddr);
  server.start();
  loop.loop();
}
