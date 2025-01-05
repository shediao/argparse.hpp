cmake_minimum_required(VERSION 3.30)
project(argparse)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

# 创建接口库target
add_library(argparse INTERFACE)
target_include_directories(argparse INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 启用测试
enable_testing()
add_subdirectory(test)

# 安装配置
install(TARGETS argparse
    EXPORT argparse-targets
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT argparse-targets
    FILE argparse-targets.cmake
    NAMESPACE argparse::
    DESTINATION lib/cmake/argparse
)
