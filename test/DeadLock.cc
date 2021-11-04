#include <muduo/base/Mutex.h>
#include <stdio.h>

class Request {
public:
  void process() {
    printf("process...\n");
    muduo::MutexLockGuard lock(mutex_);
    print();
  }

  void print() {
    printf("print...\n");
    muduo::MutexLockGuard lock(mutex_);
  }

private:
  mutable muduo::MutexLock mutex_;
};

int main(int argc, char **argv) {
  Request req;
  req.process();
}
