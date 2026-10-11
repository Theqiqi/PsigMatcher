# 28 Qt GUI 代码规范（v0.2.1）

> 前置：统一要求、`design/v0.2.0/gui/28_QtGUI代码规范.md`（基线）

## 结论：沿用既有规范

- View 只画 UI；**无 signals/slots/connect**；可单独编译；头文件只依赖 Qt 与 Model。
- View 暴露控件 getter、公开 UI 方法、右键菜单/对话框 getter（含 `buttonBox()`/内部按钮 getter）。
- VM 持 `View*`；集中 `connect`（含右键菜单 QAction、**对话框按钮**、语言菜单、表头排序）；经 getter 读控件、公开方法更新 UI，不用 findChild。
- Model 纯净（仅 Qt + `model/types.h`），不含业务 I/O。
- `Q_OBJECT` 宏；UI 线程不阻塞（耗时走 QtConcurrent）；后台不直接操作 UI。
- `.qrc` 管理资源；GUI 生产代码禁止平台 API 与 Qt 以外第三方库。
- 头文件包含使用 `名字/名字`，禁止 `../`。

## 本次遵循点（新增约定）

- **国际化**：所有面向用户的字符串必须经 `tr()`（含对话框按钮、菜单、状态/日志模板、列标题、占位符）；新增控件文案随 `retranslateUi()` 刷新；禁止硬编码中文字面量到非 `tr()` 位置。
- **平台能力经 psig**：进程图标必须由 `psig::ProcessIcon`/`DefaultProcessIcon` 提供，GUI 不得 `#include <windows.h>` 或调用 shell32 等。
- **对话框按钮**：View 仅暴露 `buttonBox()`/内部按钮 getter；`accepted/rejected` 与内部按钮的 `clicked` 连接一律在 VM。
- **配色/着色**：统一由 `res/style.qss` 控制；日志着色在 View 内部实现（不改数据）。
- **构建**：Release 去控制台用 `WIN32_EXECUTABLE`；i18n 用 `Qt6::LinguistTools`。

## 约束满足说明

- 沿用既有 Qt GUI 代码规范，本次改动遵循之，并补充 i18n 与「平台能力经 psig」两条约定。
