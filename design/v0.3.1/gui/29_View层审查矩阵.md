# 29 View 层审查矩阵（GUI 流程步骤 31，v0.3.1）

- 版本：v0.3.1
- 前置：`design/v0.3.1/gui/21_职责和边界.md`、`22_View层接口.md`
- 审查对象：`gui/src/view/`（本版改动 `hoverrowtableview.{h,cpp}`（新增）、`hoverrowdelegate.{h,cpp}`（重写）、`processselectionpage.cpp`、`signatureeditorpage.cpp`、`resultpage.cpp`；其余 view 文件未受影响）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行；View 不 include viewmodel/psig

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `hoverrowtableview.h`=26、`hoverrowtableview.cpp`=37、`hoverrowdelegate.h`=18、`hoverrowdelegate.cpp`=29、`processselectionpage.cpp`=67、`signatureeditorpage.cpp`=52、`resultpage.cpp`=67 |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | `HoverRowTableView::mouseMoveEvent`（`hoverrowtableview.cpp:15-23`）=9；`leaveEvent`（`:25-31`）=7；`updateRow`（`:33-37`）=5；`HoverRowDelegate::initStyleOption`（`hoverrowdelegate.cpp:18-29`）=12；`ProcessSelectionPage` 构造（`processselectionpage.cpp:15-44`）=30；`setDefaultColumnWidths`（`:48-60`）=13；`SignatureEditorPage` 构造（`signatureeditorpage.cpp:13-37`）=25；`ResultPage` 构造（`resultpage.cpp:14-50`）=37 |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | `mouseMoveEvent`：`if` 1 层（`hoverrowtableview.cpp:17-21`）；`updateRow`：`if` 1 层（`:34`）；`initStyleOption`：`if` 1 层（`hoverrowdelegate.cpp:25-28`）；`selectRow`：`if` 1 层（`signatureeditorpage.cpp:42`） |
| 单一职责 | 一个函数只做一件事 | ✅ | `mouseMoveEvent`=跟踪悬停行并重绘；`updateRow`=重绘单行；`initStyleOption`=设定悬停底色；构造=建控件/布局/样式 |
| 错误后提前 return | 非法状态立即返回 | ✅ | `updateRow`：`row<0` 或无 model 提前 `return`（`hoverrowtableview.cpp:34`）；`selectRow`：越界提前 `return`（`signatureeditorpage.cpp:42`） |
| 头文件越界 | View 不 include viewmodel/psig | ✅ | `grep -n '#include' hoverrowtableview.h` → 仅 `<QTableView>`；`hoverrowdelegate.h` → 仅 `<QStyledItemDelegate>`；各页头 → 仅 `<QWidget>`；`grep -rn 'viewmodel\|psig' gui/src/view/*.h` → 仅 `columnmanagerdialog.h:29` 一处**注释**命中，无真实依赖 |
| 只创建控件/布局/样式 | 不含事件处理逻辑 | ✅ | 本版 view 改动为：控件类型替换（`HoverRowTableView`）、`setCornerButtonEnabled(false)`（`processselectionpage.cpp:36`）、悬停行重绘逻辑（视图自身行为，仅操作 viewport 重绘，不触碰 Model） |
| 无 signals/private slots | View 不声明 signals、无 private slots | ✅ | `grep -n 'signals\|private slots\|Q_SIGNALS\|emit ' hoverrowtableview.h hoverrowdelegate.h` → 无命中 |
| 页面独立成文件 | 每页独立文件、mainwindow.cpp 只做骨架 | ✅ | `HoverRowTableView`/`HoverRowDelegate` 独立成对文件；`processselectionpage`/`signatureeditorpage`/`resultpage` 各自独立 |
| 公开方法暴露 | 经公开方法更新 UI | ✅ | `hoveredRow()` 为公开只读方法（`hoverrowtableview.h:17`）；页面 `setModel`/`selectRow` 为公开方法，VM 经 `m_view->` 调用 |

## 其余 view 文件

- `mainwindow.{h,cpp}`/`columnmanagerdialog`/`columnexportdialog`/各对话框：**未受影响，跳过**（本版 diff 未触及）。

## 结论

View 层全部检查项通过（无 ❌），无需回退。C-01/C-02/C-03/C-04 均在 View 层闭环：委托只画自身矩形（不越界）+ 视图驱动整行重绘 + 禁用角落按钮。
