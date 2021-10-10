#ifndef __ATOMIC_H__
#define __ATOMIC_H__

#include <stdint.h>

#include "Noncopyable.h"

namespace muduo {

namespace detail {

template <typename T>
class AtomicIntegerT : Noncopyable {
public:
  AtomicIntegerT() : value_(0) {
  }

  T get() {
    return __sync_val_compare_and_swap(&value_, 0, 0);
  }

  T getAndAdd(T x) {
    return __sync_fetch_and_add(&value_, x);
  }

  T addAndGet(int x) {
    return getAndAdd(x) + x;
  }

  T incrementAndGet() {
    return addAndGet(1);
  }

  T decrementAndGet() {
    return addAndGet(-1);
  }

  void add(int x) {
    addAndGet(x);
  }

  void increment() {
    incrementAndGet();
  }

  void decrement() {
    decrementAndGet();
  }

  T getAndSet(T newvalue) {
    return __sync_lock_test_and_set(&value_, newvalue);
  }

private:
  volatile T value_;
};

}  // namespace detail

typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
typedef detail::AtomicIntegerT<int64_t> AtomicInt64;

}  // namespace muduo

#endif /* __ATOMIC_H__ */
