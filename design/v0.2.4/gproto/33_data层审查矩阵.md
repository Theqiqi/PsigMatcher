# 33 data 层审查矩阵（静态库流程步骤 33，v0.2.4）

- 版本：v0.2.4
- 前置：`design/v0.2.4/gproto/13_数据结构变更.md`
- 审查对象：`psig/src/data/`（本版 `types.h` 新增 `ProcessEntry`）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `types.h`=30、`codec.h`=14、`codec.cpp`=61 |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | `codec.cpp` 最长函数 <80（沿用 v0.2.3） |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | 本版 data 层无新增控制流 |
| 单一职责 | 结构只承载数据 | ✅ | `ProcessEntry{pid,name}` 纯数据，无行为 |
| 平台无关 | 不得 include windows.h 或任何平台头 | ✅ | `types.h` 仅 `<cstdint>/<string>/<vector>`；`grep -rn "windows.h" psig/src/data` 无命中 |
| 框架无关 | 不得 include Qt | ✅ | `grep -rnE '#include <Q' psig/src/data` 无命中 |
| 头文件越界 | 不得 include 上层（algorithm/atomic/api）头 | ✅ | `grep -rnE '#include "(algorithm\|atomic\|api)/' psig/src/data` 无命中 |
| 无全局状态 | 无散落可变全局 | ✅ | 仅类型定义 |
| 新增结构独立性 | 新结构独立、可单独编译 | ✅ | `ProcessEntry` 仅依赖 `<cstdint>/<string>` |

## 结论

data 层全部检查项通过（无 ❌），无需回退。
