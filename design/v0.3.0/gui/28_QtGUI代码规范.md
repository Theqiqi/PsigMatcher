# 28 Qt GUI 代码规范（v0.3.0）

> 前置：统一要求、`design/v0.2.4/gui/28_QtGUI代码规范.md`（上一版本）

## 结论：沿用既有规范

沿用 `design/v0.2.4/gui/28_QtGUI代码规范.md` 全部约定（含「单类多文件拆分命名」规则），本版无新增/修改规范。

## 沿用要点（与本次改动相关）

- View 只画 UI、可单独编译、无 signals/slots/connect；ViewModel 持 `View*` 并集中 `connect`；Model 纯净无 I/O；参与信号槽的类含 `Q_OBJECT`。
- UI 线程不阻塞（耗时走 `QtConcurrent`）；**后台不直接操作 UI**。
- `.qrc` 管理资源；GUI 生产代码禁止平台 API 与 Qt 以外第三方库。
- `#include` 只用 `名字/名字.h` 形式，禁止 `../`。
- 文件命名：每个类一对 `.h/.cpp`，文件名与类名一致（类名小写）。

## 本次改动的规范符合点

- 进度回调在**后台工作线程**触发，只写 `std::atomic`，不触碰 Qt 对象（符合「后台不直接操作 UI」）。
- 界面刷新统一由主线程 `QTimer`（`onProgressTick`）执行（符合「UI 线程刷新」）。
- VM 不创建线程/线程池；并行由 psig（静态库）承担（符合「GUI 生产代码禁止平台 API/第三方库」）。
- 本版未新增文件，未改变分层与命名。

## 约束满足说明

- View 只画 UI；View 可单独编译；VM 持 View 指针链接事件；Model 纯净；`Q_OBJECT` 宏；UI 线程不阻塞；后台不直接操作 UI；`.qrc` 管理资源；GUI 生产代码禁止平台 API/第三方库。
