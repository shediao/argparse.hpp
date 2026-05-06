set(CMAKE_CXX_STANDARD 20)

set(INSTALL_GTEST OFF)

include(FetchContent)

if(NOT CMAKE_HOST_SYSTEM_NAME MATCHES "Windows" AND MSVC)
  set(gtest_disable_pthreads
      ON
      CACHE BOOL "" FORCE)
endif()
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(googletest)

# Suppress -Wcharacter-conversion for gtest/gmock when using Clang and C++20.
# char8_t (C++20) to char32_t implicit conversion in gtest-printers.h triggers
# this warning, which becomes an error under -WX.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  foreach(gtarget gtest gtest_main gmock gmock_main)
    if(TARGET ${gtarget})
      target_compile_options(${gtarget} PRIVATE -Wno-character-conversion)
    endif()
  endforeach()
endif()

function(add_argparse_test test_name test_files)
  add_executable(${test_name} ${test_files})
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
     OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
     OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    if(NOT MSVC)
      target_compile_options(${test_name} PRIVATE -Wall -Wextra -Werror)
    endif()
  endif()
  if(MSVC)
    target_compile_options(${test_name} PRIVATE /utf-8 /EHsc /W4)
  endif()
  target_link_libraries(${test_name} PRIVATE argparse::argparse GTest::gtest
                                             GTest::gtest_main)
  add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

file(GLOB test_files "*.cc")

foreach(test_file ${test_files})
  get_filename_component(test_name ${test_file} NAME_WE)
  add_argparse_test(${test_name} ${test_file})
endforeach()

add_argparse_test(all_test "${test_files}")
