#include "../ThreadLocalSingle.h"

#include <stdio.h>

#include "../CurrentThread.h"
#include "../Noncopyable.h"
#include "../Thread.h"

class Test : muduo::Noncopyable {
public:
  Test() {
    printf("tid = %d, constructing %p\n", muduo::CurrentThread::tid(), this);
  }

  ~Test() {
    printf("tid = %d, destructing %p %s\n",
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

void threadFunc(const char *changeTo) {
  printf("tid = %d, %p name = %s\n",
         muduo::CurrentThread::tid(),
         &muduo::ThreadLocalSingle<Test>::instance(),
         muduo::ThreadLocalSingle<Test>::instance().name().c_str());
  muduo::ThreadLocalSingle<Test>::instance().setName(changeTo);
  printf("tid = %d, %p name = %s\n",
         muduo::CurrentThread::tid(),
         &muduo::ThreadLocalSingle<Test>::instance(),
         muduo::ThreadLocalSingle<Test>::instance().name().c_str());
}

int main() {
  muduo::ThreadLocalSingle<Test>::instance().setName("main one");
  muduo::Thread t1(std::bind(threadFunc, "thread1"));
  muduo::Thread t2(std::bind(threadFunc, "thread2"));
  t1.start();
  t2.start();
  t1.join();

  printf("tid = %d, %p name = %s\n",
         muduo::CurrentThread::tid(),
         &muduo::ThreadLocalSingle<Test>::instance(),
         muduo::ThreadLocalSingle<Test>::instance().name().c_str());
  t2.join();
  pthread_exit(0);
}
