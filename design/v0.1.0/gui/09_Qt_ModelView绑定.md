# 09 Qt Model/View 数据绑定文档

- 版本：v0.1.0
- 前置：`08_窗口层次结构与导航图.md`

## 1. 绑定总览

| 视图 (QTableView) | 模型 (QAbstractTableModel 子类) | 可写性 | 选择模式 |
| --- | --- | --- | --- |
| SignatureEditorPage.m_table | SignatureTableModel | 可编辑 | ExtendedSelection（多选） |
| ProcessSelectionPage.m_table | ProcessTableModel | 只读 | ExtendedSelection（多选） |
| ResultPage.m_table | ResultTableModel | 只读 | ExtendedSelection（多选） |

## 2. Model 接口约定（QAbstractTableModel 子类）

所有模型实现：`rowCount`、`columnCount`、`data`、`headerData`；可编辑模型额外实现 `flags`、`setData`。

### 2.1 SignatureTableModel
- 数据：`std::vector<SignatureRow>`（动态列：内置列 + 用户列，列为 `std::vector<ColumnDef>`）。
- `rowCount = rows.size()`；`columnCount = columns.size()`。
- `data()`：
  - `DisplayRole`/`EditRole` → 返回该行该列的值（QString）。
  - `BackgroundRole` → 模式非法行返回浅红。
- `setData()`：写入值并 `emit dataChanged`。
- `flags()`：可编辑列返回 `ItemIsEditable | ItemIsEnabled | ItemIsSelectable`。
- 结构方法（非 Model/View 标准，供 VM 调用）：`addRow()`、`removeRows(indices)`、`setColumns(cols)`、`setRows(rows)`。

### 2.2 ProcessTableModel
- 数据：`std::vector<ProcessInfo>`。
- 列固定：PID / 名称 / 路径 / 可访问。
- `flags()`：只返回 `ItemIsEnabled | ItemIsSelectable`（只读）。
- `data()`：按列返回对应字段（DisplayRole）。
- `setProcesses(list)`：整体替换并 reset。

### 2.3 ResultTableModel
- 数据：`std::vector<MatchResult>`。
- 列固定：特征码名 / 进程 / PID / 地址 / 模块 / 匹配字节 / 命中时间。
- 只读；`appendResult`、`clear`、`setResults` 供 VM 调用。
- 地址、匹配字节格式化在 Model 的 `data()` 内完成（十六进制显示），不做业务。

## 3. 绑定时机

- Model 由 ViewModel 持有并负责生命周期。
- View 在构造时 `setModel(nullptr)`；由 ViewModel 在 `install()` 后通过 View 公开方法 `setSignatureModel(model)` / `setProcessModel(model)` / `setResultModel(model)` 注入。
- 视图选择模式在 View 构造时设置：`setSelectionBehavior(SelectRows)`、`setSelectionMode(ExtendedSelection)`。

## 4. 编辑流（特征码表）

```
用户双击单元格 → QTableView 委托进入编辑
   → 用户提交 → View.setData(...) 由 ViewModel 经 Model.setData 写入
   → Model emit dataChanged → 视图刷新
```

- 只读表（进程/结果）`setEditTriggers(NoEditTriggers)`，不产生编辑流。

## 5. 数据流一致性

- 所有数据变更经 Model 方法（`setData`/`setRows`/`setProcesses`/`setResults`/`clear`），视图不直接改数据结构。
- Model 不持有 View/ViewModel，不包含业务逻辑；仅做数据存取与显示格式转换。

## 6. 约束满足说明

- 已给出每个表格视图与其 QAbstractTableModel 子类的绑定关系、列定义、可写性与刷新机制。
