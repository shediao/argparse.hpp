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
  if(Clang
     OR AppleClang
     OR GNU)
    target_compile_options(${test_name} PRIVATE -Wall -Wextra -Werror)
  endif()
  target_link_libraries(${test_name} PRIVATE argparse::argparse GTest::gtest
                                             GTest::gtest_main)
  add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

file(GLOB test_files "*.cc")

foreach(test_file ${test_files})
  get_filename_component(test_name ${test_file} NAME_WE)
  add_argparse_test(${test_name})
endforeach()
