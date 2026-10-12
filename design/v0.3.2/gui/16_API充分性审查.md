# 16 API 充分性审查（v0.3.2，增量）

- 版本：v0.3.2
- 前置：`14_API能力需求.md`、`psig/include/psig.h`
- 审查对象：本版改动点所需的静态库公共 API 是否满足全部功能需求并支持并发调用

## 一、逐项审查

| 需求 | 期望公共 API | 头文件逐字命中 | 满足 |
| --- | --- | --- | --- |
| 导出特征码（保存） | `ExportSignaturesDb` / `ExportSignaturesCsv` | `psig.h:156-157` | ✅ |
| 导入特征码 | `ImportSignaturesDb` / `ImportSignaturesCsv` | `psig.h:151-152` | ✅ |
| 表结构组装/读取 | `Table` / `TableRow` / `ColumnDef` / `BuiltinSignatureColumns()` | `psig.h:63-76,143` | ✅ |

## 二、并发性

- 本版新增/修改的调用点（导入/导出）均在主线程同步调用，且 v0.3.1 已在扫描路径验证静态库并发可用（`Scan` 系列）。
- 本版未新增并发调用静态库的路径。

## 三、结论

- **满足** → 继续 GUI 设计。
- ViewModel 直接调用静态库公共 API（不引入 OS API 作为能力提供方；不在 GUI 自行实现缺失功能或编造数据）。

## 四、约束满足说明

- 匹配对象为静态库公共 API；无 OS API 充当能力提供方；GUI 未编造数据/功能。
