#include "Logging.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <cstdlib>
#include <sstream>

#include "CurrentThread.h"
#include "TimeStamp.h"
#include "TimeZone.h"

namespace muduo {

__thread char   t_errnnobuf[512];
__thread char   t_time[64];
__thread time_t t_lastSecond;

const char *strerror_tl(int savedErrno) {
  return strerror_r(savedErrno, t_errnnobuf, sizeof t_errnnobuf);
}

Logger::LogLevel initLogLevel() {
  if (::getenv("MUDUO_LOG_TRACE")) {
    return Logger::TRACE;
  } else if (::getenv("MUDUO_LOG_DEBUG")) {
    return Logger::DEBUG;
  } else {
    return Logger::INFO;
  }
}

Logger::LogLevel g_loglevel = initLogLevel();

const char *LogLevelName[Logger::NUM_LOG_LEVELS] = {
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL ",
};

class T {
public:
  const char *   str_;
  const unsigned length_;

  T(const char *str, unsigned length) : str_(str), length_(length) {
    assert(strlen(str) == length_);
  }

};  // class T

inline LogStream &operator<<(LogStream &s, T v) {
  s.append(v.str_, v.length_);
  return s;
}

inline LogStream &operator<<(LogStream &s, Logger::SourceFile v) {
  s.append(v.data_, v.size_);
  return s;
}

void defaultOutput(const char *msg, int len) {
  size_t n = fwrite(msg, 1, len, stdout);
  (void)n;
}

void defaultFlush() {
  fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc  g_flush  = defaultFlush;
TimeZone           g_logTimeZone;
}  // namespace muduo

using namespace muduo;

Logger::Impl::Impl(LogLevel          level,
                   int               savedErrno,
                   const SourceFile &file,
                   int               line)
    : time_(TimeStamp::now()),
      stream_(),
      level_(level),
      line_(line),
      basename_(file) {
  formatTime();
  CurrentThread::tid();

  stream_ << T(CurrentThread::tidString(), CurrentThread::tidStringLength());
  stream_ << T(LogLevelName[level], 6);
  if (savedErrno != 0) {
    stream_ << strerror_tl(savedErrno) << " ( errno = " << savedErrno << " ) ";
  }
}

void Logger::Impl::formatTime() {
  int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();

  time_t seconds      = static_cast<time_t>(microSecondsSinceEpoch /
                                       TimeStamp::kMicroSecondsPerSecond);
  int    microSeconds = static_cast<int>(microSecondsSinceEpoch %
                                      TimeStamp::kMicroSecondsPerSecond);

  if (t_lastSecond != seconds) {
    t_lastSecond = seconds;
    struct tm tm_time;

    if (g_logTimeZone.valid()) {
      tm_time = g_logTimeZone.toLocalTime(seconds);
    } else {
      ::gmtime_r(&seconds, &tm_time);
    }

    int len = snprintf(t_time,
                       sizeof t_time,
                       "%4d%02d%02d %02d:%02d:%02d",
                       tm_time.tm_year + 1900,
                       tm_time.tm_mon + 1,
                       tm_time.tm_mday,
                       tm_time.tm_hour,
                       tm_time.tm_min,
                       tm_time.tm_min);
    assert(len == 17);
    (void)len;
  }

  if (g_logTimeZone.valid()) {
    Fmt us(".%06d ", microSeconds);
    assert(us.length() == 8);
    stream_ << T(t_time, 17) << T(us.data(), 8);
  } else {
    Fmt us(".%06dZ ", microSeconds);
    assert(us.length() == 9);
    stream_ << T(t_time, 17) << T(us.data(), 9);
  }
}

void Logger::Impl::finish() {
  stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::Logger(SourceFile file, int line) : impl_(INFO, 0, file, line) {
}

Logger::Logger(SourceFile file, int line, LogLevel level, const char *func)
    : impl_(level, 0, file, line) {
  impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level)
    : impl_(level, 0, file, line) {
}

Logger::Logger(SourceFile file, int line, bool toAbort)
    : impl_(toAbort ? FATAL : ERROR, errno, file, line) {
}

Logger::~Logger() {
  impl_.finish();
  const LogStream::Buffer &buf(stream().buffer());

  g_output(buf.data(), buf.length());
  if (FATAL == impl_.level_) {
    g_flush();
    abort();
  }
}

void Logger::setLogLevel(Logger::LogLevel level) {
  g_loglevel = level;
}

void Logger::setOutput(OutputFunc out) {
  g_output = out;
}

void Logger::setFlush(FlushFunc flush) {
  g_flush = flush;
}

void Logger::setTimeZone(const TimeZone &tz) {
  g_logTimeZone = tz;
}