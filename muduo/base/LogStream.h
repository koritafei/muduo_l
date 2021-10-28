#ifndef __LOGSTREAM_H__
#define __LOGSTREAM_H__

#include <assert.h>
#include <string.h>

#include "Noncopyable.h"
#include "StringPiece.h"
#include "Types.h"

namespace muduo {

namespace detail {

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template <int SIZE>
class FixedBuffer : Noncopyable {
public:
  FixedBuffer() : cur_(data_) {
    setCookie(cookieStart);
  }

  ~FixedBuffer() {
    setCookie(cookieEnd);
  }

  void append(const char* buf, size_t len) {
    if (implicit_cast<size_t>(avail()) > len) {
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }

  const char* data() const {
    return data_;
  }

  int length() const {
    return static_cast<int>(cur_ - data_);
  }

  char* current() {
    return cur_;
  }

  int avail() {
    return static_cast<int>(end() - cur_);
  }

  void add(size_t len) {
    cur_ += len;
  }

  void reset() {
    cur_ = data_;
  }

  void bzero() {
    memZero(data_, sizeof data_);
  }

  const char* debugString();
  void        setCookie(void (*cookie)()) {
    cookie_ = cookie;
  }

  std::string toString() const {
    return std::string(data_, length());
  }

  StringPiece toStringPiece() const {
    return StringPiece(data_, length());
  }

private:
  char  data_[SIZE];
  char* cur_;
  void (*cookie_)();

  static void cookieStart();
  static void cookieEnd();
  const char* end() const {
    return data_ + sizeof(data_);
  }

};  // class FixedBuffer

}  // namespace detail

class LogStream : Noncopyable {
private:
  typedef LogStream self;

public:
  typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;

  self& operator<<(bool v) {
    buffer_.append(v ? "1" : "0", 1);
    return *this;
  }

  self& operator<<(short);
  self& operator<<(unsigned short);
  self& operator<<(int);
  self& operator<<(unsigned int);
  self& operator<<(long);
  self& operator<<(unsigned long);
  self& operator<<(long long);
  self& operator<<(unsigned long long);

  self& operator<<(const void*);

  self& operator<<(float f) {
    *this << static_cast<double>(f);
    return *this;
  }

  self& operator<<(double);

  self& operator<<(char v) {
    buffer_.append(&v, 1);
    return *this;
  }

  self& operator<<(const char* str) {
    if (str) {
      buffer_.append(str, sizeof str);
    } else {
      buffer_.append("null", 6);
    }

    return *this;
  }

  self& operator<<(const unsigned char* str) {
    return operator<<(reinterpret_cast<const char*>(str));
  }

  self& operator<<(const std::string& str) {
    buffer_.append(str.c_str(), str.size());
    return *this;
  }

  self& operator<<(const StringPiece& str) {
    buffer_.append(str.data(), str.size());
    return *this;
  }

  self& operator<<(const Buffer& v) {
    *this << v.toStringPiece();
    return *this;
  }

  void append(const char* data, int len) {
    buffer_.append(data, len);
  }

  const Buffer& buffer() const {
    return buffer_;
  }

  void resetBuffer() {
    buffer_.reset();
  }

private:
  Buffer           buffer_;
  static const int kMaxNumericSize = 48;

  void staticCheck();

  template <typename T>
  void formatInteger(T v);

};  // class LogStream

class Fmt {
public:
  template <typename T>
  Fmt(const char* fmt, T val);

  const char* data() const {
    return buf_;
  }

  int length() const {
    return length_;
  }

private:
  char buf_[32];
  int  length_;
};  // class Fmt

inline LogStream& operator<<(LogStream& ls, const Fmt& fmt) {
  ls.append(fmt.data(), fmt.length());
  return ls;
}

std::string formatSI(int64_t n);

std::string formatICE(int64_t n);

}  // namespace muduo

#endif /* __LOGSTREAM_H__ */
