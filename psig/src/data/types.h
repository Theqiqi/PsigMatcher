// data/types.h - 内部数据结构（数据层）
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace psig::data {

// 内存区域
struct Region {
    uint64_t base = 0;
    uint64_t size = 0;
    uint32_t protect = 0;
};

// 模块信息
struct ModuleInfo {
    std::wstring name;
    uint64_t base = 0;
    uint64_t size = 0;
};

// 进程快照条目（来自 Toolhelp PROCESSENTRY32W）
struct ProcessEntry {
    uint32_t pid = 0;
    std::wstring name;  // szExeFile，无需打开进程即可得
};

} // namespace psig::data
