# 23 Model 层接口（v0.2.1，增量）

> 前置：`09_Qt_ModelView绑定.md`、`design/v0.2.0/gui/23_Model层接口.md`（基线）

## ProcessTableModel（增量）

```cpp
class ProcessTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    int rowCount(...) const override;
    int columnCount(...) const override;       // 4
    QVariant data(...) const override;         // DisplayRole；名称列 DecorationRole = 图标
    QVariant headerData(...) const override;
    Qt::ItemFlags flags(...) const override;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override; // 新增

    void setProcesses(const std::vector<ProcessRow>& list);
    void setIcon(int row, const QIcon& icon);  // 新增（异步图标回填）
    std::vector<ProcessRow> processes() const;
    ProcessRow processAt(int row) const;
};
```

### 列布局（4 列）

| 列 | 标题 | DecorationRole |
| --- | --- | --- |
| 0 | PID | 否 |
| 1 | 名称 | **是**（进程图标） |
| 2 | 路径 | 否 |
| 3 | 可访问 | 否 |

- `sort()`：列 0=PID（数值）、1=名称（忽略大小写）、2=路径、3=可访问（bool）；以 `std::stable_sort` 重排 `m_rows` 并 `beginResetModel/endResetModel`（重置后由 VM 重新应用筛选）。

## SignatureTableModel（增量）

```cpp
class SignatureTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    // 既有：rowCount/columnCount/data/setData/headerData/flags/setColumns/columns/setRows/rows/removeRows/valueAt
    void addRow(const QString& processDefault = QString());        // 修改：可选默认值
    void addRows(int count, const QString& processDefault = QString()); // 新增
};
```

- `addRow/addRows`：插入空行；若存在键为 `process` 的列且 `processDefault` 非空，则写入该列。
- `addRows`：`beginInsertRows(first, last)` 一次性插入 `count` 行。
- `headerData`：内置列（`name`/`pattern`/`note`/`process`）标题经 `tr()` 返回（随语言切换）；自定义列沿用其 `ColumnDef.title`。
- `retranslate()`（新增，三个 Model 均提供）：语言切换后 `emit headerDataChanged(Qt::Horizontal, 0, n-1)` 刷新表头。

## model/types.h（增量）

```cpp
struct ProcessRow {
    quint32 pid = 0;
    QString name;
    QString path;
    bool accessible = false;
    QIcon icon;              // 新增（显示属性，不参与导入导出）
};
```

> 需 `#include <QIcon>`（Model 层仅依赖 Qt）。

## ResultTableModel

- 不变（沿用 v0.2.0）。

## 约束满足说明

- Model 纯净：仅依赖 Qt 与 `model/types.h`，不依赖 psig；图标以 `QIcon` 承载（VM 注入），排序仅作用于展示顺序。
