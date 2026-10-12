# 31 ViewModel 层审查矩阵（GUI 流程步骤 33，v0.3.1）

- 版本：v0.3.1
- 前置：`design/v0.3.1/gui/21_职责和边界.md`、`24_ViewModel接口.md`
- 审查对象：`gui/src/viewmodel/`（本版 `signature.cpp`、`mainviewmodel.h` 改动；其余 VM 文件未受影响）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `signature.cpp`=198、`mainviewmodel.h`=129 |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | `importSignaturesFromPath`（`signature.cpp:97-126`）=30；`onImportSignatures`（`:73-95`）=23；`tableFromModel`（`:163-184`）=22；`onManageColumns`（`:54-71`）=18；`onDeleteSignatureRows`（`:38-52`）=15；`exportSignaturesToPath`（`:147-161`）=15；`applyTableToModel`（`:186-198`）=13；`onAddSignatureRows`（`:26-36`）=11；`confirmTargetProcessMismatch`（`:129-138`）=10；`onAddSignatureRow`（`:17-24`）=8 |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | `onImportSignatures`：`if(rowCount>0)`→`if(clickedButton==saveBtn)`→`if(savePath.isEmpty())` = 3 层（`:75-90`）；`importSignaturesFromPath`：`if(!libProc...)`→`if(!confirm...)` = 2 层（`:112-117`）；`tableFromModel`：`for` 1 层（`:168-176`） |
| 单一职责 | 一个函数只做一件事 | ✅ | `importSignaturesFromPath`=读库+校验+套用；`confirmTargetProcessMismatch`=仅弹提示返回决策（C-05 拆出独立函数）；`tableFromModel`=模型→psig::Table 单向转换 |
| 错误后提前 return | 非法状态立即返回 | ✅ | `importSignaturesFromPath`：`err` 非空提前 `return`（`:105-109`）、用户取消提前 `return`（`:113-117`）；`onImportSignatures`：取消/空路径提前 `return`（`:83,87,93`） |
| 头文件越界 | VM 不 include view/*.cpp 内部实现 | ✅ | `grep -n '#include' signature.cpp` → `mainviewmodel.h`/`view/*.h`（经公开头）/`viewmodel_helpers.h`，无 `*.cpp` |
| 持 View*/Model* 经 getter | 不 findChild | ✅ | `grep -rn "findChild" gui/src/viewmodel` → 无命中 |
| connect 集中 | install()/bind* 集中 | ✅ | 本版未新增 connect；`signature.cpp` 内 connect 仅 `onManageColumns` 的对话框按钮（`signature.cpp:62-65`），沿用既有模式 |
| 决策弹窗经 VM | View 不弹窗，弹窗由 VM 发起 | ✅ | `confirmTargetProcessMismatch`（`signature.cpp:129-138`）在 VM 内构造 `QMessageBox`；View 无 `QMessageBox` |
| 业务数据入 psig 结构 | VM 负责 model↔psig::Table | ✅ | `t.targetProcess = m_targetName.toStdString();`（`:166`）；导入侧读 `table.targetProcess`（`:111`） |
| QObject 生命周期 | parent-child 正确无泄漏 | ✅ | `QMessageBox box(m_view)` 栈对象，随作用域析构（`:76,130`）；无 `new` 裸指针 |
| 指针 nullptr 保护 | 使用前判空 | ✅ | `if (!sel) return;`（`:40`）；`if (!m_table->model()...)` 由页面处理（沿用） |

## 其余 VM 文件

- `bind.cpp`/`process.cpp`/`result.cpp`/`language.cpp`/`scan.cpp`/`viewmodel_helpers.cpp`：**未受影响，跳过**（本版 diff 未触及）。

## 结论

ViewModel 层全部检查项通过（无 ❌），无需回退。C-05 的携带（导出写 `targetProcess`）与提示（导入不一致弹窗、可继续）均在 VM 层闭环，决策弹窗未泄漏到 View。
