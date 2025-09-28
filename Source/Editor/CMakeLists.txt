# ========================
# Editor CMakeLists
# ========================

# 分组：核心逻辑
set(EDITOR_CORE
    Source/Core/Application.cpp
    Source/Core/Application.h

    # src/Core/EditorLayer.cpp
    # src/Core/EditorLayer.h
    # src/Core/EditorApplication.cpp
    # src/Core/EditorApplication.h
)

# 分组：面板
set(EDITOR_PANELS
    # src/Panels/SceneHierarchyPanel.cpp
    # src/Panels/SceneHierarchyPanel.h
    # src/Panels/ContentBrowserPanel.cpp
    # src/Panels/ContentBrowserPanel.h
)

# 分组：UI/工具类
set(EDITOR_UI
    # src/UI/EditorUI.cpp
    # src/UI/EditorUI.h
)

# 主入口
set(EDITOR_SRC
    Source/main.cpp
    ${EDITOR_CORE}
    ${EDITOR_PANELS}
    ${EDITOR_UI}
)

# 可执行文件
add_executable(Editor ${EDITOR_SRC})

# 包含路径
target_include_directories(Editor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)



target_link_libraries(Editor PRIVATE 
    Bamboo 
    imgui 
    spdlog::spdlog_header_only 
    stb
    glfw
    )

# 如果想分组显示在 IDE（比如 VS / Xcode）里更清晰
source_group("Core" FILES ${EDITOR_CORE})
source_group("Panels" FILES ${EDITOR_PANELS})
source_group("UI" FILES ${EDITOR_UI})


# 让 VS 按磁盘目录显示文件夹结构
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${BAMBOO_SRC})

if (MSVC)
    add_compile_options("/utf-8")
endif()