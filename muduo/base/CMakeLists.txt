set(BASE_SRCS 
    TimeStamp.cc
    Date.cc
    TimeZone.cc
    Thread.cc
    CurrentThread.cc 
    Exception.cc
    LogStream.cc 
    Logging.cc
    CountDownLatch.cc
    Condition.cc)

add_library(muduo_base ${BASE_SRCS})
target_link_libraries(muduo_base pthread rt)

install(TARGETS muduo_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/muduo/base)

if(MUDUO_BUILD_EXAMPLES)
  add_subdirectory(tests)
endif()

