#ifndef __STRINGPIECE_H__
#define __STRINGPIECE_H__

#include <string.h>

#include <cmath>
#include <iosfwd>

#include "Types.h"

namespace muduo {

class StringArg {
public:
  StringArg(const char *str) : str_(str) {
  }

  StringArg(const std::string &str) : str_(str.c_str()) {
  }

  const char *c_str() const {
    return str_;
  }

private:
  const char *str_;
};  // class StringArg

class StringPiece {
public:
  StringPiece() : ptr_(nullptr), length_(0) {
  }

  StringPiece(const char *str)
      : ptr_(str), length_(static_cast<int>(strlen(str))) {
  }

  StringPiece(const unsigned char *str)
      : ptr_(reinterpret_cast<const char *>(str)),
        length_(static_cast<int>(strlen(ptr_))) {
  }

  StringPiece(std::string &str)
      : ptr_(str.data()), length_(static_cast<int>(str.size())) {
  }

  StringPiece(const char *offset, int len) : ptr_(offset), length_(len) {
  }

  const char *data() const {
    return ptr_;
  }

  int size() const {
    return length_;
  }

  bool empty() const {
    return length_ == 0;
  }

  const char *begin() const {
    return ptr_;
  }

  const char *end() const {
    return ptr_ + length_;
  }

  void clear() {
    ptr_    = nullptr;
    length_ = 0;
  }

  void set(const char *buff, int len) {
    ptr_    = buff;
    length_ = len;
  }

  void set(const char *str) {
    ptr_    = str;
    length_ = static_cast<int>(strlen(str));
  }

  void set(const void *buffer, int len) {
    ptr_    = static_cast<const char *>(buffer);
    length_ = len;
  }

  char operator[](int i) const {
    return ptr_[i];
  }

  void remove_prefix(int n) {
    ptr_ += n;
    length_ -= n;
  }

  void remove_suffix(int n) {
    length_ -= n;
  }

  bool operator==(const StringPiece &x) {
    return ((length_ == x.length_) && memcmp(ptr_, x.ptr_, length_) == 0);
  }

  bool operator!=(const StringPiece &x) {
    return !(*this == x);
  }

#define STRINGPIECE_BINARY_PREDICATE(cmp, auxcmp)                              \
  bool operator cmp(const StringPiece &x) const {                              \
    int r = memcmp(ptr_, x.ptr_, length_ < x.length_ ? length_ : x.length_);   \
    return ((r auxcmp 0) || ((r == 0) && (length_ cmp x.length_)));            \
  }
  STRINGPIECE_BINARY_PREDICATE(<, <);
  STRINGPIECE_BINARY_PREDICATE(<=, <);
  STRINGPIECE_BINARY_PREDICATE(>=, >);
  STRINGPIECE_BINARY_PREDICATE(>, >);
#undef STRINGPIECE_BINARY_PREDICATE

  int compare(const StringPiece &x) const {
    int r = memcmp(ptr_, x.ptr_, length_ < x.length_ ? length_ : x.length_);
    if (0 == r) {
      if (length_ < x.length_) {
        r = -1;
      }
      if (length_ > x.length_) {
        r = +1;
      }
    }

    return r;
  }

  std::string as_string() const {
    return std::string(data(), size());
  }

  void copyToString(std::string *target) {
    target->assign(ptr_, length_);
  }

  bool startWith(const StringPiece &x) {
    return ((length_ >= x.length_) && (memcmp(ptr_, x.ptr_, x.length_) == 0));
  }

private:
  const char *ptr_;
  int         length_;
};  // class StringPiece

}  // namespace muduo

#ifdef HAVE_TYPE_TRIVIAL

template <>
struct __type_traits<muduo::StringPiece> {
  typedef std::__true_type has_trivial_default_constructor;
  typedef std::__true_type has_trivial_copy_constructor;
  typedef std::__true_type has_trivial_assignment_operator;
  typedef std::__true_type has_trivial_destructor;
  typedef std::__true_type is_POD_type;
};

#endif

std::ostream &operator<<(std::ostream &            o,
                         const muduo::StringPiece &stringpiece);

#endif /* __STRINGPIECE_H__ */
