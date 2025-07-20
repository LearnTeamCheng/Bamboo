project(Bamboo)

# 自动收集所有 cpp 和 h 文件（递归遍历 Bamboo 文件夹）
file(GLOB_RECURSE BAMBOO_SRC 
    CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

# 生成静态库
add_library(Bamboo STATIC ${BAMBOO_SRC})

# 公共头文件路径（允许外部项目 include <Bamboo/XXX.h>）
target_include_directories(Bamboo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 链接依赖库（注意：这些库应已在顶层用 add_subdirectory 或 find_package 添加）
target_link_libraries(Bamboo PUBLIC spdlog glfw entt glm)

# 设置 C++ 标准（保险起见再次设置）
set_target_properties(Bamboo PROPERTIES 
    CXX_STANDARD 17 
    CXX_STANDARD_REQUIRED YES 
    CXX_EXTENSIONS OFF
)
