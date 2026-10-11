# 集成测试：串联多个 api 功能函数（真实场景）
find_package(GTest CONFIG REQUIRED)

add_executable(psig_integration test_scan.cpp)

target_link_libraries(psig_integration PRIVATE psig GTest::gtest GTest::gtest_main)

# 提供测试目标程序路径（跨进程扫描）
target_compile_definitions(psig_integration PRIVATE
    PSIG_TARGET_PATH="$<TARGET_FILE:psig_target>")

add_test(NAME psig_integration COMMAND psig_integration)
