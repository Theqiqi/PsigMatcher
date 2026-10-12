# 29 View 层审查矩阵（修改流程步骤 31，v0.2.4）

- 版本：v0.2.4
- 前置：`design/v0.2.4/gui/21_职责和边界.md`、`22_View层接口.md`
- 审查对象：本次改动涉及的 View 层文件 —— `view/hoverrowdelegate.{h,cpp}`（新增）、`view/processselectionpage.cpp`、`view/signatureeditorpage.cpp`、`view/resultpage.{h,cpp}`（C-02/C-05）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `hoverrowdelegate.h`=20、`hoverrowdelegate.cpp`=41、`processselectionpage.cpp`=67、`signatureeditorpage.cpp`=53、`resultpage.cpp`=68、`resultpage.h`=43 |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | 最长 `HoverRowDelegate::paint`（`hoverrowdelegate.cpp:19-40`）=22；`attach`（`:14-18`）=5 |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | `paint`：`if(State_MouseOver)`→`if(view)` = 2 层（`hoverrowdelegate.cpp:23-38`）；页面构造无控制流 |
| 单一职责 | 一个函数只做一件事 | ✅ | `attach`=启用悬停、`paint`=绘制整行背景；页面构造仅建控件 |
| 错误后提前 return | 非法状态立即返回 | ✅ | `attach`：`if (!view) return;`（`hoverrowdelegate.cpp:15`） |
| 头文件越界 | View 不得 include `viewmodel/*` 与静态库头、不引用 ViewModel 类型 | ✅ | `grep -rnE '#include "(viewmodel\|psig)' gui/src/view` 无命中；`grep -rn "MainViewModel" gui/src/view` 无命中 |
| View 纯净 | 只创建控件/布局/样式，无事件处理逻辑 | ✅ | `grep -rn "connect(" gui/src/view` 无命中（连接集中在 VM）；`hoverrowdelegate` 仅 `paint` |
| 无 signals/private slots | 不声明信号与私有槽 | ✅ | `grep -rnE "signals:\|private slots:\|Q_SIGNALS\|Q_SLOTS" gui/src/view` 无命中（`Q_OBJECT` 用于 delegate 元对象，不含信号/槽） |
| 页面拆分独立 | 页面为独立 QWidget 子类、mainwindow 只做骨架 | ✅ | 三页各自 `.h/.cpp`；本次仅在页面构造追加 `HoverRowDelegate::attach(m_table)` |
| 公开 getter / 公开 UI 方法 | 每控件有 getter、每更新有 UI 方法 | ✅ | `resultpage.h:20-25` getter；`exportDbAction()` 已移除（C-05） |
| 头文件可独立编译 | include 自洽 | ✅ | `hoverrowdelegate.h` 仅依赖 `<QStyledItemDelegate>` + 前置声明 `QTableView`；构建通过证明自洽 |

## 结论

View 层全部检查项通过（无 ❌），无需回退。

> 说明：本版 View 改动为「新增整行悬停委托（C-02）」+「三张表安装委托」+「特征码/结果文件对话框过滤器收敛（C-05）」+「结果页移除 DB 导出动作」；其余 view 文件未受影响。
