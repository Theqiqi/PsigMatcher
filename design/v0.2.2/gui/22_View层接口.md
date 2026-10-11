# 22 View 层接口（v0.2.2，增量）

> 前置：`08_窗口层次结构与导航图.md`、`09_Qt_ModelView绑定.md`、`04_GUI需求分析.md`
> 说明：View 只依赖 Qt 与 Model 层；**无 signals/private slots、无 connect**；暴露控件 getter、公开 UI 方法、右键菜单 getter。

## MainWindow（增量）

```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    // 既有 getter 保留（语言菜单、页面切换、日志 appendLog/clearLog、状态/进度等）…

    // 新增：日志开关动作 getter（供 VM connect）
    QAction* toggleLogAction() const;          // 视图菜单「显示/隐藏日志」（checkable）

    // 新增：日志窗口右键菜单 getter（含「清空日志」动作）
    QMenu* logContextMenu() const;             // 内含 m_clearLogAction
    QAction* clearLogAction() const;           // 供 VM connect（右键触发）

    // 既有公开 UI 方法沿用：retranslateUi() / setLanguageChecked / showXxxPage /
    // setPersistentStatus / setTransientMessage / setProgress / appendLog / clearLog /
    // setProcessSelectionLocked
};
```

- 视图菜单移除「清空日志」动作；改为 `m_logEdit->setContextMenuPolicy(Qt::CustomContextMenu)`，右键弹出 `logContextMenu()`。
- `toggleLogAction` 为 `checkable`，勾选态随日志 dock 实际显隐同步（VM 在显隐后更新 `setChecked`）。

## ProcessSelectionPage（增量）

```cpp
class ProcessSelectionPage : public QWidget {
    Q_OBJECT
public:
    QTableView* table() const;
    QLineEdit* filterEdit() const;
    QMenu* contextMenu() const;
    QAction* useAsTargetAction() const;
    QAction* refreshAction() const;
    QAction* selectAllAction() const;
    void setModel(QAbstractItemModel*);
    void setDefaultColumnWidths();             // 新增：Name×2、Accessible=PID、Path 自适应；可后续手动调整
    void retranslateUi();
};
```

- 初始列宽：`horizontalHeader()->setDefaultSectionSize(...)` 配合 `setColumnWidth(PID, w)`、`setColumnWidth(Accessible, w)`、`setColumnWidth(Name, w*2)`、`setSectionResizeMode(Path, Stretch)`；用户仍可拖拽调整。

## ColumnManagerDialog（增量）

```cpp
class ColumnManagerDialog : public QDialog {
    Q_OBJECT
public:
    QComboBox* keyCombo() const;               // 新增：键下拉（来自 psig 内置列键 + 当前已用键）
    QComboBox* typeCombo() const;              // 新增：类型下拉（固定枚举：文本/数字/…）
    QPushButton* addBtn() const;
    QPushButton* removeBtn() const;
    QPushButton* upBtn() const;
    QPushButton* downBtn() const;
    QTableWidget* table() const;
    QDialogButtonBox* buttonBox() const;
    void retranslateUi();
};
```

- 键/类型改为 `QComboBox` 下拉，禁止自由文本输入（C-02），避免键拼写错误与无效键。

## SignatureEditorPage / ResultPage / AboutDialog / ExportColumnsDialog

- 沿用 v0.2.1 接口（`table()`/`contextMenu()`/各 `QAction*` getter/`buttonBox()` 等），无结构性变更；`ResultTableModel` 末列表头文案由 Model 内部 `tr("备注")` 提供。

## 约束满足说明

- View 头文件只依赖 Qt 与 Model 层；无 signals/slots/connect；控件经 getter 暴露；菜单/对话框/右键菜单由 View 创建、由 VM 连接与驱动。
