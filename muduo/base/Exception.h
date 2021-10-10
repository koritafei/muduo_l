#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <exception>

#include "Types.h"

namespace muduo {

class Exception : public std::exception {
public:
  Exception(std::string waht);
  ~Exception() noexcept override = default;

  const char *what() const noexcept override {
    return message_.c_str();
  }

  const char *stackTrace() const noexcept {
    return stack_.c_str();
  }

private:
  std::string message_;
  std::string stack_;
};  // class Exception

}  // namespace muduo

#endif /* __EXCEPTION_H__ */
