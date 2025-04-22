cmake_minimum_required(VERSION 3.15)
project(
  argparse
  VERSION "0.1"
  LANGUAGES CXX)

set("PROJECT_DESCRIPTION"
    "A header-only lightweight C++ command line option parser")
set("PROJECT_HOMEPAGE_URL" "https://github.com/shediao/argparse.hpp")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  option(ARGPARSE_BUILD_TESTS "Set to ON to build tests" ON)
else()
  option(ARGPARSE_BUILD_TESTS "Set to OFF to build tests" OFF)
endif()

add_library(argparse INTERFACE)

target_include_directories(
  argparse INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                     $<INSTALL_INTERFACE:include>)

target_compile_features(argparse INTERFACE cxx_std_20)
add_library(argparse::argparse ALIAS argparse)

if(ARGPARSE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

install(
  TARGETS argparse
  EXPORT argparse-targets
  INCLUDES
  DESTINATION include)

install(
  DIRECTORY include/
  DESTINATION include
  FILES_MATCHING
  PATTERN "*.hpp")

install(
  EXPORT argparse-targets
  FILE argparse-targets.cmake
  NAMESPACE argparse::
  DESTINATION lib/cmake/argparse)
