# 17 API 缺失需求（v0.3.0）

> 前置：`16_API充分性审查.md`
> 目标：逐项列出静态库无法满足的能力、期望公共 API、输入输出、异常要求、验收标准。

## 缺失能力清单

### M-1：进度语义修正（C-01）

- **期望公共 API**：`ScanProcesses(const ScanRequest&, ProgressCallback, const std::atomic_bool*)`（**签名不变**，`ScanProgress` 取值修正）。
- **行为**：
  - `ScanProgress::total` = 本次扫描总工作量（所有目标进程可读区域数之和；区域字节加权见静态库流程定稿）；
  - `ScanProgress::done` = 已完成工作量，随扫描**单调递增**，结束时 `done == total`；
  - `hits` / `currentPid` 语义不变。
- **输入**：无新增。
- **输出**：回调参数 `done`/`total` 反映真实进度。
- **异常要求**：不抛异常；无区域时 `total==0`。
- **验收标准**：单进程多区域扫描，进度值从 <100 单调增到 100；不再恒为 100。

### M-2：区域级并行扫描（C-02）

- **期望公共 API**：`ScanProcesses(...)`（**签名不变**，实现内部并行化）。
- **行为**：以内存区域为并行单位，多工作线程并发扫描；结果/进度/取消并发安全；并行度按 CPU 自适应；线程创建失败时回落串行。
- **契约变更（非签名）**：
  - 进度回调**可能被多个工作线程并发调用**（调用方须线程安全）；
  - 返回结果**顺序不保证**与串行一致（内容/数量一致）；
  - `maxResultsPerPattern` 仍按**逐模式**保证上限。
- **输入**：无新增。
- **输出**：`std::vector<MatchResult>`（内容/数量与串行一致）。
- **异常要求**：不抛异常；个别区域读取失败隔离在该区域。
- **验收标准**：多核机器同一任务并行版本耗时明显低于串行；命中集合与串行一致。

### M-3：进度回调粒度粗化（C-04）

- **期望公共 API**：`ProgressCallback`（**签名不变**，触发频率降低）。
- **行为**：不再按 1MiB 分块触发；改为按区域或更粗粒度触发。
- **输入**：无新增。
- **输出**：回调次数显著下降。
- **异常要求**：不抛异常。
- **验收标准**：大区域扫描回调次数由「与 1MiB 块数同量级」降为「与区域数同量级或更低」。

## 期望的 API 变更摘要（供静态库流程定稿）

```cpp
namespace psig {
// M-1：进度语义修正（签名不变，取值语义变化）
struct ScanProgress {
    uint32_t currentPid = 0;
    size_t done = 0;    // 已完成工作量（原：已处理模式数）
    size_t total = 0;   // 总工作量（原：总模式数）
    size_t hits = 0;
};

// M-2 + M-3：内部区域并行 + 回调粒度粗化（签名不变）
std::vector<MatchResult> ScanProcesses(const ScanRequest& req,
                                       ProgressCallback progress = nullptr,
                                       const std::atomic_bool* cancel = nullptr);
// 契约：progress 可能被多线程并发调用；结果顺序不保证；maxResultsPerPattern 逐模式保证。
}
```

## 约束满足说明

- 逐项列出缺失能力、期望公共 API、输入输出、异常要求、验收标准（M-1、M-2、M-3）。
- 静态库完成契约返回前，GUI 停止 ViewModel 层相关编码（不得用模拟数据/假接口）。
