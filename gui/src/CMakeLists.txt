# 生产目标：PsigMatcher 可执行文件
qt_add_executable(PsigMatcher
    main.cpp
    ${GUI_VIEW_SOURCES}
    ${GUI_MODEL_SOURCES}
    ${GUI_VM_SOURCES}
    ${CMAKE_SOURCE_DIR}/res/resources.qrc
)

target_include_directories(PsigMatcher PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../psig/include
)

target_link_libraries(PsigMatcher PRIVATE
    Qt6::Widgets
    Qt6::Concurrent
    ${PSIG_LIB}
)

# 静态 Qt（Release）：显式导入平台插件
if(PSIG_QT_STATIC)
    qt_import_plugins(PsigMatcher INCLUDE_BY_TYPE platforms Qt6::QWindowsIntegrationPlugin)
endif()

# Release 去掉控制台窗口（WIN32 子系统）；Debug 保留控制台便于调试
set_target_properties(PsigMatcher PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)
