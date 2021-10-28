set(BASE_SRCS
  AsyncLogging.cc
  Condition.cc
  CountDownLatch.cc
  CurrentThread.cc
  Date.cc
  Exception.cc
  FileUtil.cc
  LogFile.cc
  Logging.cc
  LogStream.cc
  ProcessInfo.cc
  TimeStamp.cc
  Thread.cc
  ThreadPool.cc
  TimeZone.cc
  )

add_library(muduo_base ${BASE_SRCS})
target_link_libraries(muduo_base pthread rt)

install(TARGETS muduo_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/muduo/base)

if(MUDUO_BUILD_EXAMPLES)
  add_subdirectory(tests)
endif()

