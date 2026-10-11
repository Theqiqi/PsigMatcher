// atomic/win_process.h - 原子化底层：Windows 进程/内存原语
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "data/types.h"

namespace psig::atomic {

// 枚举系统进程 id
std::vector<uint32_t> EnumProcessIds();

// 以读权限打开进程；失败返回 nullptr
void* OpenForRead(uint32_t pid);

// 关闭进程句柄
void CloseProcessHandle(void* handle);

// 取进程名与可执行路径
bool ProcessNamePath(uint32_t pid, std::wstring& name, std::wstring& path);

// 枚举可读（MEM_COMMIT + 可读保护）内存区域
std::vector<data::Region> EnumRegions(void* handle);

// 读取内存块；失败返回空
std::vector<uint8_t> ReadMemory(void* handle, uint64_t base, size_t size);

// 枚举进程模块
std::vector<data::ModuleInfo> EnumModules(uint32_t pid);

} // namespace psig::atomic
