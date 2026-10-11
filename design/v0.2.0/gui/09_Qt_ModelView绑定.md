# 09 Qt Model/View 数据绑定（v0.2.0，增量）

> 前置：`08_窗口层次结构与导航图.md`、`design/v0.1.0/gui/09_Qt_ModelView绑定.md`（基线）
> 说明：仅更新受改动影响的模型/视图绑定。

## 1. 模型清单（增量）

| 模型 | 视图 | 变更 |
| --- | --- | --- |
| SignatureTableModel | SignatureEditorPage::table | 不变（动态列、可编辑） |
| ProcessTableModel | ProcessSelectionPage::table | 不变（只读） |
| ResultTableModel | ResultPage::table | **修改**：新增列、末列可编辑、支持删行 |

## 2. ResultTableModel 变更

### 2.1 列定义（10 列）

| 列号 | 标题 | 可编辑 | 说明 |
| --- | --- | --- | --- |
| 0 | 特征码名 | 否 | |
| 1 | 进程 | 否 | |
| 2 | PID | 否 | |
| 3 | **地址(十进制)** | 否 | **新增**：`QString::number(address)` |
| 4 | 地址(十六进制) | 否 | 原「地址」，`0x…`（大写 hex，前缀小写 `0x`） |
| 5 | 模块 | 否 | |
| 6 | 模块基址 | 否 | |
| 7 | 匹配字节 | 否 | |
| 8 | 命中时间 | 否 | |
| 9 | **自定义** | **是** | **新增**：末列，可单格编辑/批量填充 |

### 2.2 接口变更

- `Qt::ItemFlags flags()`：第 9 列返回 `ItemIsEditable`，其余只读。
- `bool setData(index, value, Qt::EditRole)`：仅第 9 列生效，写回 `ResultRow::custom` 并 `emit dataChanged`。
- `void removeRows(const std::vector<int>& rows)`：**新增**，升序排序后逆序 `beginRemoveRows/endRemoveRows`。
- `void setCustom(const std::vector<int>& rows, const QString& text)`：**新增**，批量填充自定义列（多选/全选）。
- `QVariant data()`：第 3 列返回十进制地址；第 9 列返回 `custom`。
- `std::vector<ResultRow> results()`：随 `custom` 一并返回，供导出使用。

## 3. 数据流（增量）

```
扫描完成 → psig::MatchResult[] → ResultRow[]（含 custom=""）→ ResultTableModel::setResults
末列编辑 → QTableView 编辑 → setData(col=9) → ResultRow::custom → dataChanged → 视图刷新
批量填充 → VM 取选中行 → ResultTableModel::setCustom(rows, text) → dataChanged
删行     → VM 取选中行 → ResultTableModel::removeRows(rows)
导出     → VM 取 ResultTableModel::results() + 模板文本 + 选中列 → psig::ExportResults*（携带模板/自定义列/选列）
导入     → psig::ImportSignatures* → Table（含 templateText）→ SignatureTableModel::setColumns/setRows + ResultPage 模板区回填
```

## 4. 约束满足说明

- Model 纯净：ResultTableModel 仅依赖 Qt 与 `model/types.h`，不含 psig 类型（导出转换在 ViewModel 完成）。
- 可编辑性由 `flags()`/`setData()` 精确控制，仅末列可写。
- 模板文本为**表级元数据**，不属于行模型，由 View（QPlainTextEdit）承载、VM 读写并经 psig 持久化。
