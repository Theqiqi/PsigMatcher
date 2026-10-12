# 30 Model 层审查矩阵（GUI 流程步骤 32，v0.3.2）

- 版本：v0.3.2
- 前置：`design/v0.3.2/gui/21_职责和边界.md`、`23_Model层接口.md`
- 审查对象：`gui/src/model/`（本版**无改动**）

## 结论：未受影响，跳过

| 检查项 | 结论 | 证据 |
| --- | --- | --- |
| 本版是否改动 `model/` | **未受影响，跳过** | `git diff v0.3.1..HEAD --stat -- gui/src/model/` → 无输出（本版未触碰任何 model 文件） |

## 对 C-01 依赖的既有契约复核（只读复核，非改动）

| 检查项 | 判定标准（量化） | 结论 | 证据 |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `signaturetablemodel.cpp`=147、`signaturetablemodel.h`=38（`wc -l`） |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | `SignatureTableModel::setData`（`signaturetablemodel.cpp:30-40`，11 行）≤80 |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | `setData` 仅 3 个并列 `if`（`signaturetablemodel.cpp:31-33`），无嵌套 |
| 单一职责 | 一个函数只做一件事 | ✅ | `setData`=写入单元格并发 `dataChanged` |
| 错误后提前 return | 非法状态立即返回 | ✅ | `setData` 对 role/index/row/column 非法各提前 `return false`（`:31-33`） |
| 头文件越界 | Model 不 include `view/`、`viewmodel/` | ✅ | `grep -n '#include' signaturetablemodel.h` → 仅 `<QAbstractTableModel>`、`model/types.h` |
| Model 纯净 | 无业务/格式化逻辑 | ✅ | 仅数据存取；格式化在 ViewModel/委托 |
| Model/View 契约 | `flags` 正确 | ✅ | `SignatureTableModel::flags` 返回 `ItemIsEnabled\|ItemIsSelectable\|ItemIsEditable`（可编辑表）；只读表（进程/结果首列）不含 `ItemIsEditable`（`test_view.cpp` 断言） |
| 不 include 静态库 / 不调静态库 | — | ✅ | `grep -rn 'psig' gui/src/model/` → 仅 2 处**注释**命中（`processtablemodel.cpp:28`、`types.h:11`），无 include、无 API 调用 |

## 结论

Model 层本版**未受影响，跳过**；C-01 所依赖的 `dataChanged` 契约经复核满足（`setData` 已发信号），无需改动 Model。
