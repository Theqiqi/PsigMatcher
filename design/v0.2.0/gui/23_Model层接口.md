# 23 Model 层接口（v0.2.0，增量）

> 前置：`09_Qt_ModelView绑定.md`

## ResultTableModel（增量）

```cpp
class ResultTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    // 既有：rowCount/columnCount/data/headerData/flags/setResults/results/appendResult/clear
    int rowCount(...) const override;
    int columnCount(...) const override;               // 由 8 → 10
    QVariant data(...) const override;
    QVariant headerData(...) const override;
    Qt::ItemFlags flags(...) const override;           // 第 9 列 ItemIsEditable

    bool setData(const QModelIndex&, const QVariant&, int role = Qt::EditRole) override; // 仅第 9 列

    void setResults(const std::vector<ResultRow>&);
    std::vector<ResultRow> results() const;
    void appendResult(const ResultRow&);
    void clear();

    void removeRows(const std::vector<int>& rows);              // 新增
    void setCustom(const std::vector<int>& rows, const QString& text); // 新增（多选/全选批量填充）
};
```

### 列布局（10 列）

| 列 | 标题 | 可编辑 |
| --- | --- | --- |
| 0 | 特征码名 | 否 |
| 1 | 进程 | 否 |
| 2 | PID | 否 |
| 3 | 地址(十进制) | 否（派生：`QString::number(address)`） |
| 4 | 地址 | 否（`0x` + 大写 hex） |
| 5 | 模块 | 否 |
| 6 | 模块基址 | 否 |
| 7 | 匹配字节 | 否 |
| 8 | 命中时间 | 否 |
| 9 | 自定义 | **是** |

## model/types.h（增量）

```cpp
struct ResultRow {
    QString patternName;
    quint32 pid = 0;
    QString processName;
    quint64 address = 0;
    QString moduleName;
    quint64 moduleBase = 0;
    QString matchedBytes;
    QString hitTime;
    QString custom;          // 新增
};
```

## 其它模型

- `SignatureTableModel`、`ProcessTableModel`：不变。

## 约束满足说明

- Model 纯净：仅依赖 Qt 与 `model/types.h`，不依赖 psig；可编辑性由 `flags/setData` 精确控制（仅末列）。
