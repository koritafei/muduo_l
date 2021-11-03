#include "../../base/Logging.h"
#include "../EventLoop.h"
#include "../TcpClient.h"

using namespace muduo;
using namespace muduo::net;

TcpClient *g_client;

void timeout() {
  LOG_INFO << "timeout";
  g_client->stop();
}

int main(int argc, char **argv) {
  EventLoop   loop;
  InetAddress serverAddr("127.0.0.1", 2);
  TcpClient   tcpClient(&loop, serverAddr, "TcpClient");
  g_client = &tcpClient;
  loop.runAfter(0.0, timeout);
  loop.runAfter(1.0, std::bind(&EventLoop::quit, &loop));
  tcpClient.connect();
  CurrentThread::sleepUsec(100 * 1000);
  loop.loop();
}
