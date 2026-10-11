# 25 静态库 API 调用映射（v0.2.0，增量）

> 前置：`24_ViewModel接口.md`、`design/v0.2.0/gproto/77_静态库完成契约.md`

## 映射表

| VM 行为 | psig 公共 API | 说明 |
| --- | --- | --- |
| 刷新进程 | `EnumerateProcesses()` | 不变 |
| 开始扫描 | `ParsePattern` + `ScanProcesses` | 不变 |
| 取消扫描 | `RequestCancel` / cancel 标志 | 不变 |
| 初始列 | `BuiltinSignatureColumns()` | 不变 |
| 导入特征码 | `ImportSignaturesCsv/Db(path, err) -> Table` | 读 `table.templateText` |
| 导出特征码 | `ExportSignaturesCsv/Db(path, table, err)` | `table.templateText = m_templateText` |
| 导出结果 CSV | `ExportResultsCsv(path, results, err, opt)` | `opt = {templateText, columns}` |
| 导出结果 DB | `ExportResultsDb(path, results, err, opt)` | 同上 |
| 导出结果 TXT | `ExportResultsTxt(path, results, err, opt)` | 同上 |
| 结果自定义列 | `MatchResult.custom` | `ResultRow.custom` ↔ `MatchResult.custom` |
| 结果删行 | —（Model 层 `removeRows`） | 不涉及 psig |
| 十进制地址列 | —（Model 派生显示） | 不涉及 psig |
| 关于/图标/右键/显隐 | —（View/VM） | 不涉及 psig |

## 转换（VM 私有）

- `ResultRow → MatchResult`：`m.custom = r.custom.toStdString()`（其余既有）。
- `MatchResult → ResultRow`：`r.custom = QString::fromStdString(m.custom)`。
- `Table.templateText` ↔ `m_templateText`（QString ↔ std::string，UTF-8）。

## 约束满足说明

- VM 每个业务方法对应真实 psig 公共 API；无对应 API 的纯 UI 行为不经 psig。
- 匹配对象仅为 psig 公共 API；未使用 OS API 充当能力提供方；未自行实现缺失能力。
