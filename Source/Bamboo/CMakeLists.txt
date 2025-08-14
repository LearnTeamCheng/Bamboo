# file(GLOB_RECURSE BAMBOO_SRC
#     ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/*.h
# )

# add_library(Bamboo STATIC ${BAMBOO_SRC})

# target_include_directories(Bamboo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# target_link_libraries(Bamboo
#     PUBLIC
#         glfw
#         glad
#         entt
# )

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
)