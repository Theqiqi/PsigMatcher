# 集成测试：走通完整 MVVM 链路（View信号→VM槽→静态库API→Model→View刷新）
find_package(GTest CONFIG REQUIRED)

set(GUI_TEST_INCLUDES
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/test
    ${CMAKE_SOURCE_DIR}/../psig/include
)

add_executable(psig_test_mvvm
    test_mvvm_flow.cpp
    ${GUI_VIEW_SOURCES}
    ${GUI_MODEL_SOURCES}
    ${GUI_VM_SOURCES}
)
target_include_directories(psig_test_mvvm PRIVATE ${GUI_TEST_INCLUDES})
target_link_libraries(psig_test_mvvm PRIVATE
    Qt6::Widgets Qt6::Concurrent Qt6::Test GTest::gtest GTest::gtest_main ${PSIG_LIB})
if(PSIG_QT_STATIC)
    qt_import_plugins(psig_test_mvvm INCLUDE_BY_TYPE platforms Qt6::QOffscreenIntegrationPlugin)
endif()
add_test(NAME psig_test_mvvm COMMAND psig_test_mvvm)

add_executable(psig_test_userflow
    test_user_flow.cpp
    ${GUI_VIEW_SOURCES}
    ${GUI_MODEL_SOURCES}
    ${GUI_VM_SOURCES}
)
target_include_directories(psig_test_userflow PRIVATE ${GUI_TEST_INCLUDES})
target_link_libraries(psig_test_userflow PRIVATE
    Qt6::Widgets Qt6::Concurrent Qt6::Test GTest::gtest GTest::gtest_main ${PSIG_LIB})
if(PSIG_QT_STATIC)
    qt_import_plugins(psig_test_userflow INCLUDE_BY_TYPE platforms Qt6::QOffscreenIntegrationPlugin)
endif()
add_test(NAME psig_test_userflow COMMAND psig_test_userflow)

# 测试以 offscreen 平台运行（静态 Qt Release 仅导入 QOffscreenIntegrationPlugin）
set_tests_properties(psig_test_mvvm psig_test_userflow
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
