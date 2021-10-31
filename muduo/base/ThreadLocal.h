#ifndef __THREADLOCAL_H__
#define __THREADLOCAL_H__

#include <pthread.h>

#include "Mutex.h"
#include "Noncopyable.h"

namespace muduo {

template <class T>
class ThreadLocal : Noncopyable {
public:
  ThreadLocal() {
    MCHECK(pthread_key_create(&pkey_, &ThreadLocal::destructor));
  }

  ~ThreadLocal() {
    MCHECK(pthread_key_delete(pkey_));
  }

  T &value() {
    T *preThreadValue = static_cast<T *>(pthread_getspecific(pkey_));

    if (!preThreadValue) {
      T *obj = new T();
      pthread_setspecific(pkey_, obj);
      preThreadValue = obj;
    }

    return *preThreadValue;
  }

private:
  pthread_key_t pkey_;

  static void destructor(void *x) {
    T *                     obj = static_cast<T *>(x);
    typedef char            T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
    T_must_be_complete_type dummy;
    (void)dummy;

    delete obj;
  }
};

}  // namespace muduo

#endif /* __THREADLOCAL_H__ */
