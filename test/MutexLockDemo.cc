#include <unistd.h>

#include <iostream>

#include "../muduo/base/Mutex.hpp"

int              j = 0;
muduo::MutexLock mutexlock_;

void *f1(void *arg) {
  muduo::MutexLockGuard lockgurad(mutexlock_);

  for (int i = 0; i < 100; i++) {
    sleep(1);
    j += i;
    std::cout << "f1 :" << j << " : " << i << std::endl;
  }

  return nullptr;
}

void *f2(void *arg) {
  muduo::MutexLockGuard lockgurad(mutexlock_);

  for (int i = 0; i < 100; i++) {
    sleep(1);
    j += i;
    std::cout << "f2 :" << j << " : " << i << std::endl;
  }

  return nullptr;
}

int main(int argc, char *argv[]) {
  pthread_t pid1, pid2;
  pthread_create(&pid1, NULL, f1, NULL);
  pthread_create(&pid2, NULL, f2, NULL);

  pthread_join(pid1, NULL);
  pthread_join(pid2, NULL);
}
