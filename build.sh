#!/bin/bash

# 确保脚本错误时停止执行
set -e

# 定义构建目录
BUILD_DIR=build

# 定义可执行文件输出目录
BIN_DIR=$BUILD_DIR/bin

# 检查并创建构建目录
if [ ! -d "$BUILD_DIR" ]; then
  mkdir "$BUILD_DIR"
fi

# 检查并创建bin目录
if [ ! -d "$BIN_DIR" ]; then
  mkdir "$BIN_DIR"
fi

# 切换到构建目录
cd "$BUILD_DIR"

# 运行CMake生成构建系统
cmake ..

# 编译项目
make

echo "Build completed successfully. Executables are in $BIN_DIR."