set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2)

FetchContent_MakeAvailable(googletest)

function(add_argparse_test test_name)
  add_executable(${test_name} ${test_name}.cc)
  target_link_libraries(${test_name} PRIVATE argparse::argparse GTest::gtest
                                             GTest::gtest_main)
  add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_argparse_test(argparse_fromstring_unittest)
add_argparse_test(argparse_unittest)
add_argparse_test(argparse_parse_unittest)
add_argparse_test(argparse_parse_gnu_coreutils_unittest)
