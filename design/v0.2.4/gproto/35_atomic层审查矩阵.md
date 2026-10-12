# 35 atomic 层审查矩阵（静态库流程步骤 35，v0.2.4）

- 版本：v0.2.4
- 前置：`design/v0.2.4/gproto/17_原子化api分析.md`、`18_原子化api设计.md`
- 审查对象：`psig/src/atomic/`（本版 `win_process.{h,cpp}` 改动：`EnumProcessIds` → `EnumProcessEntries`）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `win_process.h`=33、`win_process.cpp`=121（其余沿用 v0.2.3，均 ≤400） |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | 新增 `EnumProcessEntries`（`win_process.cpp:20-36`）≈16 行；`ProcessNamePath`=14；`EnumRegions`=19；`ReadMemory`=12；`EnumModules`=17 |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | `EnumProcessEntries`：函数体 1 + `if(Process32FirstW)` 1 + `do/while` 1 = 控制流 2 层（≤3） |
| 单一职责 | 一个函数只做一件事 | ✅ | `EnumProcessEntries` 只做「快照遍历取 pid+名」；`ProcessNamePath` 只做「取名与路径」 |
| 错误后提前 return | 非法状态立即返回 | ✅ | `EnumProcessEntries`：`if (snap == INVALID_HANDLE_VALUE) return entries;`（`win_process.cpp:23`） |
| 头文件越界 | atomic/ 不得 include algorithm/、api 层头与公共头 `psig.h` | ✅ | `grep -rnE '#include "' psig/src/atomic` 仅命中自身头 + `data/types.h`（`win_process.h:8`） |
| 平台调用归位 | WinAPI 仅出现在 atomic 层 | ✅ | `grep -rn "windows.h" psig/src` 仅命中 `atomic/`（`win_process.cpp:4` 等） |
| 无全局状态 | 无散落可变全局 | ✅ | `win_process.cpp` 仅匿名 namespace 内辅助函数 |
| 句柄成对释放 | 资源获取/释放配对 | ✅ | `CreateToolhelp32Snapshot`（`:22`）↔ `CloseHandle(snap)`（`:33`）；`OpenProcess`↔`CloseHandle`（`ProcessNamePath`） |
| 新增结构落位 | 内部结构置于 data 层 | ✅ | `ProcessEntry` 定义于 `data/types.h`，atomic 仅使用 |

## 结论

atomic 层全部检查项通过（无 ❌），无需回退。

> 说明：atomic 层的 WinAPI 调用（`CreateToolhelp32Snapshot`/`Process32W`/`QueryFullProcessImageNameW`）是**合法设计**——原子化层即平台能力封装层。
