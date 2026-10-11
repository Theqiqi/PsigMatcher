# 33 data 层审查矩阵（静态库流程步骤 33，v0.2.3）

- 版本：v0.2.3
- 前置：`design/v0.1.0/gproto/13_数据结构分析.md`、`14_算法分析.md`
- 审查对象：`psig/src/data/` 全部 `.h/.cpp`（本版有改动：`codec.{h,cpp}`）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `wc -l`：`data/types.h`=24、`data/codec.h`=23、`data/codec.cpp`=88，均 ≤400 |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | `codec.cpp` 各函数长度：HexVal=6、HexEncode=10、HexDecode=16、NowIso8601=10、EncodeCells=9、DecodeCells=18，最大 18 |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | 最深 `HexDecode`（`codec.cpp:31-46`）与 `DecodeCells`（`codec.cpp:69-86`）控制流 3 层（`for`→`if`/`else`，无第 4 层）；其余 ≤2 |
| 单一职责 | 一个函数只做一件事 | ✅ | `HexEncode/HexDecode`=hex 编解码；`EncodeCells/DecodeCells`=表格 cells 长度前缀编解码；`NowIso8601`=时间格式化 |
| 错误后提前 return | 非法状态立即返回 | ✅ | `HexDecode`：`if (v < 0) return false;`（`codec.cpp:38`）；`DecodeCells`：`if (colon == std::string::npos) return false;`（`codec.cpp:72`）、`if (text[k] < '0' ...) return false;`（`codec.cpp:76`）、`if (start + len > text.size()) return false;`（`codec.cpp:79`） |
| 头文件越界 | data/ 不得 include algorithm/、atomic/、api 层头文件 | ✅ | `grep -rnE '#include "' psig/src/data` 仅命中 `data/codec.cpp:2 #include "data/codec.h"`；`grep -rnE '#include "(algorithm|atomic|api)/' psig/src/data` 无命中 |
| 公共数据结构独立成文件 | 类型与编解码分文件 | ✅ | `data/types.h`（`ColumnDef`/`Table`/`Pattern`/`MatchResult` 等）与 `data/codec.{h,cpp}` 分离 |
| 编解码纯函数、无 I/O | 无文件/网络/进程 I/O | ✅ | `codec.cpp` 无 `fopen/ifstream/CreateFile/read/write`；`NowIso8601` 仅取本地时间（`std::time`/`localtime_s`），非 I/O |
| 编解码无 WinAPI | 不含 `windows.h` 及 WinAPI 调用 | ✅ | `grep -rn "windows.h" psig/src/data` 无命中（R-05 已把 `WideToUtf8/Utf8ToWide` 下沉至 `atomic/win_encoding`） |
| 不承载业务编排 | 无跨层调用/无流程控制 | ✅ | `data/` 仅依赖自身头；无对 algorithm/atomic/api 的调用 |

## 结论

data 层全部检查项通过（无 ❌），无需回退。

> 备注（R-05）：本版首次以「无 WinAPI」量化审查 data 层，命中 `codec.cpp` 曾含 `#include <windows.h>`；已通过把宽窄转换下沉至 `atomic/win_encoding` 修复，`data/` 现为纯 std。
