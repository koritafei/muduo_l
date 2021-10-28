#include "../TimeStamp.h"

#include <stdio.h>

#include <vector>

using muduo::TimeStamp;

void passByConstReference(const TimeStamp &x) {
  printf("passByConstReference %s\n", x.toString().c_str());
}

void passByValue(TimeStamp x) {
  printf("passByValue %s\n", x.toString().c_str());
}

void benchmark() {
  const int              kNumber = 1000 * 1000;
  std::vector<TimeStamp> stamps;
  stamps.reserve(kNumber);

  for (int i = 0; i < kNumber; i++) {
    stamps.push_back(TimeStamp::now());
  }

  printf("%s\n", stamps.front().toString().c_str());
  printf("%s\n", stamps.back().toString().c_str());
  printf("%f\n", muduo::timeDifference(stamps.back(), stamps.front()));

  int     increment[100] = {0};
  int64_t start          = stamps.front().microSecondsSinceEpoch();

  for (size_t i = 1; i < kNumber; ++i) {
    int64_t next = stamps[i].microSecondsSinceEpoch();
    int64_t inc  = next - start;
    start        = next;

    if (0 > inc) {
      printf("reverse\n");
    } else if (100 > inc) {
      ++increment[inc];
    } else {
      printf("big gap %d\n", static_cast<int>(inc));
    }
  }

  for (int i = 0; i < 100; i++) {
    printf("%2d: %d\n", i, increment[i]);
  }
}

int main(int argc, char **argv) {
  TimeStamp now = TimeStamp::now();
  printf("main thread %s\n", now.toString().c_str());
  passByConstReference(now);
  passByValue(now);
  benchmark();
}
