# 20 GUI 模块分析文档（MVVM 4 层）

- 版本：v0.1.0
- 前置：`04_GUI需求分析.md`、`14_API能力需求.md`

## 4 层划分

| 层 | 职责 | 依赖 |
| --- | --- | --- |
| View | 只画 UI（控件、布局、样式），不调业务 | 仅 Qt + Model 头（可单独编译） |
| Model | 纯数据（QAbstractTableModel 子类 + 数据结构） | 仅 Qt |
| ViewModel | 持 View* 指针，connect 控件信号，调静态库 API，更新 Model，操控 View 刷新 | View/Model/静态库 |
| 静态库 | psig 公共 API | 无 Qt |

## View 层模块（gui/src/view/）

| 文件 | 类 | 职责 |
| --- | --- | --- |
| mainwindow.h/.cpp | MainWindow(QMainWindow) | 窗口骨架：菜单栏/工具栏/QStackedWidget/日志 Dock/状态栏 |
| signatureeditorpage.h/.cpp | SignatureEditorPage(QWidget) | 特征码编辑页 |
| processselectionpage.h/.cpp | ProcessSelectionPage(QWidget) | 进程选择页 |
| resultpage.h/.cpp | ResultPage(QWidget) | 匹配结果页 |
| columnmanagerdialog.h/.cpp | ColumnManagerDialog(QDialog) | 列管理对话框 |

## Model 层模块（gui/src/model/）

| 文件 | 类 |
| --- | --- |
| signaturetablemodel.h/.cpp | SignatureTableModel(QAbstractTableModel) |
| processtablemodel.h/.cpp | ProcessTableModel(QAbstractTableModel) |
| resulttablemodel.h/.cpp | ResultTableModel(QAbstractTableModel) |

## ViewModel 层模块（gui/src/viewmodel/）

| 文件 | 类 |
| --- | --- |
| mainviewmodel.h/.cpp | MainViewModel(QObject) |

## 入口

- gui/src/main.cpp：创建 Model → View(setModel) → ViewModel(install(view)) → show → exec。

## 约束满足说明

- GUI 分 4 层；View 只画 UI 且可单独编译；ViewModel 持 View 指针操控 View。
