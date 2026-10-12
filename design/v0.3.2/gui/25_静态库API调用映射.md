# 25 静态库 API 调用映射（v0.3.2，增量）

- 版本：v0.3.2
- 前置：`psig/include/psig.h`、`24_ViewModel接口.md`

## 一、本版调用点映射

| VM 方法/流程 | 调用的公共 API | 头文件位置 | 备注 |
| --- | --- | --- | --- |
| `exportSignaturesToPath(path)` | `psig::ExportSignaturesDb` / `psig::ExportSignaturesCsv` | `psig.h:156-157` | 由「保存（关闭/重选提示）」与「导出特征码」复用；成功后 `m_sigDirty=false` |
| `importSignaturesFromPath(path)` | `psig::ImportSignaturesDb` / `psig::ImportSignaturesCsv` | `psig.h:151-152` | 成功后 `m_sigDirty=false` |
| `tableFromModel()` | `psig::Table` / `TableRow` / `ColumnDef` | `psig.h:63-76` | 组装导出数据 |
| `applyInitialColumns()` | `psig::BuiltinSignatureColumns()` | `psig.h:143` | 初始列 |
| `onManageColumns()` | `psig::BuiltinSignatureColumns()` | `psig.h:143` | 列键下拉源 |
| 扫描相关（`scan.cpp`） | `psig::Scan` 系列 | `psig.h`（v0.3.1 映射不变） | 本版未改 |

## 二、本版是否新增静态库调用

- **否**。C-01/C-02/C-03 复用既有导入/导出调用；C-04 为命名改动。
- GUI 不自行实现 `.db`/`.csv` 读写（全部经静态库公共 API）。

## 三、版本字符串

- `psig/include/psig.h` 顶部注释版本 `v0.3.1` → `v0.3.2`（与 GUI 版本统一；纯注释，无 API/行为变化）。
- `psig::Table::targetProcess` 字段注释中的 `v0.3.1` 保留其引入版本语义（不改）。

## 四、约束满足说明

- 每个 VM 业务方法对应真实静态库 API 调用；无对应 API 的情形仅 C-03 的「保存」——它复用既有 `ExportSignatures*`，不触发静态库流程。
- 静态库公共头不引用 Qt；GUI 只经公开 API 调用，不 include `psig/src/*`。
