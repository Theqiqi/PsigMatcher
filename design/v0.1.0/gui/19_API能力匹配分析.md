# 19 API 能力匹配分析文档（静态库完成回退检查）

- 版本：v0.1.0
- 前置：静态库完成契约（`design/v0.1.0/gproto/77_静态库完成契约.md`）

## 1. 契约完整性检查

| 项 | 结果 |
| --- | --- |
| 公共头文件 | 有：`psig/include/psig.h` |
| API 列表 | 有：12 个公共 API |
| Debug 库产物 | 有：`psig/out/lib/Debug/psig.lib` |
| Release 库产物 | 有：`psig/out/lib/Release/psig.lib` |
| 能力覆盖结果 | 有：M1–M10 全覆盖 |
| 测试结果 | 单元 25/25、集成 6/6（Debug+Release） |

## 2. 重新比较 GUI 能力与静态库公共 API

| GUI 能力 | 静态库 API | 覆盖 |
| --- | --- | --- |
| 枚举进程 | EnumerateProcesses | 是 |
| 解析/校验模式串 | ParsePattern | 是 |
| 批量扫描 | ScanProcesses | 是 |
| 取消扫描 | RequestCancel / cancel 标志 | 是 |
| 初始化列 | BuiltinSignatureColumns | 是 |
| 导入特征码 CSV/DB | ImportSignaturesCsv/Db | 是 |
| 导出特征码 CSV/DB | ExportSignaturesCsv/Db | 是 |
| 导出结果 CSV/DB/TXT | ExportResultsCsv/Db/Txt | 是 |

## 3. 结论

- 全部能力覆盖 → 继续 GUI 设计。
- ViewModel 层现可开始实现静态库调用（此前冻结）。
- 匹配对象均为 psig 公共 API，未使用 OS API 作为 GUI 能力提供方。

## 4. 约束满足说明

- 契约齐全、能力全覆盖、逐项列出真实函数与验证证据；匹配完成，允许实现 ViewModel 层。
