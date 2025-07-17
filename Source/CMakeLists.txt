cmake_minimum_required(VERSION 3.16)
#设置工程名称 和 语言
project(Bamboo,LANGUAGE CXX)

#设置C++版本
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # 禁用编译器私有扩展，保持跨平台一致性

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


#添加引擎目录

#第三方库
add_subdirectory(ThirdParty/glfw)
add_subdirectory(ThirdParty/entt)
add_subdirectory(Bamboo)

