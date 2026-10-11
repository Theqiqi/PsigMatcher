# 29 View 层审查矩阵（修改流程步骤 31，v0.2.3）

- 版本：v0.2.3
- 前置：`design/v0.2.2/gui/21_职责和边界.md`、`22_View层接口.md`
- 审查对象：本次改动涉及的 View 层文件 —— 仅 `view/mainwindow.{h,cpp}`（R-06 拆分 `buildMenuAndToolbar` + 版本串变更）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `mainwindow.cpp`=319、`mainwindow.h`=139；其余 view 文件最大 `columnmanagerdialog.cpp`=142 |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | `mainwindow.cpp` 最长 `buildMenus`=39、`retranslateUi`=39（R-06 前 `buildMenuAndToolbar`=113 → 拆为 4 个 ≤39） |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | `mainwindow.cpp` 全部函数控制流 ≤2 层；`createActions` 的 `for`（`mainwindow.cpp:66-69`）= 1 层 |
| 单一职责 | 一个函数只做一件事 | ✅ | R-06 后：`createActions`=建动作、`applyActionIconsAndShortcuts`=图标/快捷键、`buildMenus`=组菜单、`buildToolbar`=组工具栏 |
| 错误后提前 return | 非法状态立即返回 | ✅ | View 无错误路径（纯 UI 构建）；`setTransientMessage` 等无分支副作用 |
| 头文件越界 | View 不得 include `viewmodel/*` 与静态库头、不引用 ViewModel 类型 | ✅ | `grep -rnE '#include "(viewmodel/\|psig)' gui/src/view` 无命中；`grep -rn "MainViewModel" gui/src/view` 无命中 |
| View 纯净 | 只创建控件/布局/样式，无事件处理逻辑 | ✅ | `grep -rn "connect(" gui/src/view` 无命中（连接集中在 VM） |
| 无 signals/private slots | 不声明信号与私有槽 | ✅ | `grep -rnE "signals:\|private slots:\|Q_SIGNALS\|Q_SLOTS" gui/src/view` 无命中 |
| 页面拆分独立 | 页面为独立 QWidget 子类、mainwindow 只做骨架 | ✅ | `signatureeditorpage/processselectionpage/resultpage` 各自 `.h/.cpp`；`mainwindow.cpp` 仅骨架 |
| 公开 getter / 公开 UI 方法 | 每控件有 getter、每更新有 UI 方法 | ✅ | `mainwindow.h:25-61` 页面/动作/控件 getter；`:64-81` 公开 UI 方法（`setTransientMessage`/`appendLog`/`retranslateUi` 等） |
| 头文件可独立编译 | include 自洽 | ✅ | `mainwindow.h` 仅依赖 `<QMainWindow>` + 前置声明；构建通过证明自洽 |

## 结论

View 层全部检查项通过（无 ❌），无需回退。R-06 拆分后 `mainwindow.cpp` 达标。

> 说明：本版对 View 的改动为「版本串 `v0.2.2→v0.2.3`（`mainwindow.cpp:267`）」+「R-06 拆分」；其余 view 文件未受影响。
