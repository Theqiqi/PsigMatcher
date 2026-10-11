# 单元测试：算法层 + 数据层
find_package(GTest CONFIG REQUIRED)

add_executable(psig_unit
    test_pattern.cpp
    test_csv.cpp
    test_codec.cpp
)

target_include_directories(psig_unit PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(psig_unit PRIVATE psig GTest::gtest GTest::gtest_main)

add_test(NAME psig_unit COMMAND psig_unit)
