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
    atomic/win_process.cpp
    atomic/file_io.cpp
    atomic/sqlite_io.cpp
    api/psig.cpp
    ${PSIG_SQLITE_SRC}
)

target_include_directories(psig
    PUBLIC ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/third_party/sqlite3
)

# 无外部链接依赖：SQLite 合并源已编入 psig，GUI 只需链接 psig 与 Qt
target_compile_definitions(psig PRIVATE
    UNICODE _UNICODE NOMINMAX
    SQLITE_THREADSAFE=1
    SQLITE_OMIT_LOAD_EXTENSION
)
