#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>

class Thread {
public:
  Thread();
  virtual ~Thread();

  void Start();
  void Join();

private:
  static void *ThreadRoutine(void *arg);

  virtual void Run() = 0;
  pthread_t    threadId_;
};  // class Thread

#endif /* __THREAD_H__ */
