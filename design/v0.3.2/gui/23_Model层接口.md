# 23 Model 层接口（v0.3.2，增量）

> 前置：`09_Qt_ModelView绑定.md`、`04_GUI需求分析.md`

## 结论：**Model 层无改动**

| Model | 是否改动 | 说明 |
| --- | --- | --- |
| `SignatureTableModel` | **无** | `setData`（`signaturetablemodel.cpp:30-39`）已在编辑后 `emit dataChanged(index, index, {DisplayRole, EditRole})`；本版由 ViewModel 连接该信号实现真实脏标记，Model 不需要新增接口或状态 |
| `ProcessTableModel` | 无 | — |
| `ResultTableModel` | 无 | 结果页无持久化语义，不引入未保存概念 |

## 契约核对

- `SignatureTableModel::flags` 对有效索引返回 `ItemIsEnabled | ItemIsSelectable | ItemIsEditable`（可编辑，符合需求）。
- 行增删/列变更经 `begin/endInsertRows`、`begin/endRemoveRows`、`begin/endResetModel`，与 `dataChanged` 语义区分正确。
- Model 不 include `view/`、`viewmodel/`、静态库头；不直接调用静态库 API（沿用）。

## 约束满足说明

- 修改模式：本版未改动 Model 层文件，仅确认既有契约（`setData` 发 `dataChanged`）足以支撑 C-01。
