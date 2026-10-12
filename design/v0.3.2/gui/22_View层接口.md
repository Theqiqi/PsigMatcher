# 22 View 层接口（v0.3.2，增量）

> 前置：`08_窗口层次结构与导航图.md`、`09_Qt_ModelView绑定.md`、`04_GUI需求分析.md`
> 说明：View 只依赖 Qt 与 Model 层；**无 signals/private slots、无 connect、无事件处理逻辑**；暴露控件 getter、公开 UI 方法。

## MainWindow（修改，`view/mainwindow.h/.cpp`）

- **接口不变**：`quitAction()` 等 getter 在 v0.3.1 已存在（本版仅由 ViewModel 连接它）。
- **无新增方法、无新增 getter、无新增 QAction**。
- **不含关闭事件逻辑**：`closeEvent` **不**在 View 中重写；关闭裁决由 ViewModel 事件过滤器完成（见 `24_ViewModel接口.md`）。

| 变更 | 内容 |
| --- | --- |
| `retranslateUi()` | 标题由 `tr("进程特征码批量匹配器 v0.3.1")` → `tr("进程特征码批量匹配器 v0.3.2")` |

- 其余（菜单/工具栏/状态栏/日志 dock 的构建）不变。

## SignatureEditorPage / ProcessSelectionPage / ResultPage / 各对话框

- **无改动**。

## 约束满足说明

- View 头文件只依赖 Qt 与 Model 层；无 signals/无 private slots/无 connect；控件经公开 getter 暴露。
- 本版 View 侧改动仅一处字符串（版本号），保持「只画 UI」边界。
