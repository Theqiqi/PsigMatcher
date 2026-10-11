# 22 View 层接口文档

- 版本：v0.1.0
- 前置：`08_窗口层次结构与导航图.md`

> 风格：控件 getter + 公开 UI 方法。View 不声明用户交互信号、无 private slots；按钮点击等由 ViewModel 直接 connect 控件信号处理。

## MainWindow（QMainWindow）

```cpp
class MainWindow : public QMainWindow {
public:
    // 页面 getter
    SignatureEditorPage* signaturePage() const;
    ProcessSelectionPage* processPage() const;
    ResultPage* resultPage() const;

    // 全局动作 getter
    QAction* importSignaturesAction() const;
    QAction* exportSignaturesAction() const;
    QAction* refreshProcessAction() const;
    QAction* startScanAction() const;
    QAction* cancelScanAction() const;
    QAction* manageColumnsAction() const;

    // 状态/日志控件 getter
    QLabel* persistentLabel() const;      // 持久状态区（左）
    QLabel* transientLabel() const;       // 瞬时反馈区（右）
    QProgressBar* progressBar() const;
    QPlainTextEdit* logEdit() const;

    // 公开 UI 方法
    void showSignaturePage();
    void showProcessPage();
    void showResultPage();
    void setPersistentStatus(const QString& text);
    void setTransientMessage(const QString& text, bool error = false);
    void setProgress(int value);          // 0..100，负值隐藏
    void appendLog(const QString& text);
    void clearLog();
};
```

## SignatureEditorPage（QWidget）

```cpp
class SignatureEditorPage : public QWidget {
public:
    QTableView* table() const;
    QPushButton* addRowBtn() const;
    QPushButton* deleteRowBtn() const;
    QPushButton* manageColumnsBtn() const;
    void setModel(QAbstractItemModel* model);
    void selectRow(int row);
};
```

## ProcessSelectionPage（QWidget）

```cpp
class ProcessSelectionPage : public QWidget {
public:
    QTableView* table() const;
    QPushButton* refreshBtn() const;
    QPushButton* selectAllBtn() const;
    QLineEdit* filterEdit() const;
    void setModel(QAbstractItemModel* model);
};
```

## ResultPage（QWidget）

```cpp
class ResultPage : public QWidget {
public:
    QTableView* table() const;
    QPushButton* exportCsvBtn() const;
    QPushButton* exportDbBtn() const;
    QPushButton* exportTxtBtn() const;
    QPushButton* clearBtn() const;
    void setModel(QAbstractItemModel* model);
};
```

## ColumnManagerDialog（QDialog）

> 列定义使用 **Model 层**的 `ColumnDef`（`model/columndef.h`），不使用 `psig::ColumnDef`，避免 View 依赖静态库头文件；由 ViewModel 负责两类型转换。

```cpp
class ColumnManagerDialog : public QDialog {
public:
    QPushButton* addBtn() const;
    QPushButton* removeBtn() const;
    QPushButton* upBtn() const;
    QPushButton* downBtn() const;
    QTableWidget* table() const;
    QDialogButtonBox* buttonBox() const;
    void setColumns(const std::vector<ColumnDef>& cols); // ColumnDef = model 层类型
    std::vector<ColumnDef> columns() const;
};
```

## 约束满足说明

- View 头文件只依赖 Qt 与 Model 层（含 `model/columndef.h`）；不依赖 ViewModel/静态库（可单独编译）。
- View 通过公开 getter 暴露控件，通过公开 UI 方法更新界面；不声明交互信号、无 private slots。
