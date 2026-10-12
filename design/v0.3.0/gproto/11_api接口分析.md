# 11 api 接口分析（psig v0.3.0）

> 前置：`08_功能设计.md`

## 接口细节

### `std::vector<MatchResult> ScanProcesses(const ScanRequest&, ProgressCallback, const std::atomic_bool*);`
- 语义：对 `req.pids × req.patterns` 做内存特征匹配。
- 本次：内部新增规划阶段；区域级并行扫描；进度真实化；回调按区域触发。**签名不变**。
- 并发契约（新增）：`progress` 回调**可能被多个工作线程并发调用**（调用方须线程安全）；返回结果**顺序不保证**。
- 异常：不抛异常；个别区域读取失败隔离在该区域。

### `struct ScanProgress`
- `done`：已完成工作量（区域数）。`total`：总工作量（区域数）。
- 语义修正：原注释「已处理模式数/总模式数」→「已完成工作量/总工作量」。字段类型不变。

### `using ProgressCallback`
- 契约补充：可能被多线程并发调用；调用方仅做轻量、线程安全操作（如写原子变量）。
