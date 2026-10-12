# 34 algorithm 层审查矩阵（静态库流程步骤 34，v0.2.4）

- 版本：v0.2.4
- 前置：`design/v0.2.4/gproto/14_算法变更.md`
- 审查对象：`psig/src/algorithm/`（本版**无改动**，复核）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `csv.cpp`、`pattern.cpp`、`module_locate.cpp`、`result_columns.cpp` 均 ≤400（沿用 v0.2.3） |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | 沿用 v0.2.3（`SplitLine` 已拉平） |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | 沿用 v0.2.3（`SplitLine` ≤3） |
| 纯函数 | 无 I/O、无平台 API、无全局状态 | ✅ | `grep -rnE '#include "(atomic\|api)/\|windows.h\|<Q' psig/src/algorithm` 无命中 |
| 本版无改动 | 未引入新算法 | ✅ | 本轮 R-01/R-02 均落在 atomic/api 层（见 14_算法变更.md） |

## 结论

algorithm 层本版无改动，复核通过（无 ❌）。
