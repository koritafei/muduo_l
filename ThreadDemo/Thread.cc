
#include "Thread.h"

#include <iostream>

Thread::Thread() {
  std::cout << "Thread ..." << std::endl;
}

Thread::~Thread() {
  std::cout << "~Thread ..." << std::endl;
}

void Thread::Join() {
  pthread_join(threadId_, nullptr);
}

void Thread::Start() {
  pthread_create(&threadId_, NULL, ThreadRoutine, this);
}

void *Thread::ThreadRoutine(void *arg) {
  Thread *thr = static_cast<Thread *>(arg);
  thr->Run();

  return nullptr;
}
