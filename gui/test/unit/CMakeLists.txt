# 单元测试：逐层测内部逻辑
find_package(GTest CONFIG REQUIRED)

set(GUI_TEST_INCLUDES
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/test
    ${CMAKE_SOURCE_DIR}/../psig/include
)

# View 层可单独编译测试（仅 View + Model，不链接 ViewModel/静态库）
add_executable(psig_test_view test_view.cpp ${GUI_VIEW_SOURCES} ${GUI_MODEL_SOURCES})
target_include_directories(psig_test_view PRIVATE ${GUI_TEST_INCLUDES})
target_link_libraries(psig_test_view PRIVATE Qt6::Widgets Qt6::Test GTest::gtest GTest::gtest_main)
if(PSIG_QT_STATIC)
    qt_import_plugins(psig_test_view INCLUDE_BY_TYPE platforms Qt6::QOffscreenIntegrationPlugin)
endif()
add_test(NAME psig_test_view COMMAND psig_test_view)

# Model 层
add_executable(psig_test_model test_model.cpp ${GUI_MODEL_SOURCES})
target_include_directories(psig_test_model PRIVATE ${GUI_TEST_INCLUDES})
target_link_libraries(psig_test_model PRIVATE Qt6::Widgets Qt6::Test GTest::gtest GTest::gtest_main)
if(PSIG_QT_STATIC)
    qt_import_plugins(psig_test_model INCLUDE_BY_TYPE platforms Qt6::QOffscreenIntegrationPlugin)
endif()
add_test(NAME psig_test_model COMMAND psig_test_model)

# ViewModel 层 + 错误弹窗（含事件链接、槽、静态库调用链）
add_executable(psig_test_viewmodel
    test_viewmodel.cpp
    test_errordialog.cpp
    ${GUI_VIEW_SOURCES}
    ${GUI_MODEL_SOURCES}
    ${GUI_VM_SOURCES}
)
target_include_directories(psig_test_viewmodel PRIVATE ${GUI_TEST_INCLUDES})
target_link_libraries(psig_test_viewmodel PRIVATE
    Qt6::Widgets Qt6::Concurrent Qt6::Test GTest::gtest GTest::gtest_main ${PSIG_LIB})
if(PSIG_QT_STATIC)
    qt_import_plugins(psig_test_viewmodel INCLUDE_BY_TYPE platforms Qt6::QOffscreenIntegrationPlugin)
endif()
add_test(NAME psig_test_viewmodel COMMAND psig_test_viewmodel)

# ViewModel 测试需验证语言切换：嵌入翻译资源（/i18n/*.qm）
gui_embed_i18n(psig_test_viewmodel test_i18n)

# 测试以 offscreen 平台运行：静态 Qt（Release）仅导入 QOffscreenIntegrationPlugin，
# 不设此环境变量时默认 windows 平台缺失会触发致命错误并阻塞
set_tests_properties(psig_test_view psig_test_model psig_test_viewmodel
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
