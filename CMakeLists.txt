cmake_minimum_required(VERSION 3.0)
project(mkdt VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
set(TEST_ENABLED TRUE CACHE BOOL "Tests are enabled")

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
find_package(Boost COMPONENTS log system thread REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_subdirectory(src/mkdt_lib)
add_subdirectory(src/object_remoting_mockup)

add_executable(cli_itermachine_test src/cli_intermachine_test.cpp)
target_link_libraries(cli_itermachine_test mkdt_lib object_remoting_mockup ${Boost_LIBRARIES})


