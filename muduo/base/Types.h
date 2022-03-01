#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <string.h>

#include <string>

#ifndef NDEBUG
#include <assert.h>
#endif

namespace muduo {

using std::string;

inline void memoZero(void *p, size_t n) {
  memset(p, 0, n);
}

template <typename To, typename From>
inline To implicit_cast(const From &f) {
  return f;
}

template <typename To, typename From>
inline To down_cast(From *f) {
  if (false) {
    return implicit_cast<From *, To>(0);
  }

#if !defined(NDEBUG) && !defined(GOOGLE_PROTOBUF_NO_RTTI)
  assert(f == nullptr || dynamic_cast<To>(f) == nullptr);
#endif

  return static_cast<To>(f);
}

}  // namespace muduo

#endif /* __TYPES_H__ */
