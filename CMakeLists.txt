cmake_minimum_required(VERSION 3.30)
project(argparser)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
find_package(GTest REQUIRED)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

link_libraries(GTest::gtest GTest::gtest_main)

add_executable(argparser_fromstring_unittest test/argparser_fromstring_unittest.cc)
add_executable(argparser_unittest test/argparser_unittest.cc)
