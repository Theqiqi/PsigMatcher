# 28 Qt GUI 代码规范（v0.2.3）

> 前置：统一要求、`design/v0.2.2/gui/28_QtGUI代码规范.md`（上一版本）、`design/v0.1.0/gui/28_QtGUI代码规范.md`（基准）

## 结论：沿用既有规范 + 新增「单类多文件拆分命名」

沿用既有 Qt GUI 代码规范（分层/线程/资源/依赖/头文件/main），本次**新增**文件命名细则以覆盖「一个类拆分为多个实现文件」的情形。

## 沿用（基准 `design/v0.1.0/gui/28_QtGUI代码规范.md`）

- View 只画 UI、可单独编译、无 signals/slots/connect；ViewModel 持 `View*` 并集中 `connect`；Model 纯净无 I/O；参与信号槽的类含 `Q_OBJECT`。
- UI 线程不阻塞（耗时走 `QtConcurrent`）；后台不直接操作 UI。
- `.qrc` 管理资源；GUI 生产代码禁止平台 API 与 Qt 以外第三方库。
- `#include` 只用 `名字/名字.h` 形式，禁止 `../`；搜索路径由 CMake `target_include_directories` 提供。

## 本次新增约定（R-08）：文件命名

- **基本规则**：每个类一对 `.h/.cpp`，**文件名与类名一致**（本项目取类名小写，如 `mainwindow.{h,cpp}` ↔ `MainWindow`、`processtablemodel.{h,cpp}` ↔ `ProcessTableModel`）。
- **单类多文件拆分**：当单个类的实现过大（如超过文件长度阈值）需拆分为多个实现文件时——
  1. 保留 `<类名小写>.h` 作为**唯一声明**；
  2. 保留 `<类名小写>.cpp` 作为**主实现**（构造/析构/装配等核心）；
  3. 其余实现文件按**功能域**命名：小写、**不带类名前缀**，与 `.h` 同目录，共享同一声明（如 `bind.cpp` / `process.cpp` / `signature.cpp`）。
- **禁止** `<类名小写>_<域>.cpp` 这类带类名前缀的下划线命名（与「文件名与类名一致」冲突）。
- 非类文件（辅助命名空间/自由函数集合）按其职责命名，如 `viewmodel_helpers.{h,cpp}` 提供 `vmhelp::` 纯函数。

## 约束满足说明

- 覆盖 View/VM/Model 规范、线程、资源、依赖、头文件包含风格、main 职责；并新增单类多文件拆分的命名规则。
- 本版 VM 拆分（R-03）后按本规则命名：`mainviewmodel.{h,cpp}` + `bind/process/signature/scan/result/language.cpp`。
