# 20 GUI 模块分析（v0.3.2，增量）

- 版本：v0.3.2
- 模式：修改型。按 MVVM 4 层架构标注本版**改动涉及的层与文件**；未涉及模块引用既有文档（`design/v0.3.1/gui/20_GUI模块分析.md`）。

## 一、4 层归属与本版改动

| 层 | 文件 | 本版改动 | 改动点 |
| --- | --- | --- | --- |
| **Model** | `model/signaturetablemodel.{h,cpp}` | **无改动**（`setData` 已发 `dataChanged`，本版由 ViewModel 连接） | C-01（消费方在 VM） |
| | `model/processtablemodel.*` / `resulttablemodel.*` / `types.h` | 无 | — |
| **View** | `view/mainwindow.{h,cpp}` | 标题版本号；**新增公开方法 `quitAction()` 已在**（既有 getter）；无事件逻辑 | C-03（仅暴露 getter）、C-04 |
| | `view/signatureeditorpage.*` / 其余页面与对话框 | 无 | — |
| **ViewModel** | `viewmodel/bind.cpp` | **新增** `dataChanged → markSignatureDirty` 连接；**新增** `quitAction → close` 连接 | C-01、C-03 |
| | `viewmodel/signature.cpp` | **修改** `onImportSignatures`（覆盖确认，去掉保存）；`askSaveSignatureChanges`（重命名+参数化） | C-02 |
| | `viewmodel/process.cpp` | **修改** `onReselectProcess` 调用改名后的 `askSaveSignatureChanges(false)` | C-01/C-02（沿用语义） |
| | `viewmodel/mainviewmodel.{h,cpp}` | **新增** `eventFilter`（关闭拦截）+ `askSaveSignatureChanges` 声明 + `isSignatureDirty()`（测试可见）+ `markSignatureDirty` 触发状态刷新 | C-01、C-03 |
| | `viewmodel/language.cpp` | **修改** 翻译资源装载路径（`PsigMatcher_`） | C-04 |
| | `viewmodel/viewmodel_helpers.*` / `scan.cpp` / `result.cpp` | 无 | — |
| **静态库** | `psig` | **无改动** | — |

## 二、依赖方向（沿用）

```
View（Qt+Model） ← ViewModel（持 View*/Model*，调静态库） → 静态库 psig（公共头）
```

- 本版未新增跨层依赖；`eventFilter` 由 ViewModel 安装到 View（`m_view->installEventFilter(this)`），方向仍为 VM → View。

## 三、构建/命名模块（C-04）

| 文件 | 改动 |
| --- | --- |
| `gui/CMakeLists.txt` | `project(PsigMatcher)`；i18n 源文件名与 `gui_embed_i18n(PsigMatcher …)` |
| `gui/src/CMakeLists.txt` | `qt_add_executable(PsigMatcher …)` 及该目标的全部 `target_*` |
| `gui/src/main.cpp` | `setApplicationName("PsigMatcher")` |
| `gui/res/i18n/PsigMatcher_{zh_CN,en_US}.ts` | 由 `psig_gui_*` 重命名（内容本版有新增条目） |
| `gui/res/style.qss` | 注释改名 |
| `gui/test/target/*`、`gui/script/verify/*`、`gui/test/e2e/*` | 引用名同步 |

## 四、约束满足说明

- GUI 分为 View/Model/ViewModel/静态库 4 层；View 层仍只画 UI、可单独编译。
- 修改模式：已标注改动涉及的层与文件；未涉及模块引用既有文档。
