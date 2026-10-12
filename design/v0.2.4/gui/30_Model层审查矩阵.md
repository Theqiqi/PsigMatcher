# 30 Model 层审查矩阵（修改流程步骤 32，v0.2.4）

- 版本：v0.2.4
- 前置：`design/v0.2.4/gui/21_职责和边界.md`、`23_Model层接口.md`
- 审查对象：本次改动涉及的 Model 层文件 —— 仅 `model/processtablemodel.cpp`（C-01 名称列）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `processtablemodel.cpp`=96、`.h`=沿用（≤400） |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | `data()`（`processtablemodel.cpp:18-34`）=17；`headerData`（`:36-...`）<30 |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | `data()`：`if(DecorationRole)`→1 层、`switch` 分支 1 层（合计 ≤2）；无深层嵌套 |
| 单一职责 | 一个函数只做一件事 | ✅ | `data()` 只按 role/column 返回展示值；名称列直接返回 `r.name` |
| 错误后提前 return | 非法状态立即返回 | ✅ | `data()`：`if (!index.isValid() ...) return {};`（`:19`）、`if (role != Qt::DisplayRole) return {};`（`:24`） |
| 头文件越界 | Model 不得 include `view/` 或 `viewmodel/` | ✅ | `grep -rnE '#include "(view\|viewmodel)' gui/src/model` 无命中 |
| Model 纯净 | 只管理数据、无业务/格式化逻辑 | ✅ | 名称列返回原始 `r.name`，不再做「未知进程」兜底判断（C-01） |
| Model/View 契约正确 | 只读表 flags 不含 ItemIsEditable | ✅ | `flags()` 返回 `ItemIsEnabled \| ItemIsSelectable`（`processtablemodel.cpp:56-59` 区段） |
| 不 include 静态库头 | 无 `psig.h` 引用 | ✅ | `grep -rn "psig" gui/src/model` 无命中 |

## 结论

Model 层全部检查项通过（无 ❌），无需回退。

> 说明：本版 Model 改动仅为名称列取值（C-01）；`signaturetablemodel`/`resulttablemodel` 未受影响，跳过。
