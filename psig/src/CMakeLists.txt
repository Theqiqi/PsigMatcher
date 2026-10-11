# 生产目标：psig 静态库
find_package(unofficial-sqlite3 CONFIG REQUIRED)

add_library(psig STATIC
    data/codec.cpp
    algorithm/pattern.cpp
    algorithm/module_locate.cpp
    algorithm/csv.cpp
    atomic/win_process.cpp
    atomic/file_io.cpp
    atomic/sqlite_io.cpp
    api/psig.cpp
)

target_include_directories(psig
    PUBLIC ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(psig PUBLIC unofficial::sqlite3::sqlite3)

target_compile_definitions(psig PRIVATE UNICODE _UNICODE NOMINMAX)
