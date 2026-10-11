// api/process_scan.cpp - 接口层：进程枚举 / 图标 / 内存特征扫描 / 取消
#include "psig.h"

#include <algorithm>
#include <atomic>

#include "algorithm/module_locate.h"
#include "algorithm/pattern.h"
#include "atomic/win_icon.h"
#include "atomic/win_process.h"
#include "data/codec.h"
#include "data/types.h"

namespace psig {

namespace {

constexpr size_t kChunkSize = 1u << 20; // 1 MiB 分块读取

std::atomic_bool g_cancel{false};

bool IsCancelled(const std::atomic_bool* cancel) {
    return g_cancel.load() || (cancel && cancel->load());
}

IconBitmap ToPublicBitmap(atomic::RawBitmap&& raw) {
    IconBitmap out;
    out.width = raw.width;
    out.height = raw.height;
    out.pixels = std::move(raw.pixels);
    return out;
}

// 由命中位置构造一条结果（含模块归属与命中字节）
MatchResult MakeMatchResult(const std::vector<uint8_t>& buffer, uint64_t bufferBase, size_t pos,
                            const Pattern& p, uint32_t pid, const std::wstring& pname,
                            const std::vector<data::ModuleInfo>& modules) {
    MatchResult mr;
    mr.patternName = p.name;
    mr.pid = pid;
    mr.processName = pname;
    mr.address = bufferBase + pos;
    const int modIdx = algorithm::LocateModule(mr.address, modules);
    if (modIdx >= 0) {
        mr.moduleName = modules[modIdx].name;
        mr.moduleBase = modules[modIdx].base;
    }
    const std::vector<uint8_t> matched(buffer.begin() + static_cast<long>(pos),
                                       buffer.begin() + static_cast<long>(pos + p.bytes.size()));
    mr.matchedBytes = data::HexEncode(matched);
    mr.hitTime = data::NowIso8601();
    return mr;
}

// 在单个缓冲区内对所有模式做匹配，命中追加到 out
void ScanBuffer(const std::vector<uint8_t>& buffer, uint64_t bufferBase, uint32_t pid,
                const std::wstring& pname, const std::vector<data::ModuleInfo>& modules,
                const std::vector<Pattern>& patterns, size_t limit,
                const std::atomic_bool* cancel, std::vector<MatchResult>& out) {
    for (const auto& p : patterns) {
        if (IsCancelled(cancel)) break;
        const std::vector<size_t> hits =
            algorithm::FindAll(buffer.data(), buffer.size(), p.bytes, p.mask, limit);
        for (size_t pos : hits) {
            out.push_back(MakeMatchResult(buffer, bufferBase, pos, p, pid, pname, modules));
        }
    }
}

// 保留缓冲区尾部 overlap，保证跨块模式可命中
void UpdateCarry(const std::vector<uint8_t>& buffer, size_t maxLen, std::vector<uint8_t>& carry) {
    const size_t keep = (maxLen > 1) ? (maxLen - 1) : 0;
    if (keep > 0 && buffer.size() > keep) {
        carry.assign(buffer.end() - static_cast<long>(keep), buffer.end());
    } else {
        carry = buffer;
    }
}

// 按 kChunkSize 分块扫描单个内存区域
void ScanRegion(void* h, const data::Region& region, uint32_t pid, const std::wstring& pname,
                const std::vector<data::ModuleInfo>& modules, const std::vector<Pattern>& patterns,
                size_t maxLen, size_t limit, const std::atomic_bool* cancel,
                const ProgressCallback& progress, size_t total, std::vector<MatchResult>& out) {
    uint64_t offset = 0;
    std::vector<uint8_t> carry;
    while (offset < region.size) {
        if (IsCancelled(cancel)) break;
        const size_t want = static_cast<size_t>(
            (std::min)(static_cast<uint64_t>(kChunkSize), region.size - offset));
        const std::vector<uint8_t> chunk = atomic::ReadMemory(h, region.base + offset, want);
        if (chunk.empty()) {
            offset += want;
            carry.clear();
            continue;
        }
        std::vector<uint8_t> buffer;
        buffer.reserve(carry.size() + chunk.size());
        buffer.insert(buffer.end(), carry.begin(), carry.end());
        buffer.insert(buffer.end(), chunk.begin(), chunk.end());
        const uint64_t bufferBase = region.base + offset - carry.size();
        ScanBuffer(buffer, bufferBase, pid, pname, modules, patterns, limit, cancel, out);
        UpdateCarry(buffer, maxLen, carry);
        offset += want;
        if (progress) {
            ScanProgress sp;
            sp.currentPid = pid;
            sp.done = total;
            sp.total = total;
            sp.hits = out.size();
            progress(sp);
        }
    }
}

// 扫描单个进程的全部区域
void ScanOneProcess(uint32_t pid, const std::vector<Pattern>& patterns, size_t maxLen, size_t limit,
                    const std::atomic_bool* cancel, const ProgressCallback& progress,
                    std::vector<MatchResult>& out) {
    void* h = atomic::OpenForRead(pid);
    if (!h) return;
    std::wstring pname, ppath;
    atomic::ProcessNamePath(pid, pname, ppath);
    const std::vector<data::ModuleInfo> modules = atomic::EnumModules(pid);
    const std::vector<data::Region> regions = atomic::EnumRegions(h);
    const size_t total = patterns.size();
    for (const auto& region : regions) {
        if (IsCancelled(cancel)) break;
        ScanRegion(h, region, pid, pname, modules, patterns, maxLen, limit, cancel, progress, total,
                   out);
    }
    atomic::CloseProcessHandle(h);
}

} // namespace

std::vector<ProcessInfo> EnumerateProcesses() {
    std::vector<ProcessInfo> result;
    for (uint32_t pid : atomic::EnumProcessIds()) {
        ProcessInfo info;
        info.pid = pid;
        std::wstring name, path;
        atomic::ProcessNamePath(pid, name, path);
        info.name = name;
        info.path = path;
        void* h = atomic::OpenForRead(pid);
        info.accessible = (h != nullptr);
        atomic::CloseProcessHandle(h);
        result.push_back(std::move(info));
    }
    std::sort(result.begin(), result.end(),
              [](const ProcessInfo& a, const ProcessInfo& b) { return a.pid < b.pid; });
    return result;
}

IconBitmap ProcessIcon(uint32_t pid) {
    return ToPublicBitmap(atomic::ExtractProcessIcon(pid));
}

IconBitmap DefaultProcessIcon() {
    return ToPublicBitmap(atomic::ExtractDefaultIcon());
}

std::vector<MatchResult> ScanProcesses(const ScanRequest& req, ProgressCallback progress,
                                       const std::atomic_bool* cancel) {
    std::vector<MatchResult> results;
    std::vector<Pattern> patterns;
    for (const auto& p : req.patterns) {
        if (p.valid && !p.bytes.empty()) patterns.push_back(p);
    }
    size_t maxLen = 0;
    for (const auto& p : patterns) maxLen = (std::max)(maxLen, p.bytes.size());

    for (uint32_t pid : req.pids) {
        if (IsCancelled(cancel)) break;
        ScanOneProcess(pid, patterns, maxLen, req.maxResultsPerPattern, cancel, progress, results);
    }
    return results;
}

void RequestCancel(bool flag) { g_cancel.store(flag); }

} // namespace psig
