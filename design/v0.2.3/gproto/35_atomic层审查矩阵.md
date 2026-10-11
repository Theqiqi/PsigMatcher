# 35 atomic 层审查矩阵（静态库流程步骤 35，v0.2.3）

- 版本：v0.2.3
- 前置：`design/v0.1.0/gproto/17_原子化api分析.md`、`18_原子化api设计.md`
- 审查对象：`psig/src/atomic/` 全部 `.h/.cpp`（本版新增 `win_encoding.{h,cpp}`）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `file_io.h`=14、`file_io.cpp`=25、`sqlite_io.h`=32、`sqlite_io.cpp`=103、`win_icon.h`=26、`win_icon.cpp`=122、`win_process.h`=33、`win_process.cpp`=118、`win_encoding.h`=14、`win_encoding.cpp`=28，最大 122 |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | 最大 `IconToBitmap`=78（`win_icon.cpp:14-91`）；新增 `WideToUtf8`=10、`Utf8ToWide`=8 |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | `IconToBitmap`（`win_icon.cpp:14-91`）原始括号深 4 = 函数体 1 + 控制流 3（`if(!anyAlpha&&ii.hbmMask)`→`if(maskDib&&maskBits)`→`for`）；`SqliteQuery`（`sqlite_io.cpp:64-90`）控制流 3 层；其余 ≤3 |
| 单一职责 | 一个函数只做一件事 | ✅ | `WideToUtf8`/`Utf8ToWide` 各只做单一方向转换；`SqliteOpen/Exec/Run/Query` 职责分明 |
| 错误后提前 return | 非法状态立即返回 | ✅ | `WideToUtf8`：`if (w.empty()) return {};`（`win_encoding.cpp:9`）、`if (n <= 0) return {};`（`:12`）；`IconToBitmap`：`if (!hIcon) return out;`（`win_icon.cpp:16`）等 |
| 头文件越界 | atomic/ 不得 include algorithm/、api 层头文件与公共头 `psig.h` | ✅ | `grep -rnE '#include "' psig/src/atomic` 仅命中自身头 + `data/types.h`（`win_process.h:8`）；无 `algorithm/`、`api/`、`psig.h` |
| 平台调用归位 | WinAPI 调用仅出现在 atomic 层 | ✅ | `grep -rn "windows.h" psig/src` 仅命中 `atomic/`（`win_encoding.cpp:4`、`win_icon.cpp`、`win_process.cpp`） |
| 无全局状态 | 无散落可变全局 | ✅ | atomic 各文件仅在匿名 namespace 内定义辅助函数，无导出可变全局 |
| 句柄成对释放 | 资源获取/释放配对 | ✅ | `win_icon.cpp:31-34` `cleanupHandles`（DeleteObject）；`:86-88` `SelectObject/DeleteObject/DeleteDC`；`win_process.cpp` `CloseProcessHandle` |
| 新增文件独立性 | 新原子模块独立成文件 | ✅ | `atomic/win_encoding.{h,cpp}` 独立，与 `win_icon`/`win_process` 不混杂 |

## 结论

atomic 层全部检查项通过（无 ❌），无需回退。

> 说明：atomic 层的 WinAPI 调用（`CreateDIBSection`/`EnumProcesses`/`WideCharToMultiByte` 等）是**合法设计**——原子化层即平台能力封装层；「无 WinAPI」约束只针对 data/algorithm 层。
