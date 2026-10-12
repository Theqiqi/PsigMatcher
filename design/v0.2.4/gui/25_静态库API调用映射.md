# 25 静态库 API 调用映射（v0.2.4，增量）

> 前置：`24_ViewModel接口.md`、`design/v0.2.4/gproto/77_静态库完成契约.md`

## 映射表（含本次扩展）

| VM 行为 | psig 公共 API | 说明 |
| --- | --- | --- |
| 刷新进程 | `EnumerateProcesses()` | **行为修正**：`name` 快照名优先，非空（C-01） |
| 进程图标 | `ProcessIcon(pid)` / `DefaultProcessIcon()` | 沿用（v0.2.2） |
| 开始扫描 | `ParsePattern` + `ScanProcesses` | 不变（仅读 `pattern`/`name`） |
| 取消扫描 | `RequestCancel` / cancel 标志 | 不变 |
| 初始列 | `BuiltinSignatureColumns()` | 沿用（v0.2.2） |
| 内置列定义 | `ResultColumnDefs()` | 沿用（含 `address_dec`） |
| 选择导出键 | `SelectResultKeys(columns)` | 沿用 |
| 导入特征码 | `ImportSignaturesCsv/Db(path, err) -> Table` | 沿用；**GUI 不再读 `Table::templateText`**（C-04） |
| 导出特征码 | `ExportSignaturesCsv/Db(path, table, err)` | 沿用；**GUI 传空 `templateText`**（C-04） |
| 导出结果 CSV | `ExportResultsCsv(path, results, err, opt)` | `opt={templateText, columns, headers, summary}`；`headers`/`summary` 优先（C-03） |
| 导出结果 TXT | `ExportResultsTxt(path, results, err, opt)` | 同上；摘要行取 `opt.summary`（C-03） |
| 导出结果 DB | `ExportResultsDb(...)` | psig 保留；**GUI 移除入口**（C-05） |
| 结果自定义列 | `MatchResult.custom` | 沿用 |

## 转换（VM 私有）

- 导出表头构造（C-03）：`exportOptions()` 按 `m_language` 经 `resultHeaderForKey` 填充 `opt.headers`（`custom`→`tr("备注")`，英文 `Note`）。
- 导出摘要构造（C-03）：`opt.summary = tr("匹配结果 (%1 条)").arg(n)`（英文 `Results (n)`）。
- 特征码表构造（C-04）：`tableFromModel()` 不再设置 `templateText`；`applyTableToModel()` 不再读取 `templateText`。
- `IconBitmap → QIcon`：沿用 v0.2.2。

## 约束满足说明

- VM 每个业务方法对应真实 psig 公共 API；进程名与摘要本地化由 psig 提供/消费其字段，GUI 不调用 Win32、不自行实现缺失能力。
- 模板文本与特征码表解耦：psig 的 `Table::templateText` 保留（通用能力），GUI 不再使用。
