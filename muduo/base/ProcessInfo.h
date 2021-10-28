#ifndef __PROCESSINFO__H__
#define __PROCESSINFO__H__

#include <sys/types.h>

#include <vector>

#include "StringPiece.h"
#include "TimeStamp.h"
#include "Types.h"

namespace muduo {

namespace ProcessInfo {
pid_t     pid();
string    pidString();
uid_t     uid();
string    username();
uid_t     euid();
TimeStamp startTime();
int       clockTicksPerSecond();
int       pageSize();
bool      isDebugBuild();  // constexpr

string      hostname();
string      procname();
StringPiece procname(const string& stat);

/// read /proc/self/status
string procStatus();

/// read /proc/self/stat
string procStat();

/// read /proc/self/task/tid/stat
string threadStat();

/// readlink /proc/self/exe
string exePath();

int openedFiles();
int maxOpenFiles();

struct CpuTime {
  double userSeconds;
  double systemSeconds;

  CpuTime() : userSeconds(0.0), systemSeconds(0.0) {
  }

  double total() const {
    return userSeconds + systemSeconds;
  }
};
CpuTime cpuTime();

int                numThreads();
std::vector<pid_t> threads();
}  // namespace ProcessInfo

}  // namespace muduo

#endif /* __PROCESSINFO__H__ */
