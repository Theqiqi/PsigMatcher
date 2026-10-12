# 31 ViewModel 层审查矩阵（GUI 流程步骤 33，v0.3.3）

- 版本：v0.3.3
- 前置：`design/v0.3.3/gui/21_职责和边界.md`、`24_ViewModel接口.md`
- 审查对象（本版改动）：`gui/src/viewmodel/mainviewmodel.cpp`（启动日志文案中的版本号）、`gui/src/viewmodel/result.cpp`（关于框文案中的版本号）
- 量化标准：嵌套 ≤3；函数体 ≤80 行；文件 ≤400 行；VM 不 include `view/*.cpp` 内部实现

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `mainviewmodel.cpp`=127、`result.cpp`=173（`wc -l`）；`mainviewmodel.h` 未改动 |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | 本版仅改 2 处字符串字面量（不新增函数），所在函数未变长 |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | `git diff v0.3.2..HEAD -- gui/src/viewmodel` → 2 文件各 **1 增 1 删**，无控制流变化 |
| 单一职责 | 一个函数只做一件事 | ✅ | 改动仅替换 `tr()` 中的版本号文本 |
| 错误后提前 return | 非法状态立即返回 | ✅ | 本版无新增分支（不适用） |
| 头文件越界 | VM 不 include `view/*.cpp` 内部实现 | ✅ | `mainviewmodel.h` 本版未改动；`grep -n '#include' gui/src/viewmodel/mainviewmodel.h` → 仅 Qt + `model/*.h` |
| 持 `View*`/`Model*` 经 getter 读控件 | 不用 `findChild` | ✅ | `grep -rn 'findChild' gui/src/viewmodel` → **无命中** |
| `connect` 集中在 `install()`/`bind*` | 不在业务函数里散落 connect | ✅ | 本版未新增/移动任何 `connect`（`git diff` 仅字符串） |
| 耗时操作 `QtConcurrent::run` 且主线程无 `QThread::wait()` | 不阻塞 UI | ✅ | 既有实现未改动（`process.cpp:21,72`、`scan.cpp:52` 仍为 `QtConcurrent::run`）；`grep -rn 'QThread::wait' gui/src/viewmodel` → **无命中** |
| 后台异常经信号回主线程 | 不跨线程抛异常 | ✅ | 本版未引入线程/回调 |
| QObject 生命周期 / parent-child 无泄漏 | — | ✅ | 本版未新增/销毁任何 QObject（仅字符串） |
| 指针 nullptr 保护 | — | ✅ | 本版未新增指针访问 |
| **图标装配不进 VM** | VM 不得持外壳资源职责 | ✅ | `grep -rn 'setWindowIcon\|app\.png\|app_ico\|QIcon(' gui/src/viewmodel` → 本版**无新增命中**（唯一 `QIcon` 使用是既有的进程图标回填：`process.cpp:81,88`） |

## 未受影响（跳过）

- `bind.cpp`、`process.cpp`、`signature.cpp`、`scan.cpp`、`language.cpp`、`viewmodel_helpers.cpp`、`mainviewmodel.h`：**未受影响，跳过**（本版 diff 未触及）。

## 结论

ViewModel 层全部检查项通过（无 ❌）。本版的 ViewModel 改动**只有两处版本号文本**，未新增控件访问、静态库调用或线程；图标装配按 `24_ViewModel接口.md` 第二节的判据归属入口层。无需回退。
