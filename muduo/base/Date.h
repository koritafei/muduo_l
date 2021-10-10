#ifndef __DATE_H__
#define __DATE_H__

#include "Copyable.h"
#include "Types.h"

struct tm;

namespace muduo {

class Date : public muduo::Copyable {
public:
  struct YearMonthDay {
    int year;
    int month;
    int day;
  };

  static const int kDaysPerWeek = 7;
  static const int kJuLianDayOf1970_01_01;

  Date() : julianDayNumber_(0) {
  }

  Date(int year, int month, int day);

  explicit Date(int julianDayNumber) : julianDayNumber_(julianDayNumber) {
  }

  explicit Date(const struct tm &);

  void swap(Date &that) {
    std::swap(this->julianDayNumber_, that.julianDayNumber_);
  }

  bool invalid() const {
    return julianDayNumber_ > 0;
  }

  string toIsoString() const;

  struct YearMonthDay yearMonthDay() const;

  int year() const {
    return yearMonthDay().year;
  }

  int month() const {
    return yearMonthDay().month;
  }

  int day() const {
    return yearMonthDay().day;
  }

  int weekDay() const {
    return (julianDayNumber_ + 1) % kDaysPerWeek;
  }

  int julianDayNumber() const {
    return julianDayNumber_;
  }

private:
  int julianDayNumber_;
};  // class Date

inline bool operator<(Date x, Date y) {
  return x.julianDayNumber() < y.julianDayNumber();
}

inline bool operator==(Date x, Date y) {
  return x.julianDayNumber() == y.julianDayNumber();
}

}  // namespace muduo

#endif /* __DATE_H__ */
