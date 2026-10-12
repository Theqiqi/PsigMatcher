cmake_minimum_required(VERSION 3.20)
project(PsigMatcher LANGUAGES CXX)

# 图标资源（PE 资源节）需要 MSVC 资源编译器 rc.exe，故启用 RC 语言；
# 非 MSVC 工具链在上方 FATAL_ERROR 处已拦截。
enable_language(RC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT MSVC)
    message(FATAL_ERROR "PsigMatcher 仅支持 MSVC (cl.exe) 工具链")
endif()

# Qt6（Debug 共享 / Release 静态），经 CMAKE_PREFIX_PATH 定位
find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent Test LinguistTools)

# 判定静态 Qt
get_target_property(_qt_core_type Qt6::Core TYPE)
if(_qt_core_type STREQUAL "STATIC_LIBRARY")
    set(PSIG_QT_STATIC ON)
    message(STATUS "PsigMatcher: 静态 Qt 构建（Release /MT）")
else()
    set(PSIG_QT_STATIC OFF)
    message(STATUS "PsigMatcher: 共享 Qt 构建（Debug /MDd）")
endif()

# 跨项目引用 psig 静态库（直接指向拥有方构建输出）
find_library(PSIG_LIB NAMES psig
    PATHS ${CMAKE_SOURCE_DIR}/../psig/out/lib/${CMAKE_BUILD_TYPE}
    NO_DEFAULT_PATH)
if(NOT PSIG_LIB)
    message(FATAL_ERROR "未找到 psig.lib：请先编译 psig（../psig/out/lib/${CMAKE_BUILD_TYPE}/）")
endif()
message(STATUS "PsigMatcher: PSIG_LIB=${PSIG_LIB}")

# 共享 Qt 运行期：将 Qt bin 加入 PATH（qt.conf/插件由 Qt 自动定位）
if(NOT PSIG_QT_STATIC)
    get_target_property(_qt_core_loc Qt6::Core IMPORTED_LOCATION_RELEASE)
    if(NOT _qt_core_loc)
        get_target_property(_qt_core_loc Qt6::Core IMPORTED_LOCATION_DEBUG)
    endif()
    if(_qt_core_loc)
        get_filename_component(_qt_bin_dir "${_qt_core_loc}" DIRECTORY)
        set(PSIG_QT_BIN_DIR "${_qt_bin_dir}" CACHE INTERNAL "Qt bin dir")
    endif()
endif()

# 生产/测试源文件清单（供各子项目复用）
set(GUI_VIEW_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/mainwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/signatureeditorpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/processselectionpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/resultpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/columnmanagerdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/aboutdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/exportcolumnsdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/hoverrowdelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/hoverrowtableview.cpp
)
set(GUI_MODEL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/signaturetablemodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/processtablemodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/resulttablemodel.cpp
)
set(GUI_VM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/viewmodel/viewmodel_helpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/viewmodel/mainviewmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/viewmodel/bind.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/viewmodel/process.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/viewmodel/signature.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/viewmodel/scan.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/viewmodel/result.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/viewmodel/language.cpp
)

# 生产 exe 的图标资源：res/app.rc 引用同目录的 app.ico（由 gen_app_icon.py 生成）。
# INCLUDE_DIRECTORIES 让 rc.exe 能按 "app.ico" 解析到 res/ 目录（不依赖构建时的当前目录）。
set(GUI_ICON_RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/res/app.rc
)
set_source_files_properties(${GUI_ICON_RESOURCES}
    PROPERTIES INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/res)

enable_testing()

# i18n：由 .ts 生成 .qm（lrelease），并以资源前缀 /i18n 嵌入目标，供 QTranslator 运行期装载。
# 采用自定义命令而非 qt_add_translations：可对生产与测试目标复用同一批 .qm，且构建确定。
set(GUI_TS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/res/i18n/PsigMatcher_zh_CN.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/res/i18n/PsigMatcher_en_US.ts
)
set(GUI_QM_FILES)
foreach(_ts IN LISTS GUI_TS_FILES)
    get_filename_component(_base "${_ts}" NAME_WE)
    set(_qm "${CMAKE_CURRENT_BINARY_DIR}/i18n/${_base}.qm")
    add_custom_command(OUTPUT "${_qm}"
        COMMAND Qt6::lrelease "${_ts}" -qm "${_qm}"
        DEPENDS "${_ts}"
        COMMENT "lrelease ${_base}.qm"
        VERBATIM)
    list(APPEND GUI_QM_FILES "${_qm}")
endforeach()

# 供子目录（测试目标）复用
set(GUI_QM_FILES "${GUI_QM_FILES}" CACHE INTERNAL "Generated .qm files")

# 将翻译资源嵌入目标（资源路径 /i18n/<name>.qm）；别名在调用目录作用域设置。
function(gui_embed_i18n target resname)
    foreach(_qm IN LISTS GUI_QM_FILES)
        get_filename_component(_b "${_qm}" NAME)
        set_source_files_properties("${_qm}" PROPERTIES QT_RESOURCE_ALIAS "${_b}")
    endforeach()
    qt_add_resources(${target} ${resname} PREFIX "/i18n" FILES ${GUI_QM_FILES})
endfunction()

add_subdirectory(src)
add_subdirectory(test/target)
add_subdirectory(test/unit)
add_subdirectory(test/integration)

# 生产目标嵌入翻译资源
gui_embed_i18n(PsigMatcher PsigMatcher_i18n)
