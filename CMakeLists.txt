cmake_minimum_required(VERSION 3.0)
project(mkdt VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
set(TEST_ENABLED TRUE CACHE BOOL "Tests are enabled")

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_subdirectory(src/mkdt_lib)
add_subdirectory(src/object_remoting_mockup)

add_executable(cli_itermachine_test src/cli_intermachine_test.cpp)
target_link_libraries(cli_itermachine_test mkdt_lib)


