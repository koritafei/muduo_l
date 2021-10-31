
add_executable(echoserver_unittest EchoServer_unittest.cc)
target_link_libraries(echoserver_unittest muduo_net)

add_executable(echoclient_unittest EchoClient_unittest.cc)
target_link_libraries(echoclient_unittest muduo_net)

add_executable(EventLoop_unittest EventLoop_unittest.cc)
target_link_libraries(EventLoop_unittest muduo_net)

add_executable(TimeQueue_unittest TimeQueue_unittest.cc)
target_link_libraries(TimeQueue_unittest muduo_net)

if(BOOSTTEST_LIBRARY)
add_executable(buffer_unittest Buffer_unittest.cc)
target_link_libraries(buffer_unittest muduo_net boost_unit_test_framework)
add_test(NAME buffer_unittest COMMAND buffer_unittest)
endif()

add_executable(Channel_unittest Channel_unittest.cc)
target_link_libraries(Channel_unittest muduo_net)

if(BOOSTTEST_LIBRARY)
add_executable(InetAddress_unittest InetAddress_unittest.cc)
target_link_libraries(InetAddress_unittest muduo_net boost_unit_test_framework)
add_test(NAME InetAddress_unittest COMMAND InetAddress_unittest)
endif()

add_executable(EventLoopThread_unittest EventLoopThread_unittest.cc)
target_link_libraries(EventLoopThread_unittest muduo_net)

add_executable(EventLoopThreadPool_unittest EventLoopThreadPool_unittest.cc)
target_link_libraries(EventLoopThreadPool_unittest muduo_net)

if(BOOSTTEST_LIBRARY)
if(ZLIB_FOUND)
  add_executable(zlibstream_unittest ZlibStream_unittest.cc)
  target_link_libraries(zlibstream_unittest muduo_net boost_unit_test_framework z)
  # set_target_properties(zlibstream_unittest PROPERTIES COMPILE_FLAGS "-std=c++0x")
endif()
endif()