# 19 API 能力匹配分析（静态库完成回退检查，v0.2.0）

> 前置：`design/v0.2.0/gproto/77_静态库完成契约.md`、`17_API缺失需求.md`
> 本步：确认静态库流程完成、公共接口交还 GUI，重新比较能力与公共 API。

## 1. 契约完整性检查

| 契约要素 | 要求 | 实际 | 结论 |
| --- | --- | --- | --- |
| 公共头文件 | 存在 | `psig/include/psig.h` | ✅ |
| API 列表 | 完整 | 12 个公共函数（见契约 §2） | ✅ |
| 库产物 | `out/lib/<配置>` | `psig/out/lib/Debug/psig.lib`、`psig/out/lib/Release/psig.lib` | ✅ |
| 能力覆盖结果 | 逐项 | 见下 | ✅ |

## 2. 能力再匹配

| 编号 | 能力 | 公共 API | 覆盖 |
| --- | --- | --- | --- |
| M-1 | 模板 + CSV | `Table.templateText` + `Import/ExportSignaturesCsv` | ✅ |
| M-2 | 模板 + DB | `Import/ExportSignaturesDb`（`sig_meta`） | ✅ |
| M-3 | 自定义列 | `MatchResult.custom` | ✅ |
| M-4 | 导出带模板 | `ResultExportOptions.templateText` + `ExportResultsCsv/Db/Txt` | ✅ |
| M-5 | 选列导出 | `ResultExportOptions.columns` + `FilterResultColumns` | ✅ |

**全部覆盖** → 继续 GUI 设计（步骤 22）。

## 3. 约束确认

- 契约要素齐全，无需返回静态库流程。
- 无未覆盖能力，无需更新缺失需求文档。
- GUI 现在可以开始 ViewModel 层对新增 API 的调用编码（此前已按约束停止）。
- 静态库不引用 Qt；GUI 仅经公共 API 调用静态库。

## 4. 交接

- 回退到 GUI 流程步骤 22「模块设计」，进入编码前的设计阶段。
