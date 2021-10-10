#ifndef __CURRENTTHREAD_H__
#define __CURRENTTHREAD_H__

#include <unistd.h>

#include "Types.h"

namespace muduo {

namespace CurrentThread {
// inter
extern __thread int         t_cachedTid;
extern __thread char        t_tidString[32];
extern __thread int         t_tidStringLength;
extern __thread const char *t_theadName;

void cacheTid();

inline int tid() {
  if (__builtin_expect(t_cachedTid == 0, 0)) {
    cacheTid();
  }

  return t_cachedTid;
}

inline const char *tidString() {
  return t_tidString;
}

inline int tidStringLength() {
  return t_tidStringLength;
}

inline const char *name() {
  return t_theadName;
}

bool isMainThread();

void sleepUsec(int64_t usec);

std::string stackTrace(bool demangle);

}  // namespace CurrentThread

}  // namespace muduo

#endif /* __CURRENTTHREAD_H__ */
