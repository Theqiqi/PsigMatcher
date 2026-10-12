# 17 原子化 api 分析（psig v0.3.0）

> 前置：`12_接口细化设计.md`

## 结论：本轮无新增/修改原子化 api
- 扫描并行化复用既有原子原语，不新增平台能力：
  - `void* OpenForRead(uint32_t pid)` —— 规划阶段打开（失败跳过）。
  - `void CloseProcessHandle(void* handle)` —— 扫描结束关闭（成对）。
  - `std::vector<data::Region> EnumRegions(void* handle)` —— 提供并行任务（可读区域）。
  - `std::vector<data::ModuleInfo> EnumModules(uint32_t pid)` —— 命中模块归属（沿用）。
  - `std::vector<uint8_t> ReadMemory(void* handle, uint64_t base, size_t size)` —— 区域分块读取。

## 并发安全说明
- `ReadProcessMemory` 对同一进程句柄的**并发读**是安全的（只读、无共享可变状态）；多线程共享同一 `handle` 并发读取无数据竞争。
- 各线程仅写**自己的**局部结果向量；跨线程共享的只有原子计数（`done`/`hits`/`next`）。

## 原子化 api 列表（受影响）
| 原子 api | 变更 |
| --- | --- |
| `OpenForRead` / `CloseProcessHandle` | 沿用（角色：规划阶段打开、结束关闭） |
| `EnumRegions` / `EnumModules` / `ReadMemory` | 沿用（并发读安全） |
