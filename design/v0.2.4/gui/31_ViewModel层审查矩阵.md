# 31 ViewModel 层审查矩阵（修改流程步骤 33，v0.2.4）

- 版本：v0.2.4
- 前置：`design/v0.2.4/gui/21_职责和边界.md`、`24_ViewModel接口.md`
- 审查对象：本次改动涉及的 VM 层文件 —— `viewmodel/viewmodel_helpers.cpp`（C-03）、`viewmodel/result.cpp`（C-03/C-05）、`viewmodel/signature.cpp`（C-04/C-05）、`viewmodel/bind.cpp`（C-05）、`viewmodel/mainviewmodel.h`（C-05）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `viewmodel_helpers.cpp`=87、`result.cpp`=173、`signature.cpp`=173、`bind.cpp`=128、`mainviewmodel.h`=119 |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | `onExportResultsCsv`≈10、`exportResultsToPath`≈25（`result.cpp:41-66`）、`exportOptions`≈10（`:157-166`）、`tableFromModel`≈19（`signature.cpp:140-159`）、`applyTableToModel`≈13（`:161-173`） |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | `exportResultsToPath`：`for` + `if/else` = 2 层；`tableFromModel`/`applyTableToModel`：`for`→`for` = 2 层；`resultHeaderForKey`：`if`+`for`+`if` = 2 层 |
| 单一职责 | 一个函数只做一件事 | ✅ | `exportOptions`=组装导出选项（含 `summary`/`headers`）；`tableFromModel`=表→psig；`applyTableToModel`=psig→表 |
| 错误后提前 return | 非法状态立即返回 | ✅ | `onExportResultsCsv/Txt`：`if (rowCount()==0) { setTransientMessage(...); return; }`；`exportResultsToPath` 导出失败 `return` |
| 头文件越界 | VM 不得 include `view/*.cpp` 内部实现 | ✅ | VM 仅 include `view/*.h`；`grep -rn "\.cpp\"" gui/src/viewmodel` 无命中 |
| 经 getter 读控件 | 不用 findChild 访问交互控件 | ✅ | `grep -rn "findChild" gui/src/viewmodel` 无命中；经 `m_view->resultPage()->...` getter |
| connect 归属 | 集中 install()/bind* | ✅ | `bind.cpp`=41 处（集中绑定）；`signature.cpp:62-65`/`viewmodel_helpers.cpp:83-84` 为**对话框内部**按钮接线（沿用 v0.2.0，非本版引入） |
| 异步与生命周期 | 耗时操作 QtConcurrent::run、主线程无 QThread::wait | ✅ | `process.cpp:20/71`、`scan.cpp:48` 用 `QtConcurrent::run`；`grep -rn "->wait(" gui/src/viewmodel` 无命中 |
| 后台异常回主线程 | 经信号/watcher finished | ✅ | 枚举/扫描经 `m_procWatcher`/`m_scanWatcher` `finished` 回主线程（沿用） |
| 指针 nullptr 保护 | 指针使用前判空 | ✅ | `exportOptions` 经 `m_resultModel->rowCount()`（成员非空）；`importSignaturesFromPath` 前置 `m_view` 已由 install 建立 |
| 静态库调用归属 | 每业务方法对应真实 psig API | ✅ | 见 `32_静态库调用层审查矩阵`；`tableFromModel`/`applyTableToModel` 消费/生产 `psig::Table` |

## 结论

ViewModel 层全部检查项通过（无 ❌），无需回退。

> 说明：本版 VM 改动为「结果导出表头/摘要本地化（C-03）」「特征码表与模板解耦（C-04）」「结果 DB 入口移除 + 文件过滤器收敛（C-05）」；`signature.cpp`/`viewmodel_helpers.cpp` 内的 `connect` 为对话框局部接线，沿用既有实现。
