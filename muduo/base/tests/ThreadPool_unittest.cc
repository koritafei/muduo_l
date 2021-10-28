#include "../ThreadPool.h"

#include <stdio.h>
#include <unistd.h>

#include "../CountDownLatch.h"
#include "../CurrentThread.h"
#include "../Logging.h"

void print() {
  printf("tid = %d\n", muduo::CurrentThread::tid());
}

void printString(const std::string &str) {
  LOG_INFO << str;
  usleep(1000 * 1000);
}

void test(int maxSize) {
  LOG_WARN << "Test ThreadPool with maxsize size = " << maxSize << "\n";
  muduo::ThreadPool pool("MainThreadPool");
  pool.setMaxQueueSize(maxSize);
  pool.start(5);

  LOG_WARN << "Adding\n";
  pool.run(print);
  pool.run(print);

  for (int i = 0; i < 100; i++) {
    char buf[32];
    snprintf(buf, sizeof buf, "task %d\n", i);
    pool.run(std::bind(printString, std::string(buf)));
  }

  LOG_WARN << "Done.\n";

  muduo::CountDownLatch latch_(1);
  pool.run(std::bind(&muduo::CountDownLatch::countDown, &latch_));
  latch_.wait();
  pool.stop();
}

void longTask(int num) {
  LOG_INFO << "long task " << num << "\n";
  muduo::CurrentThread::sleepUsec(3000000);
}

void test2() {
  LOG_WARN << "Test ThreadPool by stoping early.\n";
  muduo::ThreadPool pool("ThreadPool");
  pool.setMaxQueueSize(5);
  pool.start(3);

  muduo::Thread thread1(
      [&pool]() {
        for (int i = 0; i < 20; ++i) {
          pool.run(std::bind(longTask, i));
        }
      },
      "thread1");
  thread1.start();

  muduo::CurrentThread::sleepUsec(5000000);
  LOG_WARN << "stop pool\n";
  pool.stop();  // early stop

  thread1.join();
  // run() after stop()
  pool.run(print);
  LOG_WARN << "test2 Done\n";
}

int main() {
  test(0);
  test(1);
  test(5);
  test(10);
  test(50);
  test2();
}
