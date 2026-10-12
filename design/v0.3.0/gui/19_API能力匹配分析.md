# 19 API 能力匹配分析（静态库完成回退检查，v0.3.0）

> 前置：`design/v0.3.0/gproto/77_静态库完成契约.md`、`17_API缺失需求.md`
> 本步：确认静态库流程完成、公共接口交还 GUI，重新比较能力与公共 API。

## 1. 契约完整性检查

| 契约要素 | 要求 | 实际 | 结论 |
| --- | --- | --- | --- |
| 公共头文件 | 存在 | `psig/include/psig.h`（v0.3.0） | ✅ |
| API 列表 | 完整 | `ScanProcesses`（实现改进）、`ScanProgress`（语义修正）、`ProgressCallback`（并发契约） | ✅ |
| 库产物 | `out/lib/<配置>` | `psig/out/lib/Debug/psig.lib`、`psig/out/lib/Release/psig.lib` | ✅ |
| 能力覆盖结果 | 逐项 | 见下 | ✅ |

## 2. 能力再匹配

| 编号 | 能力 | 公共 API | 覆盖 |
| --- | --- | --- | --- |
| R-01 | 进度语义（C-01） | `ScanProcesses` + `ScanProgress{done,total}`（真实工作量） | ✅ |
| R-02 | 区域并行（C-02） | `ScanProcesses`（内部区域级并行，签名不变） | ✅ |
| R-03 | 回调粒度（C-04） | `ProgressCallback`（每区域触发） | ✅ |
| 兼容 | 签名不变 / 结果内容不变 | 结果顺序可变、内容/数量一致 | ✅ |

**全部覆盖** → 继续 GUI 设计（步骤 22）。

## 3. 约束确认

- 契约要素齐全，无需返回静态库流程。
- 无未覆盖能力，无需更新缺失需求文档。
- GUI 现在可以开始 ViewModel 层对新增/修正能力的调用编码（此前已按约束停止）：
  - C-01：`MainViewModel::onProgressTick` 读取进度原子变量并显示真实进度。
  - C-03/C-04：`onStartScan` 的进度回调只写原子变量；`QTimer` 定时采样刷新界面。
- 静态库不引用 Qt；GUI 仅经公共 API 调用静态库；**并行在 psig 内部实现，GUI 生产代码不建线程池、不调用平台 API**。

## 4. 交接

- 回退到 GUI 流程步骤 22「模块设计」，进入编码前的设计阶段。
