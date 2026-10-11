# 14 API 能力需求分析（v0.2.0，增量）

> 前置：`11_GUI执行路径.md`、`psig` 公共头文件 `psig/include/psig.h`、`design/v0.1.0/gproto/77_静态库完成契约.md`
> 模式：修改/添加功能 → 聚焦改动点**新增**的能力需求；已有 API 可覆盖的直接调用。

## 1. 改动点 → 能力需求

| 能力编号 | 关联改动点 | 能力需求 | 期望公共 API | 输入 | 输出 | 异常要求 | 验收标准 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| CAP-1 | C-07 | 特征码表 CSV **携带表级模板文本**（导出写入 / 导入读出） | `ExportSignaturesCsv` / `ImportSignaturesCsv`（`Table` 增字段） | 文件路径 + `Table{…, templateText}` | 成功/失败 + `Table.templateText` | 无模板标记 → `templateText` 为空且不报错；文件不可读写 → 置 err | 导出含多行模板的 CSV → 重新导入 → `templateText` 逐字一致 |
| CAP-2 | C-07 | 特征码表 DB **携带表级模板文本** | `ExportSignaturesDb` / `ImportSignaturesDb`（`Table` 增字段） | 同上 | 同上 | 旧库无 meta 表 → 模板为空且不报错 | DB 往返模板一致 |
| CAP-3 | C-07 | 结果导出**携带模板文本** | 结果导出 API 接受模板文本参数 | 文件路径 + 结果 + 模板文本 | 成功/失败 | 模板为空 → 正常导出 | 导出文件含模板文本；TXT 中模板位于文件最前 |
| CAP-4 | C-08 | 结果行**自定义列值**持久化 | `MatchResult` 增 `custom` 字段；结果导出写入 | 结果（含 custom） | 导出文件含自定义列 | 无 | 自定义列值出现在导出 CSV/DB/TXT |
| CAP-5 | C-07 | 结果导出支持**选择列子集** | 结果导出 API 接受列选择参数 | 列键列表（空=全部） | 仅含选中列的文件 | 空列表按全部处理 | 仅导出指定列 |
| CAP-6 | C-06 | （结果集删行） | 无需静态库 | — | — | — | GUI/Model 层实现 |
| CAP-7 | C-09/C-10/C-11/C-01/C-02/C-03/C-04/C-05 | 纯 GUI 交互 | 无需静态库 | — | — | — | GUI 层实现 |

## 2. 现有 API 覆盖情况

| 能力 | 现有 API | 是否覆盖 |
| --- | --- | --- |
| CAP-1/CAP-2 | `Import/ExportSignaturesCsv/Db` + `Table` | **部分**（`Table` 无模板字段、序列化无模板）→ 需扩展 |
| CAP-3/CAP-4/CAP-5 | `ExportResultsCsv/Db/Txt` + `MatchResult` | **部分**（`MatchResult` 无 `custom`；导出无模板/列选择参数）→ 需扩展 |
| CAP-6/CAP-7 | — | 不需要静态库 |

## 3. 约束满足说明

- 逐条列出改动点所需能力、期望公共 API、输入输出、异常要求、验收标准。
- 已有 API 能覆盖的（枚举/解析/扫描/基础导入导出）→ 直接调用；不足的（CAP-1..CAP-5）→ 记入缺失能力，走静态库流程（步骤 17-20）。
- 匹配对象仅为静态库公共 API；GUI 不得自行实现缺失能力或编造数据。
