# 31 ViewModel 层审查矩阵（GUI 流程步骤 33，v0.3.2）

- 版本：v0.3.2
- 前置：`design/v0.3.2/gui/21_职责和边界.md`、`24_ViewModel接口.md`
- 审查对象（本版改动）：`viewmodel/mainviewmodel.h`、`mainviewmodel.cpp`、`bind.cpp`、`process.cpp`、`signature.cpp`、`language.cpp`
- 量化标准：嵌套 ≤3（控制流块层数）；函数体 ≤80 行；单文件 ≤400 行；VM 不 include `view/*.cpp`

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `mainviewmodel.h`=137、`mainviewmodel.cpp`=127、`bind.cpp`=135、`process.cpp`=161、`signature.cpp`=196、`language.cpp`=50（`wc -l`） |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | 本版新增/修改函数：`install`（`mainviewmodel.cpp:30-77`，48）→ 仅新增 1 行 installEventFilter；`refreshPersistentStatus`（`:84-96`，13）；`markSignatureDirty`（`:98-102`，5）；`eventFilter`（`:104-113`，10）；`bindSignaturePage`（`bind.cpp:18-29`，12）；`bindGlobalActions`（`bind.cpp:60-85`，26）；`onImportSignatures`（`signature.cpp:73-92`，20）；`importSignaturesFromPath`（`:94-124`，31）；`exportSignaturesToPath`（`:144-159`，16）；`onReselectProcess`（`process.cpp:130-140`，11）；`askSaveSignatureChanges`（`:142-160`，19） |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | `onImportSignatures`：`if(m_sigDirty)`(1) → `if(clickedButton != continueBtn)`(2)（`signature.cpp:75,82`）；`importSignaturesFromPath`：`if`(1) → `if`(2)（`:109,110`）；`eventFilter`：`if`(1) → `if`(2)（`mainviewmodel.cpp:105,106`）；`askSaveSignatureChanges`：`if`(1) → `if`(2)（`process.cpp:153,154`）。均 ≤3 |
| 单一职责 | 一个函数只做一件事 | ✅ | `markSignatureDirty`=置脏+刷新状态；`eventFilter`=只判关闭；`askSaveSignatureChanges`=只弹保存询问；`onImportSignatures`=覆盖确认+选文件+导入 |
| 错误后提前 return | 非法状态立即返回 | ✅ | `eventFilter` 首行 `watched==m_view && event && type==Close` 才处理（`mainviewmodel.cpp:105`）；`onImportSignatures` 取消即 `return`（`signature.cpp:85`）；`askSaveSignatureChanges` 取消即 `return false`（`process.cpp:153`） |
| 头文件越界 | VM 不 include `view/*.cpp` 内部实现 | ✅ | `grep -rn 'include "view/.*\.cpp"' gui/src/viewmodel/` → **无命中**（仅 include `view/*.h`） |
| 持 View*/Model* 经 getter 读控件、不 findChild | — | ✅ | `grep -rn 'findChild' gui/src/viewmodel/` → **无命中**；新增代码经 `m_view->quitAction()`/`m_view->persistentLabel()` 等公开 getter |
| connect 集中在 install()/bind* | — | ✅ | 本版新增 2 个 connect 均在 `bind.cpp`：`dataChanged→markSignatureDirty`（`bind.cpp:20-21`）、`quitAction→close`（`bind.cpp:73`）；`bind.cpp` 共 43 处 `connect`（全部在 `bind*`/`install` 体系内）。`signature.cpp` 的 4 处 connect 为**既有**对话框按钮接线（v0.3.1 沿用，非本版新增） |
| 耗时操作 QtConcurrent / 主线程无 wait | — | ✅ | 本版无新增耗时路径（弹窗/文件对话框均在主线程同步）；`grep -n 'QThread::wait' gui/src/viewmodel/` → **无命中**（析构用 `waitForFinished`，沿用 v0.3.1） |
| 后台异常经信号回主线程 | — | ✅ | 本版无新增后台任务（沿用 `QFutureWatcher` + `QMetaObject::invokeMethod`） |
| QObject 生命周期正确 | — | ✅ | `eventFilter` 的过滤器 `this`（VM）在 `install()` 安装；Qt 在 QObject 析构时自动移除过滤器（`main()` 中 VM 先于 View 析构） |
| 指针 nullptr 保护 | — | ✅ | `eventFilter` 判 `watched == m_view && event`；`askSaveSignatureChanges` 经 `m_view`（`install` 首行 `if (!view) return;` 保证非空）；`clickedButton()` 与 `continueBtn` 比较（未命中即视为取消） |
| parent-child 正确无泄漏 | — | ✅ | 弹窗 `QMessageBox box(m_view)` 栈对象 + 父窗口；watcher/timer 以 `this` 为 parent（沿用） |
| 新增状态量可见性 | 唯一来源、可测试 | ✅ | `m_sigDirty` 为唯一未保存来源；`isSignatureDirty()` 公开只读（`mainviewmodel.h:47`） |

## 关键行为复核（本版语义）

| 行为 | 实现位置 | 结论 |
| --- | --- | --- |
| 导入与保存解耦 | `onImportSignatures`（`signature.cpp:73-92`） | ✅ 仅覆盖确认 [继续导入/取消]；`grep -n 'getSaveFileName' signature.cpp` → 仅在 `exportSignaturesToPath` 调用方（`onExportSignatures`）与 `askSaveSignatureChanges` 中，**不在**导入路径 |
| 关闭裁决在 VM | `eventFilter`（`mainviewmodel.cpp:104-113`）+ `installEventFilter`（`:44`） | ✅ View 无 closeEvent |
| 「退出」动作真正生效 | `bindGlobalActions`（`bind.cpp:73`） | ✅ `quitAction::triggered → m_view, &QWidget::close` |
| 保存提示唯一实现 | `askSaveSignatureChanges`（`process.cpp:142-160`） | ✅ 关闭（`true`）与重选进程（`false`）共用；`grep -rn 'askSaveBeforeReselect' gui/src` → 无命中（已重命名） |

## 结论

ViewModel 层全部检查项通过（无 ❌）；C-01/C-02/C-03 均在 VM 层闭环，未越界到 View，未越界调用静态库私有实现。
