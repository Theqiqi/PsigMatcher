cmake_minimum_required(VERSION 3.20)
project(psig LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT MSVC)
    message(FATAL_ERROR "psig 仅支持 MSVC (cl.exe) 工具链")
endif()

# 输出目录由 CMakePresets.json 各预设的 cacheVariables 声明，此处不硬编码。

enable_testing()

add_subdirectory(src)
add_subdirectory(test/target)
add_subdirectory(test/unit)
add_subdirectory(test/integration)
