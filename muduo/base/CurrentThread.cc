#include <cxxabi.h>
#include <execinfo.h>
#include <stdio.h>

#include <iostream>

namespace muduo {

namespace CurrentThread {

__thread int         t_cachedTid = 0;
__thread char        t_tidString[32];
__thread int         t_tidStringLength = 6;
__thread const char *t_threadName      = "unkonwn";

static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");

std::string stackTrace(bool demangle) {
  std::string track;
  const int   max_frames = 200;
  void *      frame[max_frames];

  int    nptrs   = ::backtrace(frame, max_frames);
  char **strings = ::backtrace_symbols(frame, nptrs);

  if (strings) {
    size_t len       = 256;
    char * demangled = demangle ? static_cast<char *>(::malloc(len)) : nullptr;

    for (int i = 1; i < nptrs; ++i) {
      if (demangle) {
        char *left_par = nullptr;
        char *plus     = nullptr;

        for (char *p = strings[i]; *p; ++p) {
          if ('(' == *p) {
            left_par = p;
          } else if ('+' == *p) {
            plus = p;
          }
        }

        if (left_par && plus) {
          *plus        = '\0';
          int   status = 0;
          char *ret =
              abi::__cxa_demangle(left_par + 1, demangled, &len, &status);

          *plus = '+';

          if (0 == status) {
            demangled = ret;
            track.append(strings[i], left_par + 1);
            track.append(demangled);
            track.append(plus);
            track.push_back('\n');
            continue;
          }
        }
      }

      track.append(strings[i]);
      track.push_back('\n');
    }

    free(demangled);
    free(strings);
  }

  return track;
}

}  // namespace CurrentThread

}  // namespace muduo

#include "CurrentThread.h"
