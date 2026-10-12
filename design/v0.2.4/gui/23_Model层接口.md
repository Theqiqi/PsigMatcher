# 23 Model 层接口（v0.2.4，增量）

> 前置：`09_Qt_ModelView绑定.md`、`design/v0.2.2/gui/23_Model层接口.md`（上一版本）

## ProcessTableModel（增量）

```cpp
class ProcessTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    int rowCount(...) const override;
    int columnCount(...) const override;       // 4
    QVariant data(...) const override;         // DisplayRole；名称列直接返回 r.name
    QVariant headerData(...) const override;
    Qt::ItemFlags flags(...) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void setProcesses(const std::vector<ProcessRow>& list);
    void setIcon(int row, const QIcon& icon);
    std::vector<ProcessRow> processes() const;
    ProcessRow processAt(int row) const;
};
```

### 列布局（4 列，不变）

| 列 | 标题 | DecorationRole |
| --- | --- | --- |
| 0 | PID | 否 |
| 1 | 名称 | 仅 `accessible==true` 行返回图标（沿用 v0.2.2） |
| 2 | 路径 | 否 |
| 3 | 可访问 | 否 |

### 名称列（C-01）

- `data(DisplayRole, 列=名称)`：直接返回 `r.name`（由 psig `EnumerateProcesses()` 提供的真实进程名，正常非空）。
- **移除** v0.2.2 的「PID 0→系统空闲进程 / PID 4→系统 / 其他→未知进程」展示兜底，使列表不再出现「未知进程 / Unknown Process」。

## SignatureTableModel / ResultTableModel

- 无本版改动（沿用 v0.2.2）；`resultHeaderForKey` 的本地化在 ViewModel 侧（见 24）。

## model/types.h

- 沿用 `ProcessRow { pid, name, path, accessible, icon }`。

## 约束满足说明

- Model 纯净：仅依赖 Qt 与 `model/types.h`，不依赖 psig。
- 名称来源（快照名）由 psig 提供，Model 仅展示。
