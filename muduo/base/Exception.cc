#include "CurrentThread.h"
#include "Exception.h"

namespace muduo {

Exception::Exception(std::string message)
    : message_(std::move(message)), stack_(CurrentThread::stackTrace(false)) {
}

}  // namespace muduo
