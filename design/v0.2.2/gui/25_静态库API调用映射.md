# 25 静态库 API 调用映射（v0.2.2，增量）

> 前置：`24_ViewModel接口.md`、`design/v0.2.2/gproto/77_静态库完成契约.md`

## 映射表（含本次扩展）

| VM 行为 | psig 公共 API | 说明 |
| --- | --- | --- |
| 刷新进程 | `EnumerateProcesses()` | 不变（返回含 `accessible` 标志） |
| 进程图标 | `ProcessIcon(pid)` | 沿用；失败回退 `DefaultProcessIcon()` |
| 默认图标 | `DefaultProcessIcon()` | 沿用 |
| 开始扫描 | `ParsePattern` + `ScanProcesses` | 不变（仅读 `pattern`/`name`） |
| 取消扫描 | `RequestCancel` / cancel 标志 | 不变 |
| 初始列 | `BuiltinSignatureColumns()` | **新增用途**：提供列管理下拉键选项（C-02） |
| 内置列定义 | `ResultColumnDefs()` | **变更**：10 项，新增 `address_dec`（地址(十进制)），`custom` 标题→「备注」（C-01） |
| 选择导出键 | `SelectResultKeys(columns)` | 沿用；`address_dec` 默认不含、显式请求才含（C-01） |
| 导入特征码 | `ImportSignaturesCsv/Db(path, err) -> Table` | 沿用；CSV 自动剥 BOM |
| 导出特征码 | `ExportSignaturesCsv/Db(path, table, err)` | 沿用 |
| 导出结果 CSV | `ExportResultsCsv(path, results, err, opt)` | `opt = {templateText, columns, headers}`；**`headers` 优先**（C-05） |
| 导出结果 DB | `ExportResultsDb(path, results, err, opt)` | 同上 |
| 导出结果 TXT | `ExportResultsTxt(path, results, err, opt)` | 同上 |
| 标题→键回映射 | `BuiltinKeyFromTitle(title)` | **扩展**：英文映射（Name/Signature/Remarks/Note 等）→ 内置键（C-06） |
| 结果自定义列 | `MatchResult.custom` | `custom` ↔ 备注（C-01） |
| 结果删行 | —（Model 层 `removeRows`） | 不涉及 psig |
| 列排序 / 筛选 / 显隐 / 语言 / 对话框 / 日志 | —（View/Model/VM） | 不涉及 psig |

## 转换（VM 私有）

- `IconBitmap → QIcon`：沿用 v0.2.1，仅对 `accessible` 行调用（C-09）。
- 导出表头构造（C-05/C-06）：VM 按 `m_language` 选语言标题填充 `opt.headers`：
  - 内置列：中文 `名称/特征码/备注` 等，英文 `Name/Signature/Remarks` 等（与 `ResultColumnDefs` 标题集合对应，经 `BuiltinKeyFromTitle` 可互转）。
  - 自定义「备注」列：固定 `备注`（不随语言）。
- `ResultRow → MatchResult` / `MatchResult → ResultRow`：含 `custom`（备注，沿用）。

## 约束满足说明

- VM 每个业务方法对应真实 psig 公共 API；图标、列键枚举、导出表头均由 psig 提供或消费其 `headers` 字段，GUI 不调用 Win32、不自行实现缺失能力。
- `address_dec` 十进制地址由静态库 `ResultValueForKey` 以 `std::to_string(m.address)` 输出，GUI 仅透传。
