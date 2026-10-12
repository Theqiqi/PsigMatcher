# 25 静态库 API 调用映射（v0.3.0，增量）

> 前置：`24_ViewModel接口.md`、`design/v0.3.0/gproto/77_静态库完成契约.md`

## 映射表（含本次扩展）

| VM 行为 | psig 公共 API | 说明 |
| --- | --- | --- |
| 刷新进程 | `EnumerateProcesses()` | 沿用（v0.2.4） |
| 进程图标 | `ProcessIcon(pid)` / `DefaultProcessIcon()` | 沿用 |
| 开始扫描 | `ParsePattern` + `ScanProcesses(req, cb, cancel)` | **实现改进**：内部区域并行；回调按区域触发且 `done/total` 真实（C-01/C-02/C-04） |
| 扫描进度 | `ScanProcesses` 的 `ProgressCallback` | **契约变更**：回调可能被多线程并发调用；VM 回调仅写原子变量 |
| 取消扫描 | `RequestCancel` / cancel 标志 | 沿用 |
| 其余（列/导入导出等） | `BuiltinSignatureColumns` / `ResultColumnDefs` / `SelectResultKeys` / `Import*` / `Export*` | 沿用 |

## 转换（VM 私有）

- 进度换算（C-01）：`pct = total ? static_cast<int>(done * 100 / total) : 0`（`done/total` 为 psig 提供的区域工作量）。
- 进度回调（C-04）：`cb` 仅写 `m_progDone/Total/Hits/Pid` 原子变量；不 `invokeMethod`、不触碰界面。
- 采样刷新（C-03）：`onProgressTick` 由 `QTimer` 驱动，读原子变量后经 `m_view->setProgress`/`setTransientMessage` 刷新。

## 约束满足说明

- VM 每个业务方法对应真实 psig 公共 API；并行与进度由 psig 提供，GUI 不调用 Win32、不建线程池、不自行实现缺失能力。
- 无新增公共 API（签名不变），仅消费 psig 修正后的行为。
