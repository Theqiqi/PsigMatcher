# 19 API 能力匹配分析（v0.3.2，增量）

- 版本：v0.3.2
- 前置：`15_静态库存在性匹配.md`、`16_API充分性审查.md`、`17_API缺失需求.md`、`18_静态库流程交接记录.md`

## 一、匹配结果

| 改动点所需能力 | 匹配的公共 API | 匹配结论 |
| --- | --- | --- |
| 保存/导出特征码 | `psig::ExportSignaturesDb` / `psig::ExportSignaturesCsv` | ✅ 已覆盖 |
| 导入特征码 | `psig::ImportSignaturesDb` / `psig::ImportSignaturesCsv` | ✅ 已覆盖 |
| 表结构与内置列 | `psig::Table` / `TableRow` / `ColumnDef` / `BuiltinColumns` | ✅ 已覆盖 |

## 二、完成回退检查

| 检查项 | 结论 |
| --- | --- |
| 静态库流程是否完成 | 不适用（本版未进入该流程） |
| 公共头文件 | `psig/include/psig.h` 存在且未变 |
| API 列表 | v0.3.1 契约 API 不变 |
| 库产物 | `psig/out/lib/{Debug,Release}/psig.lib` 存在 |
| 未覆盖能力 | **无** |

- → 全部覆盖，**继续 GUI 设计**；不跳回静态库流程。
- 未在匹配完成前实现 ViewModel 层的静态库调用（本版调用点均为既有 API）。

## 三、约束满足说明

- 更新后的匹配分析确认：改动点能力与静态库公共 API 一一对应，无缺失、无回退。
