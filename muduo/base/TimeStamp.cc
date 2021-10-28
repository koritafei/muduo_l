#include "TimeStamp.h"

#include <inttypes.h>
#include <stdio.h>
#include <sys/time.h>

using namespace muduo;

static_assert(sizeof(TimeStamp) == sizeof(int64_t),
              "TimeStamp shouble be same size as int64_t");

string TimeStamp::toString() const {
  char    buf[32] = {0};
  int64_t seconds = microSecondsSinceEpoch_ / TimeStamp::kMicroSecondsPerSecond;
  int64_t microSeconds =
      microSecondsSinceEpoch_ % TimeStamp::kMicroSecondsPerSecond;

  snprintf(buf,
           sizeof(buf),
           "%" PRId64 "%.06" PRId64 "",
           seconds,
           microSeconds);
  return buf;
}

string TimeStamp::toFormattedString(bool showMicroSeconds) const {
  char      buf[64] = {0};
  time_t    seconds = static_cast<time_t>(microSecondsSinceEpoch_ /
                                       TimeStamp::kMicroSecondsPerSecond);
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);

  if (showMicroSeconds) {
    int microSeconds = static_cast<int>(microSecondsSinceEpoch_) %
                       TimeStamp::kMicroSecondsPerSecond;
    snprintf(buf,
             sizeof(buf),
             "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900,
             tm_time.tm_mon + 1,
             tm_time.tm_mday,
             tm_time.tm_hour,
             tm_time.tm_min,
             tm_time.tm_sec,
             microSeconds);
  } else {
    snprintf(buf,
             sizeof(buf),
             "%4d%02d%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900,
             tm_time.tm_mon + 1,
             tm_time.tm_mday,
             tm_time.tm_hour,
             tm_time.tm_min,
             tm_time.tm_sec);
  }

  return buf;
}

TimeStamp TimeStamp::now() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  int64_t seconds = tv.tv_sec;

  return TimeStamp(seconds * TimeStamp::kMicroSecondsPerSecond + tv.tv_usec);
}