# 31 ViewModel 层审查矩阵（修改流程步骤 33，v0.2.3）

- 版本：v0.2.3
- 前置：`design/v0.2.2/gui/21_职责和边界.md`、`24_ViewModel接口.md`
- 审查对象：`gui/src/viewmodel/` 全部 `.h/.cpp`（本版主战场：R-03 拆分 + R-04 删死字段）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `viewmodel_helpers.h`=42、`mainviewmodel_language.cpp`=50、`viewmodel_helpers.cpp`=87、`mainviewmodel_scan.cpp`=90、`mainviewmodel.cpp`=97、`mainviewmodel.h`=120、`mainviewmodel_bind.cpp`=130、`mainviewmodel_process.cpp`=155、`mainviewmodel_signature.cpp`=175、`mainviewmodel_result.cpp`=184，最大 184（R-03 前 `mainviewmodel.cpp`=931） |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | 最大 `onStartScan`=43（`mainviewmodel_scan.cpp:9-51`）、`applyLanguage`=41（`mainviewmodel_language.cpp:10-50`）、`install`=39（`mainviewmodel.cpp:28-66`）、`onFillCustomColumn`=33；无 >80 |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | 最深 `startIconLoad`（`mainviewmodel_process.cpp:59-85`）原始括号深 4 = 函数体 1 + lambda 基 + 控制流 3；`collectPatterns`（`mainviewmodel_scan.cpp:73-90`）= `for`→`if`→`if` 3 层；其余 ≤3 |
| 单一职责 | 一个函数只做一件事 | ✅ | R-03 按功能域分文件后：`onExportResultsCsv/Db/Txt` 各只取路径转发；`tableFromModel`/`applyTableToModel` 只做转换 |
| 错误后提前 return | 非法状态立即返回 | ✅ | `install` 首行 `if (!view) return;`（`mainviewmodel.cpp:26`）；`importSignaturesFromPath` 失败早退（`mainviewmodel_signature.cpp:105-109`） |
| 头文件越界 | VM 不得 include `view/*.cpp` 内部实现、不得访问 View 私有成员 | ✅ | VM 仅 include `view/*.h` 公共头与 `viewmodel/viewmodel_helpers.h`；无 `*.cpp` 包含、无 `#define private public` 类技巧 |
| 持 View*/Model* | `m_view` 初值 nullptr、经 install 注入 | ✅ | `mainviewmodel.h:102 MainWindow* m_view = nullptr;`；`mainviewmodel.cpp:25-27 install()` 注入 |
| 经 View 公开 getter/方法 | 不 findChild 访问交互控件 | ✅ | `grep -rn "findChild" gui/src/viewmodel` 无命中；均经 `m_view->xxxPage()->...` 公开 getter |
| 全部连接集中 VM | View 侧零 connect | ✅ | `grep -rn "connect(" gui/src/view` 无命中；连接集中在 `mainviewmodel_bind.cpp`（`bind*` 系列）与 `install` |
| 耗时操作后台化 | QtConcurrent、主线程无 wait | ✅ | `startIconLoad` 经 `QtConcurrent::run`（`mainviewmodel_process.cpp:71`）+ `QMetaObject::invokeMethod(...QueuedConnection)`；`onStartScan` 经 `QtConcurrent::run` |
| 后台异常/回填安全 | 经队列回主线程、生命周期正确 | ✅ | 图标任务持 `m_alive` 存活标志（`mainviewmodel.cpp:16-17`、`process.cpp:73`）；析构 `m_alive->store(false)`（`mainviewmodel.cpp:20`） |
| 指针参数 nullptr 保护 | 入参判空 | ✅ | `install(MainWindow* view)` 判空；`onProcessesEnumerated` 判 watcher |
| QObject parent-child | parent 设置正确、无泄漏 | ✅ | Model 以 `this` 为 parent（`mainviewmodel.cpp:29-31`）；watcher/translator 以 `this` 为 parent |
| 死字段清理 | 无未引用成员 | ✅ | R-04：`grep -rn "m_lastError" gui` 无命中（原 `mainviewmodel.h:111` 已删） |
| tr() 上下文保持 | helpers 用 `QObject::tr`，`.ts` 上下文不失效 | ✅ | `viewmodel_helpers.cpp` 用 `QObject::tr`；`.ts` 的 `<name>QObject</name>`（`psig_gui_en_US.ts:733`）上下文条目（如 `特征码名` `:736`）保留 |
| 公开 API 不变 | 头文件公开接口与测试零改动 | ✅ | `mainviewmodel.h` 公开段（`install`/`*FromPath`/`*ToPath`/`setTemplateText`/`setExportColumns`/`applyLanguage`）签名不变；单测/集成/E2E 无改动且全绿 |

## 结论

ViewModel 层全部检查项通过（无 ❌），无需回退。R-03（拆分）与 R-04（删死字段）均达标。

> 对照：修复前 `mainviewmodel.cpp` 单文件 931 行 → 现最大文件 184 行、最大函数 43 行。
