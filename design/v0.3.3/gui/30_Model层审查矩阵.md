# 30 Model 层审查矩阵（GUI 流程步骤 32，v0.3.3）

- 版本：v0.3.3
- 前置：`design/v0.3.3/gui/21_职责和边界.md`、`23_Model层接口.md`
- 审查对象（本版改动）：**无**（本版 diff 未触及 `gui/src/model/`）
- 量化标准：嵌套 ≤3；函数体 ≤80 行；文件 ≤400 行；Model 不 include `view/`/`viewmodel/`、不 include 静态库头

## 矩阵（逐项标注「未受影响，跳过」）

| 检查项 | 判定标准（量化） | 结论 | 证据 |
| --- | --- | --- | --- |
| 本次改动涉及的 Model 文件 | — | **无** | `git diff v0.3.2..HEAD --stat -- gui/src/model` → **空输出** |
| 文件长度 | 单文件 ≤400 行 | 未受影响，跳过 | 未改动（`git diff` 空） |
| 函数体长度 | 任一函数 ≤80 行 | 未受影响，跳过 | 同上 |
| 嵌套深度 | 控制流块 ≤3 层 | 未受影响，跳过 | 同上 |
| 单一职责 | 一个函数只做一件事 | 未受影响，跳过 | 同上 |
| 错误后提前 return | 非法状态立即返回 | 未受影响，跳过 | 同上 |
| 头文件越界 | Model 不 include `view/`/`viewmodel/` | 未受影响，跳过 | 同上 |
| Model 纯净 | 无业务/格式化逻辑 | 未受影响，跳过 | 同上 |
| Model/View 契约 | 只读表 `flags()` 不含 `ItemIsEditable` | 未受影响，跳过 | 同上 |
| 不 include 静态库头 / 不调静态库 API | — | 未受影响，跳过 | 同上 |

## 易混淆点复核（本版新增图标**不**属于 Model）

| 项 | 结论 | 证据 |
| --- | --- | --- |
| `ProcessTableModel::setIcon`（模型行装饰图标） | **未改动** | `gui/src/model/processtablemodel.cpp` 不在本版 diff 中 |
| 本版新增的程序外壳图标 | **与 Model 无关** | 装配在 `gui/src/main.cpp`（入口）；资源在 `gui/res/`（见 `23_Model层接口.md` 第二节对照表） |

## 结论

本版 Model 层零改动：全部检查项按「未受影响，跳过」处理（流程允许，且已逐条标注，未默认略过）。无 ❌，无需回退。
