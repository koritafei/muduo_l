#ifndef __THREADLOCALSINGLE_H__
#define __THREADLOCALSINGLE_H__

#include <assert.h>
#include <pthread.h>

#include "Noncopyable.h"

namespace muduo {

template <typename T>
class ThreadLocalSingle : Noncopyable {
public:
  ThreadLocalSingle()  = delete;
  ~ThreadLocalSingle() = delete;

  static T &instance() {
    if (!t_value_) {
      t_value_ = new T();
      deleter_.set(t_value_);
    }

    return *t_value_;
  }

  static T *pointer() {
    return t_value_;
  }

private:
  class Deleter {
  public:
    Deleter() {
      pthread_key_create(&pkey_, &ThreadLocalSingle::destructor);
    }

    ~Deleter() {
      pthread_key_delete(pkey_);
    }

    void set(T *newObj) {
      assert(pthread_getspecific(pkey_) == nullptr);
      pthread_setspecific(pkey_, newObj);
    }

    pthread_key_t pkey_;
  };  // class Deleter

  static void destructor(void *obj) {
    assert(t_value_ == obj);
    typedef char            T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
    T_must_be_complete_type dummy;
    (void)dummy;

    delete t_value_;
    obj = nullptr;
  }

  static __thread T *t_value_;
  static Deleter     deleter_;
};  // class ThreadLocalSingle

template <typename T>
__thread T *ThreadLocalSingle<T>::t_value_ = 0;

template <typename T>
typename ThreadLocalSingle<T>::Deleter ThreadLocalSingle<T>::deleter_;

}  // namespace muduo

#endif /* __THREADLOCALSINGLE_H__ */
