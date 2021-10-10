#include "../Date.h"

#include <assert.h>
#include <stdio.h>

using muduo::Date;

const int kMonthsOfYear = 12;

int isLeapYear(int year) {
  if (0 == year / 400) {
    return 1;
  } else if (0 == year % 100) {
    return 0;
  } else if (0 == year % 4) {
    return 1;
  }

  return 0;
}

int dayOfMOnth(int year, int month) {
  static int days[2][kMonthsOfYear + 1] = {
      {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
      {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

  return days[isLeapYear(year)][month];
}

void passByConstReference(const Date &x) {
  printf("passByConstReference %s\n", x.toIsoString().c_str());
}

void passByValue(Date &x) {
  printf("passByValue %s\n", x.toIsoString().c_str());
}

int main(int argc, char **argv) {
  time_t    now = time(NULL);
  struct tm t1  = *gmtime(&now);
  struct tm t2  = *localtime(&now);

  Date someday(2021, 10, 9);
  printf("main thread someday %s\n", someday.toIsoString().c_str());
  passByConstReference(someday);
  passByValue(someday);

  Date todayUTC(t1);
  printf("Date todayUTC %s\n", todayUTC.toIsoString().c_str());
  Date todayLocal(t2);
  printf("Date todayLocal %s\n", todayLocal.toIsoString().c_str());

  int julianDayNumber = 2415021;
  int weekDay         = 1;

  for (int year = 1900; year < 2500; year++) {
    assert(Date(year, 3, 1).julianDayNumber() -
               Date(year, 2, 29).julianDayNumber() ==
           isLeapYear(year));

    for (int month = 1; month <= kMonthsOfYear; month++) {
      for (int day = 1; day <= dayOfMOnth(year, month); day++) {
        Date d(year, month, day);
        printf("%s %d\n", d.toIsoString().c_str(), d.weekDay());
        assert(year == d.year());
        assert(month == d.month());
        assert(day == d.day());
        assert(weekDay == d.weekDay());
        assert(julianDayNumber == d.julianDayNumber());

        Date d2(julianDayNumber);
        assert(year == d2.year());
        assert(month == d2.month());
        assert(day == d2.day());
        assert(weekDay == d2.weekDay());
        assert(julianDayNumber == d2.julianDayNumber());

        ++julianDayNumber;
        weekDay = (weekDay + 1) % 7;
      }
    }
  }
  printf("All passed.\n");
}