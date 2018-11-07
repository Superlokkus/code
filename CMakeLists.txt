cmake_minimum_required(VERSION 3.0)
project(mkdt_interface VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)

set(TEST_ENABLED TRUE CACHE BOOL "Tests are enabled")

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

set(${PROJECT_NAME}_implementation_files
        ${CMAKE_CURRENT_SOURCE_DIR}/src/registry.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/registry.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/router.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/router.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/object_remoting_mockup.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/object_remoting_mockup.hpp
        )

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_implementation_files})

set_property(TARGET ${PROJECT_NAME} PROPERTY LINKER_LANGUAGE CXX)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

if (${TEST_ENABLED})
    enable_testing()
    set(${PROJECT_NAME}_test_src
            ${CMAKE_CURRENT_SOURCE_DIR}/test/registry_test.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/test/object_remoting_mockup_test.cpp
            )
    foreach (testSrc ${${PROJECT_NAME}_test_src})
        get_filename_component(testName ${testSrc} NAME_WE)
        add_executable(${PROJECT_NAME}_${testName} ${testSrc})
        set_property(TARGET ${PROJECT_NAME}_${testName} PROPERTY CXX_STANDARD 14)
        target_link_libraries(${PROJECT_NAME}_${testName} ${Boost_LIBRARIES} ${PROJECT_NAME})
        add_test(NAME ${PROJECT_NAME}_${testName} COMMAND ${PROJECT_NAME}_${testName} -m HRF -l warning --build_info)
    endforeach (testSrc)
endif ()

