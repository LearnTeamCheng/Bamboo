include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)


# add_executable(BambooTests
#     # TestMain.cpp

#     Core/RefTest.cpp
# )

# 自动查找 Tests 目录下所有测试源文件
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(BambooTests
    ${TEST_SOURCES}
)


target_link_libraries(BambooTests
    PRIVATE
        Bamboo
       # GTest::gtest
        GTest::gtest_main
)


target_compile_features(BambooTests
    PRIVATE
        cxx_std_20
)


include(GoogleTest)

gtest_discover_tests(BambooTests)