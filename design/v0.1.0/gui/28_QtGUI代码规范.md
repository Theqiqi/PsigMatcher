# 28 Qt GUI 代码规范文档

- 版本：v0.1.0
- 前置：统一要求

## 分层规范

- **View 只画 UI**：View 的 .cpp 只包含 UI 创建、布局、信号发射（无业务）；不 include ViewModel/静态库头文件；不声明 signals、无 private slots、无事件处理。
- **View 可单独编译**：View 头文件只依赖 Qt 与 Model 层。
- **ViewModel 持 View 指针**：经 View 公开 getter 读控件、公开 UI 方法更新界面，不直接操作 View 内部控件、不 findChild。
- **ViewModel 链接事件**：在 `install()` 中集中 `connect(View控件信号, this, &VM::槽)`。
- **Model 纯净**：只管理数据，不含业务逻辑、不做 I/O。
- **Q_OBJECT**：所有参与信号/槽的类必须继承 QObject 并含 Q_OBJECT 宏。

## 线程规范

- **UI 线程绝不阻塞**：`app.exec()` 事件循环中不得调用 `QThread::wait()`。
- **后台线程不直接操作 UI**：后台线程不得直接操作 QWidget；必须经 `Qt::QueuedConnection` 或 `QMetaObject::invokeMethod` 回主线程。
- 耗时操作经 `QtConcurrent::run` + `QFutureWatcher`。

## 资源规范

- 图标与样式表使用 `.qrc`（Qt 资源系统），路径格式 `/prefix/file`。

## 依赖规范

- GUI 生产代码禁止平台相关 API（windows.h 等）与 Qt 以外的第三方库；只允许 Qt 框架、psig 公共 API、C/C++ 标准库。
- 静态库调用仅经 `#include "psig.h"`（仅在 ViewModel 层）。

## 头文件规范

- `#include` 只用 `名字/名字.h` 形式，禁止 `../` 相对路径；搜索路径由 CMake `target_include_directories` 提供。
- 每个类一对 .h/.cpp，文件名与类名一致。

## main 规范

- main.cpp 只创建对象 + `app.exec()`，不含业务。

## 约束满足说明

- 覆盖 View/VM/Model 规范、线程、资源、依赖、头文件包含风格、main 职责。
