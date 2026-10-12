# 29 View 层审查矩阵（GUI 流程步骤 31，v0.3.2）

- 版本：v0.3.2
- 前置：`design/v0.3.2/gui/21_职责和边界.md`、`22_View层接口.md`
- 审查对象（本版改动）：`gui/src/view/mainwindow.h`（新增 `quitAction()` getter）、`gui/src/view/mainwindow.cpp`（标题版本号 + 注释）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行；View 不 include `viewmodel/`/psig

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `mainwindow.h`=141、`mainwindow.cpp`=323（`wc -l`） |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | 本版仅 `MainWindow::retranslateUi`（`mainwindow.cpp:285-323`，39 行）与构造（`:24-31`，8 行）受影响；均 ≤80 |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | 本版改动为赋值/字符串，无新增控制流块（`git diff gui/src/view` 仅标题字符串与 getter 行） |
| 单一职责 | 一个函数只做一件事 | ✅ | `retranslateUi`=重设文案；新增 getter 仅返回值 |
| 错误后提前 return | 非法状态立即返回 | ✅ | 本版无新增分支（不适用） |
| 头文件越界 | View 不 include `viewmodel/`、psig | ✅ | `grep -n '#include' mainwindow.h` → 仅 `<QMainWindow>`；`grep -rn 'viewmodel\|psig' gui/src/view/*.h` → 无真实依赖 |
| 只创建控件/布局/样式（不含事件逻辑） | 无事件处理逻辑 | ✅ | `grep -n 'closeEvent\|eventFilter\|keyPressEvent' gui/src/view/mainwindow.*` → **无命中**（关闭裁决在 ViewModel，见 `31_ViewModel层审查矩阵.md`） |
| 无 signals/private slots | 不声明 signals、无 private slots | ✅ | `grep -n 'signals\|private slots\|Q_SIGNALS\|emit ' gui/src/view/mainwindow.h gui/src/view/mainwindow.cpp` → **无命中** |
| 页面独立成文件 | mainwindow.cpp 只做骨架 | ✅ | 本版未新增页面/类型；三页仍在各自文件 |
| 公开方法暴露 | 经公开 getter/公开 UI 方法暴露 | ✅ | 新增 `QAction* quitAction() const`（`mainwindow.h:41`）为公开只读 getter，无逻辑 |

## 未受影响（跳过）

- `signatureeditorpage.*`、`processselectionpage.*`、`resultpage.*`、`hoverrowtableview.*`、`hoverrowdelegate.*`、`aboutdialog.*`、`columnmanagerdialog.*`、`exportcolumnsdialog.*`：**未受影响，跳过**（本版 diff 未触及）。

## 结论

View 层全部检查项通过（无 ❌）；C-03 的关闭提示**未**把事件处理逻辑放入 View（仅暴露动作 getter），C-04 的改名只影响标题字符串。无需回退。
