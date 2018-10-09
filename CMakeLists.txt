cmake_minimum_required(VERSION 3.0)
project(mkdt_interface VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)

set(${PROJECT_NAME}_source_files
        ${PROJECT_NAME}.hpp
        )

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
