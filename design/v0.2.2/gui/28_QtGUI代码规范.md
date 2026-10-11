# 28 Qt GUI 代码规范（v0.2.2）

> 前置：统一要求、`design/v0.2.1/gui/28_QtGUI代码规范.md`（上一版本）

## 结论：沿用既有规范

- View 只画 UI；**无 signals/slots/connect**；可单独编译；头文件只依赖 Qt 与 Model。
- View 暴露控件 getter、公开 UI 方法、右键菜单/对话框 getter（含 `buttonBox()`/内部按钮 getter/`keyCombo()`/`typeCombo()`/`logContextMenu()`/`toggleLogAction()`）。
- VM 持 `View*`；集中 `connect`（含右键菜单 QAction、**对话框按钮**、语言菜单、表头排序、日志开关/右键、重新选择进程提示）；经 getter 读控件、公开方法更新 UI，不用 findChild。
- Model 纯净（仅 Qt + `model/types.h`），不含业务 I/O。
- `Q_OBJECT` 宏；UI 线程不阻塞（耗时走 QtConcurrent）；后台不直接操作 UI。
- `.qrc` 管理资源；GUI 生产代码禁止平台 API 与 Qt 以外第三方库。
- 头文件包含使用 `名字/名字`，禁止 `../`。

## 本次遵循点（新增/强化约定）

- **国际化**：所有面向用户的字符串必须经 `tr()`（含对话框按钮、菜单、状态/日志模板、列标题、占位符、进程兜底名）。新增：`备注` 替代 `自定义`；`地址(十进制)` 等导出列名统一走翻译。禁止硬编码中文字面量到非 `tr()` 位置。
- **平台能力经 psig**：进程图标必须由 `psig::ProcessIcon`/`DefaultProcessIcon` 提供，GUI 不得 `#include <windows.h>` 或调用 shell32 等。
- **对话框/菜单按钮**：View 仅暴露 getter；`accepted/rejected` 与内部按钮、日志开关、日志右键的 `clicked`/`triggered` 连接一律在 VM。
- **列管理下拉（C-02）**：键/类型只允许下拉选择，禁止 `QLineEdit` 自由输入，避免无效键。
- **权限图标（C-09）**：图标注入仅在 `accessible==true` 时由 VM→Model 完成，View 只经 `DecorationRole` 显示；不阻止无权限进程的选择。
- **配色/着色**：统一由 `res/style.qss` 控制；日志着色在 View 内部实现（不改数据）。
- **构建**：Release 去控制台用 `WIN32_EXECUTABLE`；i18n 用 `Qt6::LinguistTools`。

## 约束满足说明

- 沿用既有 Qt GUI 代码规范，本次改动遵循之，并强化「i18n 全部走 tr」「平台能力经 psig」「列管理下拉」「权限图标仅 accessible」四项约定。
