#ifndef __TYPES_H__
#define __TYPES_H__

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>

#include <boost/operators.hpp>
#include <cstddef>

namespace muduo {

using std::string;

inline void memZero(void *p, size_t n) {
  memset(p, 0, n);
}

template <typename To, typename From>
inline To implicit_cast(From const &f) {
  return f;
}

template <typename To, typename From>
inline To down_cast(From *f) {
  if (false) {
    implicit_cast<From *, To>(0);
  }

  return static_cast<To>(f);
}

}  // namespace muduo

#endif /* __TYPES_H__ */
