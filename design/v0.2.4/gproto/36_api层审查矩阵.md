# 36 api 层审查矩阵（静态库流程步骤 36，v0.2.4）

- 版本：v0.2.4
- 前置：`design/v0.2.4/gproto/11_api接口分析.md`、`12_接口细化设计.md`
- 审查对象：`psig/src/api/`（本版 `process_scan.cpp`、`exchange_results.cpp` 改动）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `process_scan.cpp`=184、`exchange_results.cpp`=200、`pattern_api.cpp`/`exchange_signatures.cpp` 沿用 ≤400 |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | `EnumerateProcesses`（`process_scan.cpp:137-155`）=19；`ExportResultsTxt`（`exchange_results.cpp:160-198`）=39；`ScanProcesses` 沿用（≤80） |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | `EnumerateProcesses`：`for` 1 层；`ExportResultsTxt`：`for`→`for`→`if` = 3 层（≤3） |
| 单一职责 | 接口只组装、不实现细节 | ✅ | 名称来源选择在 `process_scan.cpp:145`；摘要行取值在 `exchange_results.cpp:179-183`；平台细节在 atomic |
| 错误后提前 return | 非法状态立即返回 | ✅ | `ExportResultsTxt`：`if (!WriteTextFile(...)) { err=...; return false; }`（`:193-196`） |
| 头文件越界 | api/ 不 include `psig.h` 以外的公共头；平台调用不得出现 | ✅ | `grep -rn "windows.h" psig/src/api` 无命中；api 仅 include 下层内部头 + `psig.h` |
| 平台调用归位 | 不在 api 直接调 WinAPI | ✅ | `EnumerateProcesses` 经 `atomic::EnumProcessEntries`/`ProcessNamePath`；无直接 WinAPI |
| 无全局状态 | 无散落可变全局 | ✅ | 仅 `g_processes`/`g_cancel` 既有受控全局（沿用） |
| 组装的原子化调用 | 接口层组装下层原子能力 | ✅ | `EnumerateProcesses` = `EnumProcessEntries` + `ProcessNamePath` + `OpenForRead` 组装 |
| 行为向后兼容 | 不传新字段时行为不变 | ✅ | `summary` 空 → 走 `else` 默认中文（`:181-182`） |

## 结论

api 层全部检查项通过（无 ❌），无需回退。
