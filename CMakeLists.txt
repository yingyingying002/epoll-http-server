cmake_minimum_required( VERSION 3.1 )

#工程名
PROJECT(Epoll_Http_Server)

#设置生成文件名
SET(CLIENT_TARGET "kclient.out")
SET(SERVER_TARGET "kserver.out")
#生成静态库
add_library(httpd ${PROJECT_SOURCE_DIR}/depdence/httpd.h ${PROJECT_SOURCE_DIR}/depdence/httpd.c)

#头文件搜索目录
include_directories(
  ${PROJECT_SOURCE_DIR}/depdence
)

#库文件搜索目录
link_directories(
  ${PROJECT_SOURCE_DIR}/depdence
)

#设置可执行文件的输出目录
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

#设置库文件的输出目录
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/depdence)

link_libraries(${PROJECT_SOURCE_DIR}/depdence/libhttpd.a)

#设定SRC变量 指定所有源代码路径
set(SRC_CLIENT
  ${PROJECT_SOURCE_DIR}/client/main.cc
)

set(SRC_SERVER
  ${PROJECT_SOURCE_DIR}/server/Server.cc
  ${PROJECT_SOURCE_DIR}/server/main.cc
)

#生成可执行文件
add_executable(${CLIENT_TARGET} ${SRC_CLIENT})
add_executable(${SERVER_TARGET} ${SRC_SERVER})

#添加 -pthread 编译选项 支持多线程
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${CLIENT_TARGET} PRIVATE Threads::Threads)
target_link_libraries(${SERVER_TARGET} PRIVATE Threads::Threads httpd)