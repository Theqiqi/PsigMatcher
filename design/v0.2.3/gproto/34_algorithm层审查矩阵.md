# 34 algorithm 层审查矩阵（静态库流程步骤 34，v0.2.3）

- 版本：v0.2.3
- 前置：`design/v0.1.0/gproto/14_算法分析.md`、`15_算法执行过程描述.md`
- 审查对象：`psig/src/algorithm/` 全部 `.h/.cpp`（本版有改动：`csv.cpp` 的 `SplitLine`，R-07）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `module_locate.h`=14、`module_locate.cpp`=16、`result_columns.h`=16、`pattern.h`=32、`csv.h`=41、`result_columns.cpp`=46、`pattern.cpp`=87、`csv.cpp`=149，最大 149 |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | 最大 `CompilePattern`=29（`pattern.cpp:34-62`）、`SelectResultKeys`=24、`FindAll`=22；`SplitLine`=14（原 31） |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | `SplitLine`（`csv.cpp:9-22`）原始括号深 4 = 函数体 1 + 控制流 3（`for`→`if(!inQuotes)`→`if(c=='"')`，R-07 由 4 层拉平为 3）；`FindAll`（`pattern.cpp:64-85`）= `for`→`for`→`if` 3 层；`SelectResultKeys`（`result_columns.cpp:21-44`）= `for`→`for`→`if` 3 层 |
| 单一职责 | 一个函数只做一件事 | ✅ | `SplitLine`=按逗号切分一行、`EscapeField`=转义单字段、`CompilePattern`=编译模式、`FindAll`=全量匹配、`SelectResultKeys`=列键筛选 |
| 错误后提前 return | 非法状态立即返回 | ✅ | `SplitLine` 用 `continue` 提前跳过（`csv.cpp:14-21`）；`CompilePattern` 非法 token 早退；`FindAll` `if (m == 0 \|\| size < m) return offsets;`（`pattern.cpp:70`） |
| 头文件越界 | algorithm/ 不得 include atomic/ 或公共头 `psig.h` | ✅ | `grep -rnE '#include "(atomic\|api)/\|#include "psig.h"' psig/src/algorithm` 无命中；仅含自身头 + `data/types.h`（`module_locate.h:7`） |
| 算法独立成文件 | 算法不与功能函数混杂 | ✅ | `csv.cpp`（CSV 解析/BOM）、`pattern.cpp`（模式编译/匹配）、`result_columns.cpp`（结果列）、`module_locate.cpp`（模块定位）各自独立 |
| 纯计算、无 I/O | 无 WinAPI、无进程读写 | ✅ | `grep -rn "windows.h" psig/src/algorithm` 无命中；无文件/进程句柄调用 |
| 只依赖 data 层 | 依赖 data 类型 + 纯编解码 | ✅ | 仅 `module_locate.h:7 #include "data/types.h"` |
| I/O 编排归接口层 | 分块/读取由 api 层承担 | ✅ | 分块读取 `kChunkSize`/`ScanRegion` 在 `api/process_scan.cpp`；algorithm 仅提供纯匹配 |

## 结论

algorithm 层全部检查项通过（无 ❌），无需回退。R-07 拉平后 `SplitLine` 达标。
