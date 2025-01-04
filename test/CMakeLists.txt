find_package(GTest REQUIRED)

# 通用函数，用于添加测试
function(add_argparser_test test_name)
    add_executable(${test_name} ${test_name}.cc)
    target_link_libraries(${test_name} PRIVATE 
        argparser
        GTest::gtest 
        GTest::gtest_main
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# 添加所有测试
add_argparser_test(argparser_fromstring_unittest)
add_argparser_test(argparser_unittest)
add_argparser_test(argparser_parse_unittest)
add_argparser_test(argparser_parse_gnu_coreutils_unittest) 