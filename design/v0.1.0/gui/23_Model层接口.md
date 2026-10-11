# 23 Model 层接口文档

- 版本：v0.1.0
- 前置：`09_Qt_ModelView绑定.md`

> Model 层只管理数据，不含业务逻辑，不做 I/O。

## 数据结构（model/types.h）

```cpp
struct ColumnDef {            // 特征码表列定义（Model 层自有类型，不依赖 psig）
    QString key;
    QString title;
    QString type;             // "text" | "int" | "enum"
    bool builtin = false;
};

struct SignatureRow {         // 特征码行：cells 与 columns 对齐
    QStringList cells;
};

struct ProcessRow {           // 进程行
    quint32 pid = 0;
    QString name;
    QString path;
    bool accessible = false;
};

struct ResultRow {            // 结果行
    QString patternName;
    quint32 pid = 0;
    QString processName;
    quint64 address = 0;
    QString moduleName;
    quint64 moduleBase = 0;
    QString matchedBytes;
    QString hitTime;
};
```

## SignatureTableModel（QAbstractTableModel）

```cpp
class SignatureTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    int rowCount(const QModelIndex&) const override;
    int columnCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex&, int role) const override;
    bool setData(const QModelIndex&, const QVariant&, int role) override;
    QVariant headerData(int, Qt::Orientation, int) const override;
    Qt::ItemFlags flags(const QModelIndex&) const override;

    // 结构方法（供 VM 调用）
    void setColumns(const std::vector<ColumnDef>& cols);
    std::vector<ColumnDef> columns() const;
    void setRows(const std::vector<SignatureRow>& rows);
    std::vector<SignatureRow> rows() const;
    void addRow();
    void removeRows(const std::vector<int>& rows);
    QString valueAt(int row, const QString& key) const; // 按列键取值（供 VM 组装扫描请求）
};
```

## ProcessTableModel（QAbstractTableModel）

```cpp
class ProcessTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    // rowCount/columnCount/data/headerData/flags（只读）
    void setProcesses(const std::vector<ProcessRow>& list);
    std::vector<ProcessRow> processes() const;
    ProcessRow processAt(int row) const;
};
```

## ResultTableModel（QAbstractTableModel）

```cpp
class ResultTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    // rowCount/columnCount/data/headerData/flags（只读）
    void setResults(const std::vector<ResultRow>& list);
    std::vector<ResultRow> results() const;
    void appendResult(const ResultRow& r);
    void clear();
};
```

## 列定义

| Model | 列 |
| --- | --- |
| SignatureTableModel | 动态（内置 name/pattern/note + 用户列） |
| ProcessTableModel | PID / 名称 / 路径 / 可访问 |
| ResultTableModel | 特征码名 / 进程 / PID / 地址 / 模块 / 模块基址 / 匹配字节 / 命中时间 |

## 约束满足说明

- Model 层只管理数据，无业务逻辑；不依赖 psig，不依赖 View/ViewModel。
