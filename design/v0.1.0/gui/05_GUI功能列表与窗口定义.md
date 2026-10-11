# 05 GUI 功能列表与窗口定义文档

- 版本：v0.1.0
- 前置：`04_GUI需求分析.md`

## 1. GUI 功能列表

| 功能编号 | 功能 | 所属窗口/页面 | 入口 |
| --- | --- | --- | --- |
| F01 | 新增特征码行 | 特征码编辑页 | 工具栏按钮 / 右键菜单 |
| F02 | 编辑特征码单元格 | 特征码编辑页 | 双击单元格 |
| F03 | 删除选中特征码行 | 特征码编辑页 | 右键菜单 / 工具栏 |
| F04 | 批量删除特征码 | 特征码编辑页 | 多选 + 右键菜单 |
| F05 | 添加列 | 列管理对话框 | 菜单 编辑→列管理 |
| F06 | 重命名列 | 列管理对话框 | 对话框内编辑 |
| F07 | 删除列 | 列管理对话框 | 对话框内删除 |
| F08 | 调整列顺序 | 列管理对话框 | 对话框内上移/下移 |
| F09 | 导入特征码(CSV/DB) | 主窗口 | 菜单 文件→导入 / 工具栏 |
| F10 | 导出特征码(CSV/DB) | 主窗口 | 菜单 文件→导出 / 工具栏 |
| F11 | 刷新进程列表 | 进程选择页 | 按钮 / 菜单 扫描→刷新 |
| F12 | 多选目标进程 | 进程选择页 | Ctrl/Shift 多选、全选 |
| F13 | 开始批量扫描 | 主窗口 | 工具栏按钮 / 菜单 扫描→开始 |
| F14 | 取消扫描 | 主窗口 | 工具栏按钮（扫描中）/ 菜单 |
| F15 | 查看匹配结果 | 匹配结果页 | 页面切换 |
| F16 | 复制匹配结果 | 匹配结果页 | 右键菜单 / Ctrl+C |
| F17 | 导出结果 CSV | 匹配结果页 | 按钮 / 菜单 |
| F18 | 导出结果 DB | 匹配结果页 | 按钮 / 菜单 |
| F19 | 导出结果 TXT | 匹配结果页 | 按钮 / 菜单 |
| F20 | 清空结果 | 匹配结果页 | 右键菜单 / 按钮 |
| F21 | 查看日志 | 日志 Dock | 常驻 |
| F22 | 清空日志 | 日志 Dock | 右键菜单 / 按钮 |

## 2. 窗口/页面定义

### 2.1 MainWindow（QMainWindow）
- 职责：窗口骨架，承载菜单栏、工具栏、QStackedWidget、日志 Dock、状态栏。
- 不含具体业务控件，只持有页面对象指针并提供 getter。

### 2.2 SignatureEditorPage（QWidget，独立类/文件）
- 数据集合：特征码表（可编辑）、列定义。
- 控件：`QTableView* m_table`（可编辑模型）、`QPushButton* m_addRowBtn`、`QPushButton* m_delRowBtn`、`QPushButton* m_columnBtn`。
- 操作：F01–F04，F05–F08（经对话框）。

### 2.3 ProcessSelectionPage（QWidget，独立类/文件）
- 数据集合：进程列表（只读、多选）。
- 控件：`QTableView* m_table`（多选）、`QPushButton* m_refreshBtn`、`QLineEdit* m_filterEdit`（模糊过滤）、`QPushButton* m_selectAllBtn`。
- 操作：F11、F12。

### 2.4 ResultPage（QWidget，独立类/文件）
- 数据集合：匹配结果表（只读）。
- 控件：`QTableView* m_table`、`QPushButton* m_exportCsvBtn`、`m_exportDbBtn`、`m_exportTxtBtn`、`QPushButton* m_clearBtn`。
- 操作：F15–F20。

### 2.5 ColumnManagerDialog（QDialog）
- 数据集合：列定义集。
- 控件：`QTableWidget/QListView`（列列表）、`QPushButton`（添加/删除/上移/下移）、`QLineEdit`（标题）。
- 操作：F05–F08。

## 3. 窗口关系

```
MainWindow
 ├── 菜单栏/工具栏
 ├── QStackedWidget
 │    ├── SignatureEditorPage
 │    ├── ProcessSelectionPage
 │    └── ResultPage
 ├── QDockWidget(日志)
 ├── QStatusBar
 └── (弹出) ColumnManagerDialog, QFileDialog, QMessageBox
```

## 4. 约束满足说明

- 已拆分 GUI 功能列表并定义每个窗口/页面及其控件与操作入口。
