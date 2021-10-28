#ifndef __LOGGING_H__
#define __LOGGING_H__

#include "LogStream.h"
#include "TimeStamp.h"

namespace muduo {

class TimeZone;

class Logger {
public:
  enum LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVEL,
  };  // enum LogLevel

  class SourceFile {
  public:
    const char* data_;
    int         size_;

    template <int N>
    SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1) {
      const char* slash = strrchr(data_, '/');
      if (slash) {
        data_ = slash + 1;
        size_ -= static_cast<int>(strlen(data_));
      }
    }

  };  // class SourceFile

  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char* func);
  Logger(SourceFile file, int line, bool toAbort);

  ~Logger();

  LogStream& stream() {
    return impl_.stream_;
  }

  static LogLevel logLevel();
  static void     setLogLevel(LogLevel level);
  typedef void (*OutputFunc)(const char* msg, int len);
  typedef void (*FlushFunc)();
  static void setOutput(OutputFunc);
  static void setFlush(FlushFunc);
  static void setTimeZone(const TimeZone& tz);

private:
  class Impl {
  public:
    typedef Logger::LogLevel LogLevel;

    Impl(LogLevel level, int old_errno, const SourceFile& file, int lien);

    void formatTime();
    void finish();

    TimeStamp  time_;
    LogStream  stream_;
    LogLevel   level_;
    int        line_;
    SourceFile basename_;
  };  // class Impl

  Impl impl_;
};  // class Logger

extern Logger::LogLevel g_loglevel;

inline Logger::LogLevel Logger::logLevel() {
  return g_loglevel;
}

#define LOG_TRACE                                                              \
  if (muduo::Logger::logLevel() <= muduo::Logger::TRACE)                       \
  muduo::Logger(__FILE__, __LINE__, muduo::Logger::TRACE, __func__).stream()

#define LOG_DEBUG                                                              \
  if (muduo::Logger::logLevel() <= muduo::Logger::DEBUG)                       \
  muduo::Logger(__FILE__, __LINE__, muduo::Logger::DEBUG, __func__).stream()

#define LOG_INFO                                                               \
  if (muduo::Logger::logLevel() <= muduo::Logger::INFO)                        \
  muduo::Logger(__FILE__, __LINE__).stream()

#define LOG_WARN muduo::Logger(__FILE__, __LINE__, muduo::Logger::WARN).stream()

#define LOG_ERROR                                                              \
  muduo::Logger(__FILE__, __LINE__, muduo::Logger::ERROR).stream()

#define LOG_FATAL                                                              \
  muduo::Logger(__FILE__, __LINE__, muduo::Logger::FATAL).stream()

#define LOG_SYSERR   muduo::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL muduo::Logger(__FILE__, __LINE__, true).stream()

const char* strerror_tl(int savedErrno);

#define CHECK_NOTNULL(val)                                                     \
  ::muduo::CheckNotNull(__FILE__,                                              \
                        __LINE__,                                              \
                        "'" #val "' MUST be not NULL",                         \
                        (val))

template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char* names, T* ptr) {
  if (nullptr == ptr) {
    Logger(file, line, Logger::FATAL).stream() << names;
  }

  return ptr;
}

}  // namespace muduo

#endif /* __LOGGING_H__ */
