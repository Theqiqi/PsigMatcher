# 22 View 层接口（v0.2.1，增量）

> 前置：`08_窗口层次结构与导航图.md`、`09_Qt_ModelView绑定.md`、`04_GUI需求分析.md`
> 说明：View 只依赖 Qt 与 Model 层；**无 signals/private slots、无 connect**；暴露控件 getter、公开 UI 方法、右键菜单 getter。

## MainWindow（增量）

```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    // 既有 getter 保留 …

    // 新增：语言菜单动作 getter（供 VM connect）
    QAction* languageZhAction() const;        // 中文
    QAction* languageEnAction() const;        // English
    QAction* languageSystemAction() const;    // 跟随系统
    QActionGroup* languageGroup() const;      // 互斥组

    // 新增公开 UI 方法
    void retranslateUi();                     // 重新设置全部界面文案（切换语言后）
    void setLanguageChecked(const QString& code); // 勾选当前语言（""=跟随系统）
    // 既有：showXxxPage / setPersistentStatus / setTransientMessage / setProgress /
    //       appendLog（内部按级别着色）/ clearLog / setProcessSelectionLocked
};
```

- `appendLog(text)` 内部按内容前缀着色（信息/成功/错误），使用 `QPlainTextEdit` 的 `appendHtml` 或按段落设置字符格式。
- 语言菜单位于菜单栏（「视图」与「帮助」之间），三项互斥（`QActionGroup`），当前项 `checkable` 选中。

## SignatureEditorPage（增量）

```cpp
class SignatureEditorPage : public QWidget {
    Q_OBJECT
public:
    QTableView* table() const;
    QMenu* contextMenu() const;
    QAction* addRowAction() const;            // 新增一行
    QAction* addRowsAction() const;           // 新增多行…（新增）
    QAction* deleteRowsAction() const;        // 删除选中行
    QAction* manageColumnsAction() const;     // 列管理
    void setModel(QAbstractItemModel*);
    void selectRow(int row);
};
```

## ProcessSelectionPage（增量）

```cpp
class ProcessSelectionPage : public QWidget {
    Q_OBJECT
public:
    QTableView* table() const;                // 内部 setSortingEnabled(true)
    QLineEdit* filterEdit() const;            // 移到底部（布局变更）
    QMenu* contextMenu() const;
    QAction* useAsTargetAction() const;
    QAction* refreshAction() const;
    QAction* selectAllAction() const;
    void setModel(QAbstractItemModel*);
};
```

- 布局由上到下：`QTableView` → `QLineEdit`（筛选）。
- 表格 `horizontalHeader()->setSectionsClickable(true)` + `setSortingEnabled(true)`。

## ResultPage（增量）

- 接口不变（沿用 v0.2.0）；配色由全局 `style.qss` 控制（白底）。

## 对话框（增量）

```cpp
// ColumnManagerDialog（既有 getter，本次用于接线）
QPushButton* addBtn() const; QPushButton* removeBtn() const;
QPushButton* upBtn() const;  QPushButton* downBtn() const;
QTableWidget* table() const; QDialogButtonBox* buttonBox() const;

// AboutDialog
QDialogButtonBox* buttonBox() const;

// ExportColumnsDialog
QListWidget* list() const; QDialogButtonBox* buttonBox() const;
```

## 约束满足说明

- View 头文件只依赖 Qt 与 Model 层；无 signals/slots/connect；控件经 getter 暴露；菜单/对话框由 View 创建、由 VM 连接与驱动。
