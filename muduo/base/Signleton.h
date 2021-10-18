#ifndef __SIGNLETON_H__
#define __SIGNLETON_H__

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "Noncopyable.h"

namespace muduo {

namespace detail {

template <typename T>
struct has_no_destory {
  template <typename C>
  static char test(decltype(&C::no_destory));

  template <typename C>
  static int32_t test(...);

  const static bool value = sizeof(test<T>(0)) == 1;
};  // struct has_no_destory

}  // namespace detail

template <typename T>
class Singleton : Noncopyable {
public:
  Singleton()  = delete;
  ~Singleton() = delete;

  static T& instance() {
    pthread_once(&ponce_, &Singleton::init);
    assert(value_ != nullptr);

    return *value_;
  }

private:
  static pthread_once_t ponce_;
  static T*             value_;

  static void init() {
    value_ = new T();
    if (detail::has_no_destory<T>::value) {
      ::atexit(destroy);
    }
  }

  static void destroy() {
    typedef char            T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
    T_must_be_complete_type dummy;
    (void)dummy;
    delete value_;
    value_ = nullptr;
  }

};  // class Singleton

template <typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

template <typename T>
T* Singleton<T>::value_ = nullptr;
}  // namespace muduo

#endif /* __SIGNLETON_H__ */
