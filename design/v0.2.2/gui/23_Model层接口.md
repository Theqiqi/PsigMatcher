# 23 Model 层接口（v0.2.2，增量）

> 前置：`09_Qt_ModelView绑定.md`、`design/v0.2.1/gui/23_Model层接口.md`（上一版本）

## ProcessTableModel（增量）

```cpp
class ProcessTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    int rowCount(...) const override;
    int columnCount(...) const override;       // 4
    QVariant data(...) const override;         // DisplayRole；名称列空时兜底；DecorationRole 仅 accessible 行
    QVariant headerData(...) const override;
    Qt::ItemFlags flags(...) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void setProcesses(const std::vector<ProcessRow>& list);
    void setIcon(int row, const QIcon& icon);  // 仅对 accessible 行生效（VM 侧已判定）
    std::vector<ProcessRow> processes() const;
    ProcessRow processAt(int row) const;
};
```

### 列布局（4 列）

| 列 | 标题 | DecorationRole |
| --- | --- | --- |
| 0 | PID | 否 |
| 1 | 名称 | **仅 `accessible==true` 行返回图标**（C-09） |
| 2 | 路径 | 否 |
| 3 | 可访问 | 否 |

### 名称兜底（C-03）

- `data(DisplayRole, 列=名称)`：若 `name.isEmpty()`，按 PID 返回兜底名：
  - `pid==0` → `tr("系统空闲进程")`
  - `pid==4` → `tr("系统")`
  - 其他为空 → `tr("未知进程")`
- 兜底名经 `tr()`，随语言切换（经 `retranslate()` 重发 `dataChanged`）。

### 图标注入（C-09）

- `setIcon(row, icon)`：仅当 `processAt(row).accessible == true` 时存储；`data(DecorationRole, 名称列)` 返回该图标，否则返回空。
- VM 侧 `applyProcessIcon` 也仅在 `accessible==true` 时调用 `setIcon`，保持单一判定来源。

## SignatureTableModel（增量，C-06）

```cpp
class SignatureTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    // 沿用：rowCount/columnCount/data/setData/headerData/flags/setColumns/columns/
    //       setRows/rows/removeRows/valueAt/addRow/addRows
    void retranslate();   // 语言切换后 emit headerDataChanged(Qt::Horizontal, 0, n-1)
};
```

- `headerData`：
  - 内置列（`name`/`pattern`/`note`/`process`）标题经 `tr()` 返回（**随语言变化**，C-06）。
  - 自定义列沿用其 `ColumnDef.title`（**不随语言变化**，C-06）。
- `retranslate()`：`emit headerDataChanged(Qt::Horizontal, 0, columnCount()-1)` 刷新表头。
- 导出时表头由 VM 按键映射（见 24/25），与界面显示解耦。

## ResultTableModel（增量，C-01）

- 末列（自定义/备注）列定义：`{"custom", tr("备注")}`（原为 `tr("自定义")`）。
- `headerData` 该列返回 `tr("备注")`，随语言切换。
- 导出键固定为 `custom`，与静态库 `ResultColumnDefs` 中 `{"custom","备注"}` 对应。

## model/types.h

- 沿用 `ProcessRow { pid, name, path, accessible, icon }`（v0.2.1）。

## 约束满足说明

- Model 纯净：仅依赖 Qt 与 `model/types.h`，不依赖 psig；图标以 `QIcon` 承载（VM 注入）；名称兜底与权限图标判定均在 Model 数据层完成（判定输入 `accessible` 来自 psig 枚举结果）。
