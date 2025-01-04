cmake_minimum_required(VERSION 3.30)
project(argparser)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

# 创建接口库target
add_library(argparser INTERFACE)
target_include_directories(argparser INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 启用测试
enable_testing()
add_subdirectory(test)

# 安装配置
install(TARGETS argparser
    EXPORT argparser-targets
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT argparser-targets
    FILE argparser-targets.cmake
    NAMESPACE argparser::
    DESTINATION lib/cmake/argparser
)
