# 22 View 层接口（v0.2.0，增量）

> 前置：`08_窗口层次结构与导航图.md`、`09_Qt_ModelView绑定.md`、`04_GUI需求分析.md`
> 说明：View 只依赖 Qt 与 Model 层；**无 signals/private slots、无 connect**；暴露控件 getter、公开 UI 方法、右键菜单 getter。

## MainWindow（增量）

```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    // 既有 getter 保留 …
    // 新增动作 getter
    QAction* confirmProcessAction() const;    // 确认选择（进入特征码页）
    QAction* reselectProcessAction() const;   // 重新选择进程
    QAction* clearResultsAction() const;      // 清空结果（结果页工具栏）

    // 新增公开 UI 方法
    void setProcessSelectionLocked(bool locked); // 隐藏进程页入口/进程动作，显示「重新选择进程」
    // showSignaturePage/showProcessPage/showResultPage 内部按页显隐工具栏（applyToolbarForPage）
};
```

- 工具栏使用 `QStyle::standardIcon(...)` + `setToolButtonStyle(Qt::ToolButtonIconOnly)`；每个动作 `setToolTip`/`setText` 保留文字。

## SignatureEditorPage（增量）

```cpp
class SignatureEditorPage : public QWidget {
    Q_OBJECT
public:
    QTableView* table() const;
    QMenu* contextMenu() const;                 // 表格右键菜单
    QAction* addRowAction() const;              // 新增行
    QAction* deleteRowsAction() const;          // 删除选中行
    QAction* manageColumnsAction() const;       // 列管理
    void setModel(QAbstractItemModel*);
    void selectRow(int row);
};
```

- **移除** `addRowBtn()/deleteRowBtn()/manageColumnsBtn()` 与页面按钮条。
- 表格 `setContextMenuPolicy(Qt::CustomContextMenu)`；由 VM 连接 `customContextMenuRequested` 弹出 `contextMenu()`。

## ProcessSelectionPage（增量）

```cpp
class ProcessSelectionPage : public QWidget {
    Q_OBJECT
public:
    QTableView* table() const;
    QLineEdit* filterEdit() const;
    QMenu* contextMenu() const;
    QAction* useAsTargetAction() const;   // 以此进程为目标（确认）
    QAction* refreshAction() const;       // 刷新
    QAction* selectAllAction() const;     // 全选
    void setModel(QAbstractItemModel*);
};
```

- **移除** `refreshBtn()/selectAllBtn()` 与页面按钮。
- 双击行：VM 连接 `table()` 的 `doubleClicked`。

## ResultPage（增量）

```cpp
class ResultPage : public QWidget {
    Q_OBJECT
public:
    QTableView* table() const;
    QPlainTextEdit* templateEdit() const;     // 多行模板文本区
    QMenu* contextMenu() const;
    QAction* exportCsvAction() const;
    QAction* exportDbAction() const;
    QAction* exportTxtAction() const;         // 上述三项置于 contextMenu 的「导出」子菜单
    QAction* selectColumnsAction() const;     // 选择导出列…
    QAction* deleteRowsAction() const;        // 删除选中行
    QAction* fillCustomAction() const;        // 批量填充自定义列
    QAction* clearAction() const;             // 清空结果
    void setModel(QAbstractItemModel*);
    void setTemplateText(const QString&);     // 回填模板（导入时）
    QString templateText() const;
};
```

- **移除** `exportCsvBtn()/exportDbBtn()/exportTxtBtn()/clearBtn()`。

## AboutDialog（新增）

```cpp
class AboutDialog : public QDialog {
    Q_OBJECT
public:
    explicit AboutDialog(QWidget* parent = nullptr);
    void setInfo(const QString& text);
    QString info() const;                      // 供测试读取
    QDialogButtonBox* buttonBox() const;
};
```

## ExportColumnsDialog（新增）

```cpp
class ExportColumnsDialog : public QDialog {
    Q_OBJECT
public:
    explicit ExportColumnsDialog(QWidget* parent = nullptr);
    // cols: (key,title) 有序；selectedKeys: 预勾选
    void setColumns(const std::vector<std::pair<QString, QString>>& cols, const QStringList& selectedKeys);
    QStringList selectedKeys() const;
    QListWidget* list() const;
    QDialogButtonBox* buttonBox() const;
};
```

## 约束满足说明

- View 头文件只依赖 Qt 与 Model 层；无 signals/slots/connect；控件经 getter 暴露；菜单/对话框由 View 创建、由 VM 连接与驱动。
