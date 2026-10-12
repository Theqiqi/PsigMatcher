# 22 View 层接口（v0.3.3，增量）

- 版本：v0.3.3
- 前置：`08_窗口层次结构与导航图.md`、`09_Qt_ModelView绑定.md`、`04_GUI需求分析.md`、`21_职责和边界.md`
- 说明：View 只依赖 Qt 与 Model 层；**无 signals/private slots、无 connect**；经公开 getter / 公开 UI 方法暴露。

## 一、本版 View 接口变化：**无新增/删除公开接口**

| 项 | 结论 |
| --- | --- |
| 新增 getter | ❌ 无 |
| 新增公开 UI 方法 | ❌ 无 |
| 新增 signals / private slots | ❌ 无 |
| 删除/改签名 | ❌ 无 |
| 唯一改动 | `MainWindow::retranslateUi()` 内标题字符串 `… v0.3.2` → `… v0.3.3`（`gui/src/view/mainwindow.cpp`） |

## 二、改动前后对照（View 层）

```cpp
// gui/src/view/mainwindow.cpp
 void MainWindow::retranslateUi() {
-    setWindowTitle(tr("进程特征码批量匹配器 v0.3.2"));
+    setWindowTitle(tr("进程特征码批量匹配器 v0.3.3"));
     m_fileMenu->setTitle(tr("文件(&F)"));
     ...
```

- 头文件 `mainwindow.h`：**不变**（既有 getter 集合不变；关闭裁决仍在 ViewModel，本版不新增事件相关声明）。

## 三、图标为何**不**进 View 接口（边界论证）

| 候选做法 | 结论 | 理由 |
| --- | --- | --- |
| 在 `MainWindow` 内 `setWindowIcon(...)` | ❌ 不采用 | 图标是**应用级**属性（`QApplication`），放在 View 会造成「每个窗口各自设图标」的错误职责；且 `main.cpp` 已是图标装配的唯一入口 |
| 新增 `MainWindow::setAppIcon()` 供 ViewModel 调用 | ❌ 不采用 | 无运行期变更图标的需求（R-02 只在启动期设定一次）；新增接口即为无用耦合 |
| 在 ViewModel 中装配 `QIcon` | ❌ 不采用 | 图标与业务/数据无关，ViewModel 不得持有外壳资源职责（见 `21_职责和边界.md`） |
| **在 `main.cpp`（入口）装配并 `app.setWindowIcon`** | ✅ **采用** | 入口职责即「创建对象 + 配置应用 + install + show + exec」，`setWindowIcon` 属应用配置；与既有 `setStyleSheet`/`setApplicationName` 同一位置、同一粒度 |

## 四、View 层公开接口清单（本版复核，无变化）

- 页面 getter：`signaturePage()` / `processPage()` / `resultPage()`
- 全局动作 getter：`importSignaturesAction()` / `exportSignaturesAction()` / `refreshProcessAction()` / `startScanAction()` / `cancelScanAction()` / `manageColumnsAction()` / `addSignatureRowAction()` / `addSignatureRowsAction()` / `confirmProcessAction()` / `reselectProcessAction()` / `clearResultsAction()` / `quitAction()`
- 导航动作 getter：`showSignaturePageAction()` / `showProcessPageAction()` / `showResultPageAction()` / `toggleLogAction()` / `clearLogAction()` / `logContextMenu()` / `aboutAction()`
- 语言动作 getter：`languageZhAction()` / `languageEnAction()` / `languageSystemAction()` / `languageGroup()`
- 状态/日志控件 getter：`persistentLabel()` / `transientLabel()` / `progressBar()` / `logEdit()`
- 公开 UI 方法：`showSignaturePage()` / `showProcessPage()` / `showResultPage()` / `setPersistentStatus()` / `setTransientMessage()` / `setProgress()` / `appendLog()` / `clearLog()` / `logDockVisible()` / `setLogDockVisible()` / `setToggleLogChecked()` / `setProcessSelectionLocked()` / `retranslateUi()` / `setLanguageChecked()`

> 以上与 v0.3.2 逐条一致（`grep -n 'public:' -A200 gui/src/view/mainwindow.h` 复核）。

## 五、约束满足说明

- View 头文件只依赖 Qt 与 Model 层：`mainwindow.h` 仅 `#include <QMainWindow>` + 前置声明 ✅（本版未变）。
- 本版未新增任何 View 公开接口；图标装配归属入口（本文件第三节给出拒绝理由）。
- View 仍可单独编译（沿用 v0.3.2 的 `psig_test_view` 目标，见 `gui/test/unit/CMakeLists.txt`）。
