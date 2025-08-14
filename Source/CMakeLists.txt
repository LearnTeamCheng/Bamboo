cmake_minimum_required(VERSION 3.20)
project(BambooEngine)

enable_language(C)   # glad.c 用 C 编译
enable_language(CXX) # 引擎用 C++

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 引入 ThirdParty（指定生成路径）
add_subdirectory(./ThirdParty ${CMAKE_BINARY_DIR}/ThirdParty_build)

# 引擎核心
add_subdirectory(Bamboo)

# Sandbox 测试项目
add_subdirectory(Sandbox)