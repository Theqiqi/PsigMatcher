# 36 api 层审查矩阵（静态库流程步骤 36，v0.3.0）

- 版本：v0.3.0
- 前置：`design/v0.3.0/gproto/11_api接口分析.md`、`12_接口细化设计.md`
- 审查对象：`psig/src/api/`（本版 `process_scan.cpp` 改动；其余 api 文件未受影响）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `process_scan.cpp`=300；`exchange_results.cpp`/`exchange_signatures.cpp`/`pattern_api.cpp` 沿用（未受影响） |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | 见下表（最长 `RunParallel`=41 行） |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | `RunWorker`：`for`→`if`（2 层）；`RunParallel`：`else`→`for`→`try`（2 层）；`ScanRegion`：`while`→`if`（2 层） |
| 单一职责 | 接口只组装、不实现细节 | ✅ | 规划 `BuildPlans`、展平 `FlattenTasks`、工作线程 `RunWorker`、并行编排 `RunParallel` 各自单一职责；平台细节在 atomic |
| 错误后提前 return | 非法状态立即返回 | ✅ | `ScanProcesses`：`tasks.empty()` → `ClosePlans` 后 `return {}`（`process_scan.cpp:283-286`） |
| 头文件越界 | api/ 只 include 公共头与下层内部头 | ✅ | `grep -n '#include' process_scan.cpp`：`psig.h` + `algorithm/*` + `atomic/*` + `data/*`，无越界 |
| 平台调用归位 | 不在 api 直接调 WinAPI | ✅ | `grep -rn "ReadProcessMemory\|OpenProcess\|VirtualQuery\|CreateToolhelp\|windows.h\|HANDLE" psig/src/api` → 无命中 |
| 无 Qt 依赖 | 并行只用标准库 | ✅ | `grep -rn "Qt\|QThread\|QtConcurrent" psig/src` → 仅注释命中，无代码命中 |
| 组装的原子化调用 | 接口层组装下层原子能力 | ✅ | `BuildPlans` = `OpenForRead`+`ProcessNamePath`+`EnumModules`+`EnumRegions`；`ScanRegion` = `ReadMemory`+`FindAll` |
| 参数校验完备 | 空输入安全 | ✅ | `total==0` → 返回空；`IsCancelled` 轮询；`ReadMemory` 空块跳过 |
| 定义与公共头一致 | 签名不变 | ✅ | `ScanProcesses`/`ScanProgress` 与 `psig.h` 一致；无新增公共 API |

## 函数体长度明细（`process_scan.cpp`）

| 函数 | 行范围 | 行数 |
| --- | --- | --- |
| IsCancelled | 25-27 | 3 |
| ToPublicBitmap | 29-35 | 7 |
| MakeMatchResult | 38-53 | 16 |
| ScanBuffer | 59-69 | 11 |
| UpdateCarry | 74-81 | 8 |
| ScanRegion | 84-108 | 25 |
| BuildPlans | 127-145 | 19 |
| ClosePlans | 147-150 | 4 |
| FlattenTasks | 153-159 | 7 |
| RunWorker | 178-202 | 25 |
| RunParallel | 205-245 | 41 |
| EnumerateProcesses | 248-266 | 19 |
| ProcessIcon | 268-270 | 3 |
| DefaultProcessIcon | 272-274 | 3 |
| ScanProcesses | 276-296 | 21 |
| RequestCancel | 298 | 1 |

## 结论

api 层全部检查项通过（无 ❌），无需回退。
