file(GLOB_RECURSE BAMBOO_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

add_library(Bamboo STATIC ${BAMBOO_SRC})

target_include_directories(Bamboo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 链接 ThirdParty
target_link_libraries(Bamboo
    PUBLIC
        glfw
        glad
        entt
        # spdlog (header-only 时直接 include)
)

# 如果需要跨平台宏
# if(WIN32)
#     target_compile_definitions(Bamboo PRIVATE BAMBOO_PLATFORM_WINDOWS)
# elseif(APPLE)
#     target_compile_definitions(Bamboo PRIVATE BAMBOO_PLATFORM_MAC)
# elseif(UNIX)
#     target_compile_definitions(Bamboo PRIVATE BAMBOO_PLATFORM_LINUX)
# endif()