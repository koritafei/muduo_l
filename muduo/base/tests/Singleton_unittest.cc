#include <stdio.h>

#include "../CurrentThread.h"
#include "../Noncopyable.h"
#include "../Signleton.h"
#include "../Thread.h"

class Test : muduo::Noncopyable {
public:
  Test() {
    printf("tid = %d, constructing %p\n", muduo::CurrentThread::tid(), this);
  }

  ~Test() {
    printf("tid = %d, destructing %p, %s\n",
           muduo::CurrentThread::tid(),
           this,
           name_.c_str());
  }

  const muduo::string &name() const {
    return name_;
  }

  void setName(const muduo::string &n) {
    name_ = n;
  }

private:
  muduo::string name_;
};

class TestNoDestroy : muduo::Noncopyable {
public:
  void no_destory();

  TestNoDestroy() {
    printf("tid = %d, constructing TestNoDestory %p\n",
           muduo::CurrentThread::tid(),
           this);
  }

  ~TestNoDestroy() {
    printf("tid = %d, destructing TestNoDestory %p\n",
           muduo::CurrentThread::tid(),
           this);
  }
};

void threadFunc() {
  printf("tid = %d, %p name = %s\n",
         muduo::CurrentThread::tid(),
         &muduo::Singleton<Test>::instance(),
         muduo::Singleton<Test>::instance().name().c_str());
  muduo::Singleton<Test>::instance().setName("only one,charged");
}

int main() {
  muduo::Singleton<Test>::instance().setName("only one");
  muduo::Thread t1(threadFunc);
  t1.start();
  t1.join();

  printf("tid = %d, %p name = %s\n",
         muduo::CurrentThread::tid(),
         &muduo::Singleton<Test>::instance(),
         muduo::Singleton<Test>::instance().name().c_str());
  muduo::Singleton<TestNoDestroy>::instance();
  printf("with valgrind, you should see %zd-byte memory leak.\n",
         sizeof(TestNoDestroy));
}
