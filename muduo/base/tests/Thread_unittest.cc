#include "../Thread.h"

#include <bits/types/struct_timespec.h>
#include <stdio.h>
#include <unistd.h>

#include <ctime>
#include <string>

#include "../CurrentThread.h"

void mysleep(int seconds) {
  timespec t = {seconds, 0};
  nanosleep(&t, nullptr);
}

void ThreadFunc() {
  printf("tid = %d\n", muduo::CurrentThread::tid());
}

void ThreadFunc2(int x) {
  printf("tid = %d, x = %d\n", muduo::CurrentThread::tid(), x);
}

void ThreadFunc3() {
  printf("tid = %d\n", muduo::CurrentThread::tid());
  mysleep(1);
}

class Foo {
public:
  explicit Foo(double x) : x_(x) {
  }

  void memfunc() {
    printf("tid = %d, Foo::x_ = %f\n", muduo::CurrentThread::tid(), x_);
  }

  void memfunc2(const std::string &text) {
    printf("tid = %d, Foo::x_ = %f, text = %s\n",
           muduo::CurrentThread::tid(),
           x_,
           text.c_str());
  }

private:
  double x_;
};

int main() {
  printf("pid = %d, tid = %d\n", ::getpid(), muduo::CurrentThread::tid());

  muduo::Thread t1(ThreadFunc);
  t1.start();
  printf("t1.tid = %d\n", t1.tid());
  t1.join();

  muduo::Thread t2(std::bind(ThreadFunc2, 42),
                   "thread for free function with argument");
  t2.start();
  printf("t2.tid = %d\n", t2.tid());
  t2.join();

  Foo           foo(87.53);
  muduo::Thread t3(std::bind(&Foo::memfunc, &foo),
                   "thread for member function without argument");
  t3.start();
  t3.join();

  muduo::Thread t4(
      std::bind(&Foo::memfunc2, std::ref(foo), std::string("qwer")));
  t4.start();
  t4.join();

  {
    muduo::Thread t5(ThreadFunc3);
    t5.start();
  }
  mysleep(2);
  {
    muduo::Thread t6(ThreadFunc3);
    t6.start();
    mysleep(2);
    // t6 destruct later than thread creation.
  }
  sleep(2);
  printf("number of created threads %d\n", muduo::Thread::numCreated());
}
