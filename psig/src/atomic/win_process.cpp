// atomic/win_process.cpp - 原子化底层实现（Windows API）
#include "atomic/win_process.h"

#include <windows.h>
#include <tlhelp32.h>

namespace psig::atomic {

namespace {
bool IsReadableProtect(DWORD protect) {
    if (protect & PAGE_GUARD) return false;
    if (protect & PAGE_NOACCESS) return false;
    const DWORD base = protect & 0xFF;
    return base == PAGE_READONLY || base == PAGE_READWRITE ||
           base == PAGE_EXECUTE_READ || base == PAGE_EXECUTE_READWRITE ||
           base == PAGE_WRITECOPY || base == PAGE_EXECUTE_WRITECOPY;
}
} // namespace

std::vector<data::ProcessEntry> EnumProcessEntries() {
    std::vector<data::ProcessEntry> entries;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return entries;
    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    if (Process32FirstW(snap, &pe)) {
        do {
            data::ProcessEntry e;
            e.pid = pe.th32ProcessID;
            e.name = pe.szExeFile;
            entries.push_back(std::move(e));
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return entries;
}

void* OpenForRead(uint32_t pid) {
    HANDLE h = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (h == nullptr) {
        // 退而求其次：仅查询权限（用于取名字）
        h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    }
    return h;
}

void CloseProcessHandle(void* handle) {
    if (handle) CloseHandle(static_cast<HANDLE>(handle));
}

bool ProcessNamePath(uint32_t pid, std::wstring& name, std::wstring& path) {
    name.clear();
    path.clear();
    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!h) return false;
    wchar_t buf[MAX_PATH * 2];
    DWORD size = static_cast<DWORD>(std::size(buf));
    if (QueryFullProcessImageNameW(h, 0, buf, &size)) {
        path.assign(buf, size);
        size_t slash = path.find_last_of(L"\\/");
        name = (slash == std::wstring::npos) ? path : path.substr(slash + 1);
    }
    CloseHandle(h);
    return !name.empty();
}

std::vector<data::Region> EnumRegions(void* handle) {
    std::vector<data::Region> regions;
    if (!handle) return regions;
    HANDLE h = static_cast<HANDLE>(handle);
    uint64_t addr = 0;
    MEMORY_BASIC_INFORMATION mbi{};
    while (VirtualQueryEx(h, reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi)) == sizeof(mbi)) {
        if (mbi.State == MEM_COMMIT && IsReadableProtect(mbi.Protect)) {
            data::Region r;
            r.base = reinterpret_cast<uint64_t>(mbi.BaseAddress);
            r.size = static_cast<uint64_t>(mbi.RegionSize);
            r.protect = mbi.Protect;
            regions.push_back(r);
        }
        uint64_t next = reinterpret_cast<uint64_t>(mbi.BaseAddress) + mbi.RegionSize;
        if (next <= addr) break; // 防溢出/死循环
        addr = next;
    }
    return regions;
}

std::vector<uint8_t> ReadMemory(void* handle, uint64_t base, size_t size) {
    std::vector<uint8_t> buf;
    if (!handle || size == 0) return buf;
    buf.resize(size);
    SIZE_T got = 0;
    if (ReadProcessMemory(static_cast<HANDLE>(handle),
                          reinterpret_cast<LPCVOID>(base), buf.data(), size, &got)) {
        buf.resize(got);
        return buf;
    }
    buf.clear();
    return buf;
}

std::vector<data::ModuleInfo> EnumModules(uint32_t pid) {
    std::vector<data::ModuleInfo> modules;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE) return modules;
    MODULEENTRY32W me{};
    me.dwSize = sizeof(me);
    if (Module32FirstW(snap, &me)) {
        do {
            data::ModuleInfo mi;
            mi.name = me.szModule;
            mi.base = reinterpret_cast<uint64_t>(me.modBaseAddr);
            mi.size = me.modBaseSize;
            modules.push_back(mi);
        } while (Module32NextW(snap, &me));
    }
    CloseHandle(snap);
    return modules;
}

} // namespace psig::atomic
