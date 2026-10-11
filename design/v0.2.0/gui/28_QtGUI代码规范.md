# 28 Qt GUI 代码规范（v0.2.0）

> 前置：统一要求、`design/v0.1.0/gui/28_QtGUI代码规范.md`（基线）

## 结论：沿用既有规范

- View 只画 UI；**无 signals/slots/connect**；可单独编译；头文件只依赖 Qt 与 Model。
- View 暴露控件 getter、公开 UI 方法、右键菜单/对话框 getter。
- VM 持 `View*`；集中 `connect`（含右键菜单 QAction、表格 doubleClicked）；经 getter 读控件、公开方法更新 UI，不用 findChild。
- Model 纯净（仅 Qt + `model/types.h`），不含业务 I/O。
- `Q_OBJECT` 宏；UI 线程不阻塞（耗时走 QtConcurrent）；后台不直接操作 UI（经 `QMetaObject::invokeMethod`/watcher）。
- `.qrc` 管理资源；GUI 生产代码禁止平台 API 与 Qt 以外第三方库。
- 头文件包含使用 `名字/名字`，禁止 `../`。

## 本次遵循点

- 新增对话框（AboutDialog/ExportColumnsDialog）仅画 UI，无 connect。
- 右键菜单的 QAction 在 View 创建、在 VM 连接。
- 模板文本/自定义列/选列的持久化全部经 psig，不在 GUI 内实现文件格式逻辑。

## 约束满足说明

- 沿用既有 Qt GUI 代码规范，本次改动遵循之。
