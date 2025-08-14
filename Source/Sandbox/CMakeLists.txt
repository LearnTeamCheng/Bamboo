file(GLOB_RECURSE SANDBOX_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

add_executable(Sandbox ${SANDBOX_SRC})

target_include_directories(Sandbox PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Sandbox PRIVATE Bamboo)