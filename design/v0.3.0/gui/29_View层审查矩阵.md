# 29 View 层审查矩阵（GUI 流程步骤 31，v0.3.0）

- 版本：v0.3.0
- 前置：`design/v0.3.0/gui/21_职责和边界.md`、`22_View层接口.md`
- 审查对象：`gui/src/view/`（本版仅 `mainwindow.{h,cpp}` 改动；其余 view 文件未受影响）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行；View 不 include viewmodel/psig

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `mainwindow.cpp`=323、`mainwindow.h`=140 |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | `setTransientMessage`（`mainwindow.cpp:235-243`）=9；其余沿用 ≤80 |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | `setTransientMessage`：`if` 1 层 |
| 单一职责 | 一个函数只做一件事 | ✅ | `setTransientMessage`：更新瞬时标签（含样式表条件更新） |
| 错误后提前 return | 非法状态立即返回 | ✅ | `setProgress`：`value<0` 提前 `return`（沿用） |
| 头文件越界 | View 不 include viewmodel/psig | ✅ | `grep -n '#include' mainwindow.h` → 仅 `<QMainWindow>` |
| 只创建控件/布局/样式 | 不含事件处理逻辑 | ✅ | 本版仅改 `setTransientMessage` 样式表更新条件（纯绘制/展示） |
| 无 signals/private slots | View 不声明 signals、无 private slots | ✅ | 本版未新增 signals/slots |
| 页面独立成文件 | mainwindow.cpp 只做骨架 | ✅ | 本版未改动页面文件与骨架结构 |
| 公开方法暴露 | 经公开方法更新 UI | ✅ | `setTransientMessage`/`setProgress` 为公开方法，VM 经 `m_view->` 调用 |

## 其余 view 文件

- `processselectionpage`/`signatureeditorpage`/`resultpage`/`hoverrowdelegate`/各对话框：**未受影响，跳过**（本版 diff 未触及）。

## 结论

View 层全部检查项通过（无 ❌），无需回退。
