# 20 GUI 模块分析（v0.3.1，增量）

> 模式：修改型。按 MVVM 4 层（View / Model / ViewModel / 静态库）标注本版改动涉及的层与文件；未涉及模块引用既有文档。

## 一、四层划分与改动

| 层 | 本版改动 | 文件 |
| --- | --- | --- |
| **View** | 新增整行悬停视图子类；改写委托；三页改用子类；进程页禁用角落按钮 | `view/hoverrowtableview.{h,cpp}`（新增）、`view/hoverrowdelegate.{h,cpp}`（改）、`view/processselectionpage.cpp`、`view/signatureeditorpage.cpp`、`view/resultpage.cpp` |
| **Model** | 无改动 | — |
| **ViewModel** | 特征码导出写入目标进程名；导入比对并提示 | `viewmodel/signature.cpp`（改） |
| **静态库（psig）** | `Table::targetProcess` + `.db` 元数据读写 | `psig/include/psig.h`、`psig/src/api/exchange_signatures.cpp` |

## 二、View 层模块

- `HoverRowTableView`（新）：`QTableView` 子类，跟踪悬停行、按行重绘、装配委托。**只画 UI**。
- `HoverRowDelegate`（改）：仅在单元格矩形内绘制悬停底色，清除单格原生 hover。**只画 UI**。
- `ProcessSelectionPage`（改）：表格类型替换 + `setCornerButtonEnabled(false)`。
- `SignatureEditorPage` / `ResultPage`（改）：表格类型替换（悬停修复统一生效）。

## 三、层间依赖

- View → Qt 与 Model 层（不变）。
- ViewModel → View（经公开 getter/方法）+ psig 公共 API（`Table::targetProcess`）。
- Model 不依赖 ViewModel/静态库（不变）。

## 四、约束满足说明

- GUI 分 4 层；View 层只画 UI 可单独编译。
- 修改模式：标注改动涉及的层与文件，未涉及模块（Model、其余 VM 文件、psig 其余层）引用既有文档。
