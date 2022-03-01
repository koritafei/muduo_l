#ifndef __ATOMIC__H__
#define __ATOMIC__H__

#include <stdint.h>

#include <boost/core/noncopyable.hpp>
#include <boost/noncopyable.hpp>

namespace muduo {

namespace detail {

template <typename T>
class AtomicIntegerT : public boost::noncopyable {
public:
  AtomicIntegerT() : value_(0) {
  }

  T get() const {
    return __sync_val_compare_and_swap(
        const_cast<volatile T *>(&value_),
        0,
        0);  // __sync_val_compare_and_swap
             // 将value_的旧值比较，如果与旧值相等，则将新值更新到value_上，并返回value_的旧值
  }

  T getAndAdd(T x) {
    return __sync_fetch_and_add(
        &value_,
        x);  // __sync_fetch_and_add
             // 原子add操作，将x添加到value_上，并将value_原值返回
  }

  T addAndGet(T x) {
    return getAndAdd(x) + x;
  }

  T incrementAndGet() {
    return addAndGet(1);
  }

  void add(T x) {
    getAndAdd(x);
  }

  void increment() {
    incrementAndGet();
  }

  void drecement() {
    getAndAdd(-1);
  }

  T getAndSet(T newvalue) {
    __sync_lock_test_and_set(
        &value_,
        newvalue);  // 将value写入*ptr，对*ptr加锁，并返回操作之前*ptr的值。即，try
                    // spinlock语义
  }

private:
  volatile T value_;
};

}  // namespace detail

typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
typedef detail::AtomicIntegerT<int64_t> AtomicInt64;

}  // namespace muduo

#endif /* __ATOMIC__H__ */
