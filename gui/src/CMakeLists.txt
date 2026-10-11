# 生产目标：psig_gui 可执行文件
qt_add_executable(psig_gui
    main.cpp
    ${GUI_VIEW_SOURCES}
    ${GUI_MODEL_SOURCES}
    ${GUI_VM_SOURCES}
    ${CMAKE_SOURCE_DIR}/res/resources.qrc
)

target_include_directories(psig_gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../psig/include
)

target_link_libraries(psig_gui PRIVATE
    Qt6::Widgets
    Qt6::Concurrent
    ${PSIG_LIB}
)

# 静态 Qt（Release）：显式导入平台插件
if(PSIG_QT_STATIC)
    qt_import_plugins(psig_gui INCLUDE_BY_TYPE platforms Qt6::QWindowsIntegrationPlugin)
endif()
