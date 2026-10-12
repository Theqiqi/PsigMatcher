// api/process_scan.cpp - 接口层：进程枚举 / 图标 / 内存特征扫描（区域级并行）/ 取消
#include "psig.h"

#include <algorithm>
#include <atomic>
#include <iterator>
#include <thread>
#include <vector>

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

// 按 kChunkSize 分块扫描单个内存区域；命中追加到 out（不在此汇报进度）
void ScanRegion(void* h, const data::Region& region, uint32_t pid, const std::wstring& pname,
                const std::vector<data::ModuleInfo>& modules, const std::vector<Pattern>& patterns,
                size_t maxLen, size_t limit, const std::atomic_bool* cancel,
                std::vector<MatchResult>& out) {
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
    }
}

// 单进程扫描计划（规划阶段产出：句柄/名称/模块/可读区域）
struct Plan {
    uint32_t pid = 0;
    void* handle = nullptr; // OpenForRead 句柄；扫描结束统一关闭
    std::wstring name;
    std::vector<data::ModuleInfo> modules;
    std::vector<data::Region> regions;
};

// 并行任务：指向某个 Plan 的某个区域
struct RegionTask {
    size_t plan = 0;
    size_t region = 0;
};

// 规划阶段：逐进程打开句柄、枚举名称/模块/可读区域
std::vector<Plan> BuildPlans(const std::vector<uint32_t>& pids, const std::atomic_bool* cancel) {
    std::vector<Plan> plans;
    for (uint32_t pid : pids) {
        if (IsCancelled(cancel)) break;
        void* h = atomic::OpenForRead(pid);
        if (!h) continue;
        Plan plan;
        plan.pid = pid;
        plan.handle = h;
        std::wstring pname, ppath;
        atomic::ProcessNamePath(pid, pname, ppath);
        plan.name = pname;
        plan.modules = atomic::EnumModules(pid);
        plan.regions = atomic::EnumRegions(h);
        plans.push_back(std::move(plan));
    }
    return plans;
}

// 关闭全部计划句柄（成对释放）
void ClosePlans(std::vector<Plan>& plans) {
    for (auto& plan : plans) atomic::CloseProcessHandle(plan.handle);
    plans.clear();
}

// 展平为区域任务列表（total = 任务数 = 总工作量）
std::vector<RegionTask> FlattenTasks(const std::vector<Plan>& plans) {
    std::vector<RegionTask> tasks;
    for (size_t pi = 0; pi < plans.size(); ++pi) {
        for (size_t ri = 0; ri < plans[pi].regions.size(); ++ri) tasks.push_back({pi, ri});
    }
    return tasks;
}

// 并行扫描作业（各线程共享只读输入与原子计数，结果写各自的局部向量）
struct ScanJob {
    const std::vector<Plan>& plans;
    const std::vector<RegionTask>& tasks;
    const std::vector<Pattern>& patterns;
    size_t maxLen;
    size_t limit;
    const std::atomic_bool* cancel;
    const ProgressCallback& progress;
    size_t total;
    std::atomic<size_t>& next;
    std::atomic<size_t>& doneCount;
    std::atomic<size_t>& hitsCount;
    std::vector<std::vector<MatchResult>>& local;
};

// 工作线程：循环领取区域任务，扫描到局部结果，并汇报进度
void RunWorker(ScanJob& job, size_t tid) {
    std::vector<MatchResult>& out = job.local[tid];
    for (;;) {
        if (IsCancelled(job.cancel)) break;
        const size_t j = job.next.fetch_add(1);
        if (j >= job.total) break;
        const RegionTask& t = job.tasks[j];
        const Plan& plan = job.plans[t.plan];
        const data::Region& region = plan.regions[t.region];
        const size_t before = out.size();
        ScanRegion(plan.handle, region, plan.pid, plan.name, plan.modules, job.patterns, job.maxLen,
                   job.limit, job.cancel, out);
        const size_t added = out.size() - before;
        const size_t done = job.doneCount.fetch_add(1) + 1;
        const size_t hits = job.hitsCount.fetch_add(added) + added;
        if (job.progress) {
            ScanProgress sp;
            sp.currentPid = plan.pid;
            sp.done = done;
            sp.total = job.total;
            sp.hits = hits;
            job.progress(sp); // 可能被多线程并发调用（见公共头契约）
        }
    }
}

// 执行阶段：区域级并行（标准库线程 + 原子任务索引），返回合并后的结果
std::vector<MatchResult> RunParallel(const std::vector<Plan>& plans,
                                     const std::vector<RegionTask>& tasks,
                                     const std::vector<Pattern>& patterns, size_t maxLen,
                                     size_t limit, const std::atomic_bool* cancel,
                                     const ProgressCallback& progress) {
    const size_t total = tasks.size();
    size_t threads = std::thread::hardware_concurrency();
    if (threads == 0) threads = 1;
    if (threads > total) threads = total;

    std::atomic<size_t> next{0};
    std::atomic<size_t> doneCount{0};
    std::atomic<size_t> hitsCount{0};
    std::vector<std::vector<MatchResult>> local(threads);
    ScanJob job{plans, tasks, patterns, maxLen, limit, cancel, progress, total,
                next, doneCount, hitsCount, local};

    if (threads <= 1) {
        RunWorker(job, 0);
    } else {
        std::vector<std::thread> pool;
        pool.reserve(threads - 1);
        for (size_t i = 1; i < threads; ++i) {
            try {
                pool.emplace_back([&job, i]() { RunWorker(job, i); });
            } catch (...) {
                break; // 线程创建失败：以更少线程继续（主线程仍参与）
            }
        }
        RunWorker(job, 0); // 主线程也参与扫描
        for (auto& th : pool) th.join();
    }

    std::vector<MatchResult> results;
    for (auto& v : local) {
        results.insert(results.end(), std::make_move_iterator(v.begin()),
                       std::make_move_iterator(v.end()));
    }
    return results;
}

} // namespace

std::vector<ProcessInfo> EnumerateProcesses() {
    std::vector<ProcessInfo> result;
    for (const data::ProcessEntry& entry : atomic::EnumProcessEntries()) {
        ProcessInfo info;
        info.pid = entry.pid;
        std::wstring pathName, path;
        atomic::ProcessNamePath(entry.pid, pathName, path);
        // 名称优先取快照 szExeFile（受保护进程亦可得），映像名兜底
        info.name = entry.name.empty() ? pathName : entry.name;
        info.path = path;
        void* h = atomic::OpenForRead(entry.pid);
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
    std::vector<Pattern> patterns;
    for (const auto& p : req.patterns) {
        if (p.valid && !p.bytes.empty()) patterns.push_back(p);
    }
    size_t maxLen = 0;
    for (const auto& p : patterns) maxLen = (std::max)(maxLen, p.bytes.size());

    std::vector<Plan> plans = BuildPlans(req.pids, cancel);
    const std::vector<RegionTask> tasks = FlattenTasks(plans);
    if (tasks.empty()) {
        ClosePlans(plans);
        return {};
    }

    std::vector<MatchResult> results =
        RunParallel(plans, tasks, patterns, maxLen, req.maxResultsPerPattern, cancel, progress);
    ClosePlans(plans);
    return results;
}

void RequestCancel(bool flag) { g_cancel.store(flag); }

} // namespace psig
