# 28 Qt GUI 代码规范（v0.3.2，增量）

- 版本：v0.3.2
- 模式：修改型。沿用既有规范（`design/v0.3.1/gui/28_QtGUI代码规范.md`）。以下为**本版需遵守/新增**的要点。

## 一、沿用（逐条约束）

| 规范 | 要求 |
| --- | --- |
| View 只画 UI | 不调业务、不写 Model、不 connect；**不含事件处理逻辑** |
| View 可单独编译 | 头文件只依赖 Qt 与 `model/` |
| VM 持 View 指针链接事件 | `connect` 集中在 `install()`/`bind*`；经公开 getter 读控件、公开方法更新 UI，不 `findChild` |
| Model 纯净 | 只管理数据；不含业务/格式化逻辑 |
| `Q_OBJECT` 宏 | 所有 QObject 派生类（含 `MainWindow`、`MainViewModel`、各 Model/页面） |
| UI 线程不阻塞 | 耗时操作 `QtConcurrent::run`；主线程不得 `QThread::wait()`（析构路径除外，沿用既有） |
| 后台不直接操作 UI | 经 `QMetaObject::invokeMethod`/信号回主线程 |
| `.qrc` 管理资源 | 图标/样式经 `resources.qrc`；i18n `.qm` 经 CMake 嵌入 `/i18n` |
| GUI 生产代码禁止平台 API/第三方库 | 只允许 Qt6 与静态库公共头 |

## 二、本版新增要点

1. **关闭事件的归属**：关闭裁决**必须**在 ViewModel（`eventFilter`），**不得**在 View 重写 `closeEvent` 或加入事件处理逻辑；Header 越界检查据此执行。
2. **未保存状态的唯一来源**：`m_sigDirty`；禁止用 `rowCount()>0` 等固定条件冒充未保存判定。
3. **确定性键盘入口**：模态对话框按钮设 `setShortcut`（`Ctrl+S` 保存、`Ctrl+D` 不保存、`Esc` 取消），与主窗口「键盘快捷键作为可确定性注入入口」的既有约定一致。
4. **命名**：生产 exe 目标为 `PsigMatcher`；Qt 对象/文件命名沿用既有风格；注释中程序名同步为 `PsigMatcher`。

## 三、约束满足说明

- 规范覆盖 View/VM/Model 边界、Q_OBJECT、线程与资源；本版新增要点与既有分层一致。
