# 19 API 能力匹配分析（v0.3.1，静态库完成回退检查）

> 前置：静态库完成契约 `design/v0.3.1/gproto/77_静态库完成契约.md`（公共头、API 列表、`out/lib/<配置>` 产物、能力覆盖结果）。

## 一、契约完整性检查

| 契约项 | 状态 | 证据 |
| --- | --- | --- |
| 公共头文件 | ✅ | `psig/include/psig.h`（`Table::targetProcess`，`:80`；版本 v0.3.1） |
| API 列表 | ✅ | `ExportSignaturesDb`/`ImportSignaturesDb`（签名不变，行为增强）；无新增函数 |
| 库产物 | ✅ | `psig/out/lib/Debug/psig.lib`（Release 见编译阶段） |
| 能力覆盖结果 | ✅ | R-01 覆盖；用例 `SignaturesExchange.*` 4 例全绿 |

## 二、能力 ↔ 公共 API 重新匹配

| 改动点 | 能力 | 公共 API | 覆盖 |
| --- | --- | --- | --- |
| C-01~C-04 | 无静态库能力需求 | — | 不适用 |
| C-05 | 特征码库携带/读回目标进程名 | `psig::Table::targetProcess` + `ExportSignaturesDb`/`ImportSignaturesDb` | ✅ 全覆盖 |

## 三、结论

- 全部能力已被 `psig` 公共 API 覆盖，无未覆盖能力。
- 契约齐全 → 继续 GUI 设计（步骤 22）。
- GUI 可恢复 C-05 的 ViewModel 层编码（`importSignaturesFromPath` / `exportSignaturesToPath`）。

## 约束满足说明

- 契约缺少任一要件 → 返回静态库流程；本版契约齐全，无需回退。
- 匹配完成后方可实现 ViewModel 层静态库调用。
