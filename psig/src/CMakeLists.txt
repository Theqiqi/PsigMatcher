# 生产目标：psig 静态库（自包含，内置 SQLite 合并源，不向消费方泄漏三方依赖）
set(PSIG_SQLITE_SRC "${CMAKE_SOURCE_DIR}/third_party/sqlite3/sqlite3.c")
if(EXISTS "${PSIG_SQLITE_SRC}")
    message(STATUS "psig: SQLITE_SRC found: ${PSIG_SQLITE_SRC}")
else()
    message(WARNING "psig: SQLITE_SRC NOT found: ${PSIG_SQLITE_SRC}")
endif()
add_library(psig STATIC
    data/codec.cpp
    algorithm/pattern.cpp
    algorithm/module_locate.cpp
    algorithm/csv.cpp
    algorithm/result_columns.cpp
    atomic/win_process.cpp
    atomic/win_icon.cpp
    atomic/file_io.cpp
    atomic/sqlite_io.cpp
    api/process_scan.cpp
    api/pattern_api.cpp
    api/exchange_signatures.cpp
    api/exchange_results.cpp
    ${PSIG_SQLITE_SRC}
)

target_include_directories(psig
    PUBLIC ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/third_party/sqlite3
)

# 系统库依赖（图标提取）：PUBLIC 传播给消费方（GUI 经 psig 传递链接）
target_link_libraries(psig PUBLIC shell32 user32 gdi32)

# SQLite 合并源已编入 psig，GUI 只需链接 psig 与 Qt
target_compile_definitions(psig PRIVATE
    UNICODE _UNICODE NOMINMAX
    SQLITE_THREADSAFE=1
    SQLITE_OMIT_LOAD_EXTENSION
)
