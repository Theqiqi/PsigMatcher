# 09 Qt Model/View 数据绑定（v0.3.2，增量）

- 版本：v0.3.2
- 模式：修改型。仅列本版受影响的数据绑定；其余沿用 `design/v0.3.1/gui/09_Qt_ModelView绑定.md`。

## 一、既有绑定（不变）

| View 控件 | Model | 绑定方式 |
| --- | --- | --- |
| `SignatureEditorPage` 的 `HoverRowTableView` | `SignatureTableModel` | `setModel()`，VM 经 `m_view->signaturePage()->setModel(m_sigModel)` |
| 进程表 | `ProcessTableModel` | 同上 |
| 结果表 | `ResultTableModel` | 同上 |

## 二、本版新增绑定（C-01）

```cpp
// gui/src/viewmodel/bind.cpp（bindSignaturePage）
connect(m_sigModel, &QAbstractItemModel::dataChanged,
        this, [this]() { markSignatureDirty(); });   // 单元格编辑 → 真实脏标记
```

- **方向**：Model 信号 → ViewModel 槽（View 不参与），符合 `View → VM → Model` 的绑定方向约束。
- **触发**：`SignatureTableModel::setData` 内的 `emit dataChanged(index,index,...)`。行增删/列变更经 `begin/endInsertRows`、`begin/endRemoveRows`、`begin/endResetModel`，**不**发 `dataChanged`；这些操作由 ViewModel 槽显式 `markSignatureDirty()`（既有），故两条路径互补、不重复。
- **后果**：`m_sigDirty` 反映真实内容变化；持久状态区 `*` 与实际状态一致。

## 三、`m_sigDirty` 的读写矩阵

| 操作 | 路径 | `m_sigDirty` |
| --- | --- | --- |
| 单元格编辑 | View 编辑 → `setData` → `dataChanged` → VM | `true` |
| 新增一行 / 多行 | 动作 → VM 槽 | `true` |
| 删除选中行 | 动作 → VM 槽 | `true` |
| 列管理（确定） | 对话框 → VM 槽 | `true` |
| 导出特征码成功 | VM → `psig::ExportSignatures*` | `false` |
| 导入特征码成功 | VM → `psig::ImportSignatures*` → `applyTableToModel` | `false` |
| 语言切换 / 切页 / 刷新进程 | — | 不变 |

> 注：`applyTableToModel` 内部 `setColumns`/`setRows` 用 reset 模型，不发 `dataChanged`，故不会在导入过程中被重新置脏。

## 四、约束满足说明

- Model/View 契约不变（特征码表 `flags` 含 `ItemIsEditable`；只读表不含）。
- 新增绑定集中在 ViewModel（`bindSignaturePage`），符合「connect 集中在 install()/bind*」约束。
