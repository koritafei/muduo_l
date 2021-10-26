#ifndef __TimeStamp_H__
#define __TimeStamp_H__

#include <boost/operators.hpp>

#include "Copyable.h"
#include "Types.h"

namespace muduo {

class TimeStamp : public muduo::Copyable,
                  public boost::equality_comparable<TimeStamp>,
                  public boost::less_than_comparable<TimeStamp> {
public:
  TimeStamp() : microSecondsSinceEpoch_(0) {
  }

  explicit TimeStamp(int64_t microSecondsSinceEpoch)
      : microSecondsSinceEpoch_(microSecondsSinceEpoch) {
  }

  void swap(TimeStamp &that) {
    std::swap(this->microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
  }

  string toString() const;
  string toFormattedString(bool showMicroSeconds = true) const;

  bool valid() const {
    return microSecondsSinceEpoch_ > 0;
  }

  int64_t microSecondsSinceEpoch() const {
    return microSecondsSinceEpoch_;
  }

  time_t secondsSinceEpoch() const {
    return static_cast<time_t>(microSecondsSinceEpoch_ /
                               kMicroSecondsPerSecond);
  }

  static TimeStamp now();

  static TimeStamp invalid() {
    return TimeStamp();
  }

  static TimeStamp fromUnixTime(time_t t) {
    return fromUnixTime(t, 0);
  }

  static TimeStamp fromUnixTime(time_t t, int microSeconds) {
    return TimeStamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond +
                     microSeconds);
  }

  static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
  int64_t microSecondsSinceEpoch_;
};  // class TimeStamp

inline bool operator<(TimeStamp lhs, TimeStamp rhs) {
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(TimeStamp &lhs, TimeStamp &rhs) {
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline double timeDifference(TimeStamp &high, TimeStamp &low) {
  int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();

  return static_cast<double>(diff) / TimeStamp::kMicroSecondsPerSecond;
}

inline TimeStamp addTime(TimeStamp t, double seconds) {
  int64_t delta =
      static_cast<int64_t>(seconds * TimeStamp::kMicroSecondsPerSecond);

  return TimeStamp(t.microSecondsSinceEpoch() + delta);
}

}  // namespace muduo

#endif /* __TimeStamp_H__ */
