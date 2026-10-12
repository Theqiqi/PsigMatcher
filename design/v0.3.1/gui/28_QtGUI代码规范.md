# 28 Qt GUI 代码规范（v0.3.1，增量）

> 前置：`design/v0.2.4/gui/28_QtGUI代码规范.md`（上一版本）。

## 结论：沿用既有规范 + 一条补充

沿用 v0.2.4 全部约定（含「单类多文件拆分命名」规则）。本版**新增一条约定**（针对整行悬停绘制）：

> **委托绘制边界**：`QStyledItemDelegate` 子类的绘制必须限制在自身单元格矩形 `opt.rect` 内；**禁止**在委托中横向扩展到整行绘制（会导致覆盖相邻列、破坏选中/文字）。整行视觉（如悬停整行高亮）应由**视图**（`QTableView` 子类）驱动按行重绘，委托对每个单元格只画自己的矩形。

理由：v0.3.0 的 `HoverRowDelegate::paint` 越界 `fillRect` 导致 C-01/C-02/C-03（悬停列变深、覆盖前面列字体、单击整行选中不即时）。

## 沿用要点（与本次改动相关）

- View 只画 UI、可单独编译、无 signals/slots/connect；ViewModel 持 `View*` 并集中 `connect`；Model 纯净无 I/O；参与信号槽的类含 `Q_OBJECT`。
- UI 线程不阻塞；后台不直接操作 UI。
- `.qrc` 管理资源；GUI 生产代码禁止平台 API 与 Qt 以外第三方库。
- `#include` 只用 `名字/名字.h` 形式，禁止 `../`。
- 文件命名：每个类一对 `.h/.cpp`，文件名与类名一致（类名小写）→ `HoverRowTableView` ↔ `view/hoverrowtableview.{h,cpp}`。

## 本次改动的规范符合点

- 新增 `HoverRowTableView` 一类一文件；委托改为 `initStyleOption` 注入（纯绘制）。
- 悬停重绘由视图虚函数驱动，不经信号槽（符合 View 零 connect）。
- C-05 元数据经 psig 公共 API，GUI 不自行读写 `.db`。

## 约束满足说明

- View 只画 UI；View 可单独编译；VM 持 View 指针链接事件；Model 纯净；`Q_OBJECT` 宏；UI 线程不阻塞；后台不直接操作 UI；`.qrc` 管理资源；GUI 生产代码禁止平台 API/第三方库。
