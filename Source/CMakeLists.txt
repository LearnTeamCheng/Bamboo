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


add_definitions(-DPROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

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

# 设置 Sandbox 的运行目录 (VS / CLion 有效)
set_target_properties(Sandbox PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
)
