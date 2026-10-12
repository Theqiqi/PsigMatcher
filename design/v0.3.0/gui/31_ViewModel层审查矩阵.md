# 31 ViewModel 层审查矩阵（GUI 流程步骤 33，v0.3.0）

- 版本：v0.3.0
- 前置：`design/v0.3.0/gui/21_职责和边界.md`、`24_ViewModel接口.md`
- 审查对象：`gui/src/viewmodel/`（本版 `scan.cpp`、`mainviewmodel.{h,cpp}` 改动；其余 VM 文件未受影响）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `scan.cpp`=106、`mainviewmodel.h`=128、`mainviewmodel.cpp`=104 |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | `onStartScan`（`scan.cpp:9-56`）=48；`onProgressTick`（`58-66`）=9；`onScanFinished`（`74-87`）=14；`collectPatterns`（`89-106`）=18 |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | `onStartScan`：`if`×3（并列，各 1 层）；lambda 体独立 1 层 |
| 单一职责 | 一个函数只做一件事 | ✅ | `onStartScan`=启动；`onProgressTick`=采样刷新；`onScanFinished`=收尾 |
| 错误后提前 return | 非法状态立即返回 | ✅ | `onStartScan`：err/empty/pids 空均提前 `return`（`scan.cpp:12-25`） |
| 头文件越界 | VM 不 include view/*.cpp 内部实现 | ✅ | `grep -n '#include' scan.cpp` → `mainviewmodel.h`/`QtConcurrent`/`view/mainwindow.h`/`viewmodel_helpers.h`（经公开头） |
| 持 View*/Model* 经 getter | 不 findChild | ✅ | `grep -rn "findChild" gui/src/viewmodel` → 无命中 |
| connect 集中 | install()/bind* 集中 | ✅ | `QTimer::timeout → onProgressTick` 在 `mainviewmodel.cpp` install()（`:46-49`） |
| 耗时操作异步 | QtConcurrent::run，主线程无 wait | ✅ | `scan.cpp:51-54` `QtConcurrent::run(ScanProcesses...)`；主线程无 `QThread::wait()` |
| 后台不操作 UI | 回调只写原子变量 | ✅ | `scan.cpp:39-45` 回调仅写 `m_progDone/Total/Hits/Pid`；无 `invokeMethod` |
| 后台异常经信号回主线程 | 不跨线程抛异常 | ✅ | 结果经 `QFutureWatcher::finished → onScanFinished`（QueuedConnection） |
| QObject 生命周期 | parent-child 正确无泄漏 | ✅ | `m_progressTimer = new QTimer(this)`（`:46`）；watcher 均 `(this)` |
| 指针 nullptr 保护 | 使用前判空 | ✅ | `if (m_progressTimer) m_progressTimer->start()/stop()`（`scan.cpp:36,75`） |
| 不建线程池 | 并行在 psig，GUI 不建线程 | ✅ | `grep -rn "std::thread\|QThread\|hardware_concurrency" gui/src` → 无命中 |

## 其余 VM 文件

- `bind.cpp`/`process.cpp`/`signature.cpp`/`result.cpp`/`language.cpp`/`viewmodel_helpers.cpp`：**未受影响，跳过**（本版 diff 仅触及 `result.cpp` 的版本串）。

## 结论

ViewModel 层全部检查项通过（无 ❌），无需回退。
