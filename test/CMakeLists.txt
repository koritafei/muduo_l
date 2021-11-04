add_executable(threadDemo threaddemo.cc)
TARGET_LINK_LIBRARIES(threadDemo pthread)


# add_executable(MutexLockDemo MutexLockDemo.cc)
# TARGET_LINK_LIBRARIES(MutexLockDemo pthread)

add_executable(echosrv_poll echosrv_poll.cc)
add_executable(echocli echocli.cc)
add_executable(boostbind boostbind.cc)

add_executable(DeadLock DeadLock.cc)
target_link_libraries(DeadLock muduo_base)

