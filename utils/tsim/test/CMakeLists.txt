CMAKE_MINIMUM_REQUIRED(VERSION 2.8...3.20)
PROJECT(TDengine)

FIND_PATH(HEADER_GTEST_INCLUDE_DIR gtest.h /usr/include/gtest /usr/local/include/gtest)
FIND_LIBRARY(LIB_GTEST_STATIC_DIR libgtest.a /usr/lib/ /usr/local/lib /usr/lib64)
FIND_LIBRARY(LIB_GTEST_SHARED_DIR libgtest.so /usr/lib/ /usr/local/lib /usr/lib64)

IF(HEADER_GTEST_INCLUDE_DIR AND(LIB_GTEST_STATIC_DIR OR LIB_GTEST_SHARED_DIR))
    MESSAGE(STATUS "gTest library found, build os test")

    INCLUDE_DIRECTORIES(${HEADER_GTEST_INCLUDE_DIR})
    AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR} SOURCE_LIST)
ENDIF()

INCLUDE_DIRECTORIES(${TD_SOURCE_DIR}/src/util/inc)

IF(TD_LINUX)
    ADD_EXECUTABLE(simTests "simTests.cpp")
    TARGET_LINK_LIBRARIES(simTests os util tsim_static gtest_main)

    ADD_TEST(
        NAME simTests
        COMMAND simTests
    )
ENDIF()