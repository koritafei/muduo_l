
#include <unistd.h>

#include <iostream>

#include "Thread.h"

class TestThread : public Thread {
public:
  TestThread(int count) : count_(count) {
    std::cout << "TestThread ..." << std::endl;
  }

  ~TestThread() {
    std::cout << "~TestThread ..." << std::endl;
  }

  void Run() override {
    while (count_-- > -2) {
      std::cout << "TestThread Run ..." << std::endl;
    }
  }

private:
  int count_;
};  // class TestThread

int main(int argc, char **argv) {
  TestThread t(5);
  t.Start();
  t.Join();

  std::cout << "Main Thread ..." << std::endl;
  t.Run();  // 主线程中运行

  return 0;
}
