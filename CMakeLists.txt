cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(MyProject)

# 指定C++标准为C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 包含源文件目录
include_directories(src)

# 定义可执行文件的输出目录相对于构建目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加可执行文件 test 和 usage
add_executable(performance_test test/performance_test.cc)
add_executable(usage test/usage.cc)



