# 36 api 层审查矩阵（静态库流程步骤 36，v0.2.3）

- 版本：v0.2.3
- 前置：`design/v0.1.0/gproto/11_api接口分析.md`、`12_接口细化设计.md`、`design/v0.2.3/gproto/00_CHANGELOG.md`
- 审查对象：`psig/src/api/` 全部 `.h/.cpp`（本版主战场：R-01 拆分 + R-02 拉平）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `api_internal.h`=21、`pattern_api.cpp`=38、`exchange_signatures.cpp`=168、`process_scan.cpp`=183、`exchange_results.cpp`=196，最大 196（原单文件 `psig.cpp`=554 已删） |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | 最大 `ExportResultsDb`=47（`exchange_results.cpp:112-158`）、`ImportSignaturesDb`=42（`exchange_signatures.cpp:72-113`）、`ExportSignaturesDb`=36、`ExportResultsTxt`=35、`ScanRegion`=34、`ImportSignaturesCsv`=34；`ScanProcesses`=16（原 91） |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | 最深为 3 层：`ScanRegion`/`ScanBuffer`（`process_scan.cpp`）、`ExportSignaturesDb`/`ImportSignaturesDb`（`exchange_signatures.cpp`）、`ExportResultsDb`/`ExportResultsTxt`/`BuildResultHeaders`（`exchange_results.cpp`）。`ScanProcesses` 由 6 层 → 2 层 |
| 单一职责 | 一个函数只做一件事 | ✅ | `ScanBuffer`=单块匹配、`UpdateCarry`=跨块尾字节接续、`MakeMatchResult`=结果构造、`ScanRegion`=区域分块编排、`ScanOneProcess`=单进程扫描 |
| 错误后提前 return | 非法状态立即返回 | ✅ | `ExportSignaturesDb`：`if (!db) return false;`（`exchange_signatures.cpp:134`）；`ScanProcesses` 的 `IsCancelled` 早退（`process_scan.cpp:22-24`）；`ExportResultsCsv/Db/Txt` 首行空数据早退 |
| **按职责拆分模块文件** | 不得单文件混杂多职责 | ✅ | 进程/模式/数据交换拆为 4 文件：`process_scan.cpp`（进程枚举/扫描/取消）、`pattern_api.cpp`（模式解析/内置列/结果列转发）、`exchange_signatures.cpp`（特征码导入导出）、`exchange_results.cpp`（结果导出）；跨文件 glue 独立为 `api_internal.h`（`MakeColumn`） |
| 接口=组装函数 | 组装「算法+数据结构+原子化」，不内联重复实现算法/不直接底层 I/O | ✅ | `ScanProcesses`（`process_scan.cpp:164-179`）编排 `EnumerateProcesses`+`ScanOneProcess`；I/O 经 `atomic::*`、算法经 `algorithm::*`，未内联 |
| 参数校验完备 | 空指针/越界/非法组合 | ✅ | `ScanProcesses` 判空/取消（`process_scan.cpp`）；`ExportSignaturesDb` 首部 `SqliteOpen` 失败即返；`DecodeCells` 越界校验在 data 层 |
| 只暴露公共 API | 内部细节不外泄 | ✅ | 内部 glue `MakeColumn` 置于 `api_internal.h`（未进公共头）；`namespace { }` 匿名辅助（`exchange_results.cpp:14`）不外泄 |
| 定义与公共头一致 | 签名与 `psig.h` 逐字一致 | ✅ | 公共符号 25 个（见 `gui/32_静态库调用层审查矩阵.md`）均与 `psig.h` 声明一致；`test/unit` 直接调用验证 |
| 头文件越界 | 只 include 公共头与下层内部头 | ✅ | `grep -rnE '#include "' psig/src/api`：均含 `psig.h` + 下层 `algorithm/*`、`atomic/*`、`data/*`、`api/api_internal.h`；无跨到 GUI |

## 结论

api 层全部检查项通过（无 ❌），无需回退。R-01（拆分）与 R-02（拉平）均达标。

> 对照：修复前 `api/psig.cpp` 单文件 554 行、`ScanProcesses` 91 行/6 层 → 现最大文件 196 行、最大函数 47 行、最大控制流嵌套 3 层。
