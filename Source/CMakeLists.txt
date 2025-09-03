cmake_minimum_required(VERSION 3.20)
project(BambooEngine)

enable_language(C)
enable_language(CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ThirdParty
add_subdirectory(./ThirdParty ${CMAKE_BINARY_DIR}/ThirdParty_build)

# 引擎
add_subdirectory(Bamboo)

# Sandbox
add_subdirectory(Sandbox)


# add_definitions(-DPROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# # ----------- 资源拷贝 -----------
# add_custom_target(copy_engine_assets ALL
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/BambooAssets
#     ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/BambooAssets
# )

# add_custom_target(copy_sandbox_assets ALL
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/Sandbox/Assets
#     ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/SandboxAssets
# )
# 规范化路径
file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}/BambooAssets" BAMBOO_ASSET_ROOT_ABS)
# 生成头文件
configure_file(
  ${CMAKE_SOURCE_DIR}/Bamboo/Config.in.h
  ${CMAKE_BINARY_DIR}/generated/Config.h
  @ONLY
)
# 让目标能包含到这个生成头
target_include_directories(Bamboo  PUBLIC ${CMAKE_BINARY_DIR}/generated)
target_include_directories(Sandbox PUBLIC ${CMAKE_BINARY_DIR}/generated)



# 设置 Sandbox 的运行目录 (VS / CLion 有效)
set_target_properties(Sandbox PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
)
