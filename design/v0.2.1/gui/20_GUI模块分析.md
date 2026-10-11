# 20 GUI 模块分析（v0.2.0，增量）

> 前置：`04_GUI需求分析.md`、`14_API能力需求.md`、`design/v0.1.0/gui/20_GUI模块分析.md`（基线）
> 说明：标注改动涉及的层与文件；未涉及模块引用既有文档。

## 1. 四层结构（不变）

```
View（只画 UI，可单独编译）  →  Model（QAbstractTableModel，纯数据）  →  ViewModel（QObject，持 View*，集中 connect，调 psig）  →  静态库 psig
```

## 2. 改动涉及的层与文件

| 层 | 文件 | 变更 |
| --- | --- | --- |
| View | `view/mainwindow.h/.cpp` | 工具栏图标化 + 按页显隐 + 启动流程 + 视图菜单显隐 + 关于动作 |
| View | `view/processselectionpage.h/.cpp` | 移除按钮；确认选择入口；右键菜单 |
| View | `view/signatureeditorpage.h/.cpp` | 移除按钮条；右键菜单 getter |
| View | `view/resultpage.h/.cpp` | 模板文本区；右键菜单 getter；末列可编辑；十进制地址列 |
| View | `view/aboutdialog.h/.cpp` | **新增**：关于对话框 |
| View | `view/exportcolumnsdialog.h/.cpp` | **新增**：选择导出列对话框 |
| Model | `model/types.h` | `ResultRow` + `custom` |
| Model | `model/resulttablemodel.h/.cpp` | 10 列；末列可编辑；`removeRows`；`setCustom` |
| ViewModel | `viewmodel/mainviewmodel.h/.cpp` | 新槽（确认/重选进程、右键菜单各动作、模板文本、选列、填充自定义列、关于）；导入前置询问；结果导出携带模板/自定义列/选列 |
| 静态库 | `psig` | 已由静态库流程扩展（v0.2.0） |

## 3. 未涉及模块（引用 v0.1.0）

- `model/signaturetablemodel.*`（列/行逻辑不变）、`model/processtablemodel.*`、`view/columnmanagerdialog.*`、`main.cpp`。

## 约束满足说明

- GUI 分为 View/Model/ViewModel/静态库 4 层；View 只画 UI 可单独编译。
- 已标注改动涉及的层与文件；未涉及模块引用既有文档。
