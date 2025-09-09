

file(GLOB_RECURSE BAMBOO_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

add_library(Bamboo STATIC ${BAMBOO_SRC})

target_include_directories(Bamboo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Bamboo
    PUBLIC
        glfw
        glad
        entt
        spdlog::spdlog_header_only   
        stb
)

# 让 VS 按磁盘目录显示文件夹结构
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${BAMBOO_SRC})

if (MSVC)
    add_compile_options("/utf-8")
endif()
