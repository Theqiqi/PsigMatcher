# 14 API 能力需求（v0.3.2，增量）

- 版本：v0.3.2
- 模式：修改型。仅列改动点需要的静态库能力；其余沿用 `design/v0.3.1/gui/14_API能力需求.md`。

## 一、改动点所需能力逐条

| 编号 | 能力需求 | 期望公共 API（既有） | 输入 | 输出 | 异常要求 | 验收标准 |
| --- | --- | --- | --- | --- | --- | --- |
| C-01 | 特征码表导出为文件（「保存」语义） | `psig::ExportSignaturesDb(path, table, err)` / `ExportSignaturesCsv(...)` | 输出路径 + `psig::Table` | `bool` + `err` 字符串 | 不抛异常，失败经 `err` 返回 | 成功返回 true 且文件存在；失败返回 false 且 `err` 非空 |
| C-01/C-02 | 特征码表从文件导入 | `psig::ImportSignaturesDb(path, err)` / `ImportSignaturesCsv(...)` | 输入路径 | `psig::Table` + `err` | 不抛异常，失败经 `err` 返回 | 成功时 `err` 为空且返回表；失败时 `err` 非空 |
| C-01 | 组装/读取特征码表结构 | `psig::Table` / `psig::TableRow` / `psig::ColumnDef` / `psig::BuiltinSignatureColumns()` | — | 结构体/列表 | — | 结构字段与 v0.3.1 一致 |
| C-03 | 保存（同 C-01 导出） | 同 C-01 | 同 C-01 | 同 C-01 | 同 C-01 | 同 C-01 |
| C-02/C-03/C-04 | 无新增能力（纯 GUI 交互/命名） | — | — | — | — | — |

## 二、结论

- 本版改动点所需能力**全部**为 v0.3.1 已有的公共 API；无新增能力需求。
- 已有的 API 可覆盖 → 直接调用；**无**记入缺失能力的项。

## 三、约束满足说明

- 逐条列出改动点所需能力、期望公共 API、输入输出、异常要求、验收标准。
