cmake_minimum_required(VERSION 3.16)


enable_language(C)   # 解决 glad.c 编译器变量丢失的问题
enable_language(CXX) # 确保 C++ 也启用
# 工程名称和语言
project(BambooProject LANGUAGES CXX)

# C++ 设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 开启文件夹分组（IDE 美观）
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# # 添加 ThirdParty（glfw 必须有 CMakeLists.txt，entt/glm 是头文件）
# add_subdirectory(ThirdParty/GLFW)

# # 添加 Bamboo 子项目（此处路径是项目根目录下的 Bamboo 文件夹）
# add_subdirectory(Bamboo)

# # Bamboo 目标现在才存在，可以设置它的头文件路径
# target_include_directories(Bamboo PUBLIC
#     ${CMAKE_SOURCE_DIR}/Bamboo
#     ${CMAKE_SOURCE_DIR}/ThirdParty/entt
#     # ${CMAKE_SOURCE_DIR}/ThirdParty/glm
#     # ${CMAKE_SOURCE_DIR}/ThirdParty/spdlog/include
# )

# 包含 ThirdParty
add_subdirectory(ThirdParty)

# 添加引擎源代码
add_subdirectory(Bamboo)

# 添加 Sandbox 测试项目
add_subdirectory(Sandbox)
