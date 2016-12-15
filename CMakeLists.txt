cmake_minimum_required(VERSION 3.6)
project(os_lab2)

set(CMAKE_CXX_STANDARD 11)


set(SRC_LIST
        src/MultithreadExecutor.h 
        src/Worker.cpp
        src/Worker.h tests/TestMultithreadedExecutor.h)
set(TESTS_LIST
        )

set(ALL_CODE ${TESTS_LIST} ${SRC_LIST})

add_executable(os_lab2 src/main.cpp ${SRC_LIST})

find_package(CxxTest)
set(CXXTEST_TESTGEN_INTERPRETER bash)
include_directories(${CXXTEST_INCLUDE_DIR})
enable_testing()

FUNCTION(PREPEND var prefix)
    SET(listVar "")
    FOREACH(f ${ARGN})
        LIST(APPEND listVar "${prefix}/${f}")
    ENDFOREACH(f)
    SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

PREPEND(PATHS_FOR_CXXTEST ${CMAKE_CURRENT_SOURCE_DIR} ${ALL_CODE})
CXXTEST_ADD_TEST(tests testrunner.cpp ${PATHS_FOR_CXXTEST})
target_include_directories(tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
