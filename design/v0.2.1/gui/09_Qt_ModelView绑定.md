# 09 Qt Model/View 数据绑定（v0.2.1，增量）

> 前置：`08_窗口层次结构与导航图.md`、`design/v0.2.0/gui/09_Qt_ModelView绑定.md`（基线）
> 说明：仅更新受改动影响的模型/视图绑定。

## 1. 模型清单（增量）

| 模型 | 视图 | 变更 |
| --- | --- | --- |
| SignatureTableModel | SignatureEditorPage::table | **修改**：新增 `addRows(int)`（新增多行）；`addRow()` 支持默认值 |
| ProcessTableModel | ProcessSelectionPage::table | **修改**：新增图标列/DecorationRole、`sort()` 排序 |
| ResultTableModel | ResultPage::table | 不变（沿用 v0.2.0 的 10 列/末列可编辑/删行） |

## 2. ProcessTableModel 变更（C-11 / C-12）

### 2.1 数据角色

- `data(index, Qt::DisplayRole)`：列 0=PID、1=名称、2=路径、3=可访问（`是/否`）。
- `data(index, Qt::DecorationRole)`：**列 1（名称）** 返回进程图标 `QIcon`（由 VM 注入，缺省为空）。
- 图标以 `QIcon` 缓存在行数据（`ProcessRow::icon`）中，避免每帧重取。

### 2.2 排序（`sort(column, order)`）

- 覆写 `void sort(int column, Qt::SortOrder order)`：对 `m_rows` 就地排序后 `layoutChanged`。
- 排序键：列 0=PID（数值）、1=名称（字符串，忽略大小写）、2=路径（字符串）、3=可访问（bool）。
- 视图侧 `setSortingEnabled(true)`；点击列头触发 `sort()`。

### 2.3 接口变更

- `void setProcesses(const std::vector<ProcessRow>& list)`：不变（行内含 `QIcon icon`）。
- `void setIcon(int row, const QIcon& icon)`（或 `setProcesses` 直接携带图标）：**新增**，用于异步图标回填。
- `ProcessRow` 新增 `QIcon icon;`（见 `model/types.h`）。

## 3. SignatureTableModel 变更（C-06 / C-08）

- `void addRow(const QString& processDefault = QString())`：**修改**，新增行时可选填入默认值（写入「进程」列键对应单元格）。
- `void addRows(int count, const QString& processDefault = QString())`：**新增**，一次性插入 `count` 行（`beginInsertRows(first, last)`）。
- 进程列键：`process`（**GUI 侧内置列**，由 VM 在 `applyInitialColumns()` 中于 psig 内置列之后追加；psig 的 `BuiltinSignatureColumns()` 仍为 name/pattern/note 三列，未改动静态库）；默认值仅在存在该键时写入。

## 4. 数据流（增量）

```
启动 → install() → 自动 onRefreshProcesses() → psig::EnumerateProcesses → ProcessTableModel::setProcesses
进程图标 → psig::ProcessIcon(pid)（异步）→ QImage/QIcon → ProcessTableModel::setIcon(row) → 视图刷新
列排序   → 表头点击 → ProcessTableModel::sort(col, order) → layoutChanged
新增一行 → VM onAddSignatureRow → SignatureTableModel::addRow(目标进程名) → 视图追加
新增多行 → VM onAddSignatureRows(n) → SignatureTableModel::addRows(n, 目标进程名)
导出 CSV → psig::ExportResultsCsv（写 UTF-8 BOM）
```

## 5. 约束满足说明

- Model 纯净：ProcessTableModel/SignatureTableModel 仅依赖 Qt 与 `model/types.h`，不含 psig 类型（图标由 VM 转成 `QIcon` 注入）。
- 可编辑性由 `flags()`/`setData()` 精确控制（不变）。
- 图标为**显示属性**，不参与导入导出；排序仅作用于展示顺序。
