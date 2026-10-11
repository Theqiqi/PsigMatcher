# 25 静态库 API 调用映射文档

- 版本：v0.1.0
- 前置：`11_GUI执行路径.md`、`psig/include/psig.h`（静态库公共 API 文档）

> 每个 VM 业务方法对应一个真实的静态库公共 API 调用；VM 只做类型转换与 API 调用，不做业务。

| VM 业务方法 | 静态库公共 API | 类型转换（Model ↔ psig） |
| --- | --- | --- |
| onRefreshProcesses | `psig::EnumerateProcesses()` | `ProcessInfo` → `ProcessRow`（wstring→QString） |
| onStartScan | `psig::ParsePattern(raw,name)` × N；`psig::ScanProcesses(req, cb, &m_cancel)` | `SignatureRow` → `Pattern`（取 pattern 列）；`MatchResult` → `ResultRow`（地址→"0x..."，wstring→QString） |
| onCancelScan | `psig::RequestCancel(true)` 或 `m_cancel.store(true)` | — |
| onImportSignatures(CSV) | `psig::ImportSignaturesCsv(path, err)` | `psig::Table` → `SignatureTableModel`（ColumnDef/rows） |
| onImportSignatures(DB) | `psig::ImportSignaturesDb(path, err)` | 同上 |
| onExportSignatures(CSV) | `psig::ExportSignaturesCsv(path, table, err)` | `SignatureTableModel` → `psig::Table` |
| onExportSignatures(DB) | `psig::ExportSignaturesDb(path, table, err)` | 同上 |
| onExportResultsCsv | `psig::ExportResultsCsv(path, results, err)` | `ResultRow` → `psig::MatchResult` |
| onExportResultsDb | `psig::ExportResultsDb(path, results, err)` | 同上 |
| onExportResultsTxt | `psig::ExportResultsTxt(path, results, err)` | 同上 |
| 初始化列 | `psig::BuiltinSignatureColumns()` | `psig::ColumnDef` → Model `ColumnDef` |

## 进度回调映射

- `psig::ScanProcesses` 的 `ProgressCallback` 在后台线程触发 → VM 经 `QMetaObject::invokeMethod(..., Qt::QueuedConnection)` 更新 `MainWindow::setProgress`。

## 无对应 API 的情况

- 无（全部能力由 psig 覆盖，见静态库完成契约）。

## 约束满足说明

- 每个 VM 业务方法对应一个真实静态库 API 调用；无未映射能力。
