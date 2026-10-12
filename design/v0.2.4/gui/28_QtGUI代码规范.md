# 28 Qt GUI 代码规范（v0.2.4）

> 前置：统一要求、`design/v0.2.3/gui/28_QtGUI代码规范.md`（上一版本）

## 结论：沿用既有规范

沿用 `design/v0.2.3/gui/28_QtGUI代码规范.md` 全部约定（含「单类多文件拆分命名」规则），本版无新增/修改规范。

## 沿用要点（与本次改动相关）

- View 只画 UI、可单独编译、无 signals/slots/connect；ViewModel 持 `View*` 并集中 `connect`；Model 纯净无 I/O；参与信号槽的类含 `Q_OBJECT`。
- UI 线程不阻塞（耗时走 `QtConcurrent`）；后台不直接操作 UI。
- `.qrc` 管理资源；GUI 生产代码禁止平台 API 与 Qt 以外第三方库。
- `#include` 只用 `名字/名字.h` 形式，禁止 `../`。
- 文件命名：每个类一对 `.h/.cpp`，文件名与类名一致（类名小写）。

## 本次改动的规范符合点

- 新增 `view/hoverrowdelegate.{h,cpp}` ↔ `HoverRowDelegate`（一类一文件，符合命名规则）。
- 结果导出表头/摘要本地化统一走 `tr()`（`QObject::tr` 用于辅助自由函数，保留 `<name>QObject</name>` 翻译上下文）。
- View 不写 connect：三张表的 `setItemDelegate` 在 View 构造中完成（非 connect）。

## 约束满足说明

- View 只画 UI；View 可单独编译；VM 持 View 指针链接事件；Model 纯净；`Q_OBJECT` 宏；UI 线程不阻塞；后台不直接操作 UI；`.qrc` 管理资源；GUI 生产代码禁止平台 API/第三方库。
