#include "../Mutex.h"

#include <stdio.h>

#include <vector>

#include "../CountDownLatch.h"
#include "../Thread.h"
#include "../TimeStamp.h"

using namespace muduo;
using namespace std;

MutexLock   g_mutex;
vector<int> g_vec;
const int   kCount = 10 * 1000 * 1000;

void threadFunc() {
  for (int i = 0; i < kCount; i++) {
    MutexLockGuard lock(g_mutex);
    g_vec.push_back(i);
  }
}

int foo() __attribute__((noinline));

int g_count = 0;
int foo() {
  MutexLockGuard lock(g_mutex);
  if (!g_mutex.isLockedByThisThread()) {
    printf("FATAL\n");
    return -1;
  }
  ++g_count;
  return 0;
}

int main() {
  printf("sizeof pthread_mutex_t :%zd\n", sizeof(pthread_mutex_t));
  printf("sizeof Mutex :%zd\n", sizeof(MutexLock));
  printf("sizeof pthread_cond_t :%zd\n", sizeof(pthread_cond_t));
  printf("sizeof Condition : %zd\n", sizeof(Condition));

  MCHECK(foo());

  if (1 != g_count) {
    printf("MCHECK call twice!\n");
    abort();
  }

  const int kMaxThreads = 8;
  g_vec.reserve(kMaxThreads * kCount);

  TimeStamp start(TimeStamp::now());
  for (int i = 0; i < g_count; i++) {
    g_vec.push_back(i);
  }

  TimeStamp end(TimeStamp::now());
  printf("single thread without lock %f\n", timeDifference(end, start));

  start = TimeStamp::now();
  threadFunc();
  end = TimeStamp::now();
  printf("thread with lock %f\n", timeDifference(end, start));

  for (int nthreads = 1; nthreads < kMaxThreads; nthreads++) {
    std::vector<std::unique_ptr<Thread>> threads;
    g_vec.clear();
    start = TimeStamp::now();
    for (int i = 0; i < nthreads; ++i) {
      threads.emplace_back(new Thread(&threadFunc));
      threads.back()->start();
    }

    for (int i = 0; i < nthreads; ++i) {
      threads[i]->join();
    }

    end = TimeStamp::now();
    printf("%d thread(s) with lock %f\n", nthreads, timeDifference(end, start));
  }
}
