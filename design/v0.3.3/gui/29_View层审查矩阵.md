# 29 View 层审查矩阵（GUI 流程步骤 31，v0.3.3）

- 版本：v0.3.3
- 前置：`design/v0.3.3/gui/21_职责和边界.md`、`22_View层接口.md`
- 审查对象（本版改动）：`gui/src/view/mainwindow.cpp`（标题版本号 1 处字符串）
- 量化标准：嵌套 ≤3（控制流块层数，函数体不计层）；函数体 ≤80 行；文件 ≤400 行；View 不 include `viewmodel/`/psig

## 矩阵

| 检查项 | 判定标准（量化） | 结论 | 证据（文件:行号 / 计数 / grep 命中） |
| --- | --- | --- | --- |
| 文件长度 | 单文件 ≤400 行 | ✅ | `mainwindow.cpp`=323、`mainwindow.h`=141（`wc -l`） |
| 函数体长度 | 任一函数 ≤80 行 | ✅ | 本版唯一受影响函数 `MainWindow::retranslateUi`（`mainwindow.cpp:285-323`）= **39 行** ≤80；构造等未改动 |
| 嵌套深度 | 控制流块 ≤3 层 | ✅ | 本版 diff 仅替换一处字符串（`git diff v0.3.2..HEAD -- gui/src/view` → 1 增 1 删），无新增控制流块 |
| 单一职责 | 一个函数只做一件事 | ✅ | `retranslateUi` 仍只做「重设界面文案」 |
| 错误后提前 return | 非法状态立即返回 | ✅ | 本版无新增分支（不适用） |
| 头文件越界 | View 不 include `viewmodel/`、psig | ✅ | `grep -n '#include' gui/src/view/mainwindow.h` → 仅 `<QMainWindow>`；`grep -rn 'viewmodel\|psig' gui/src/view/mainwindow.{h,cpp}` → **无命中**；`mainwindow.h` 本版未改动（141 行） |
| 只创建控件/布局/样式（不含事件逻辑） | 无事件处理逻辑 | ✅ | `grep -n 'closeEvent\|eventFilter\|keyPressEvent\|mousePressEvent' gui/src/view/mainwindow.{h,cpp}` → **无命中**（关闭裁决仍在 ViewModel，见 `31_ViewModel层审查矩阵.md`） |
| 无 signals/private slots | 不声明 signals、无 private slots | ✅ | `grep -n 'signals\|private slots\|Q_SIGNALS\|emit ' gui/src/view/mainwindow.{h,cpp}` → **无命中** |
| 页面独立成文件 | `mainwindow.cpp` 只做骨架 | ✅ | 本版未新增页面/类型；三页仍在各自文件；`mainwindow.cpp` 只做菜单/工具栏/中央区/状态区装配 |
| 公开方法暴露 | 经公开 getter/公开 UI 方法暴露 | ✅ | `mainwindow.h` 公开面**零变化**（`22_View层接口.md` 第四节逐条对照）；图标装配**不**进 View（在 `main.cpp`，见该文档第三节论证） |
| 图标不引入 View 依赖 | View 不得因图标/版本新增依赖 | ✅ | `git diff v0.3.2..HEAD -- gui/src/view` 仅 1 行字符串；`.qrc`/`.ico` 由入口与构建层处理 |

## 未受影响（跳过）

- `signatureeditorpage.*`、`processselectionpage.*`、`resultpage.*`、`hoverrowtableview.*`、`hoverrowdelegate.*`、`aboutdialog.*`、`columnmanagerdialog.*`、`exportcolumnsdialog.*`：**未受影响，跳过**（本版 diff 未触及）。
- `mainwindow.h`：**未受影响，跳过**（无公开接口变化）。

## 截图证据（E2E 实测）

| 项 | 实测 |
| --- | --- |
| 标题（英文界面） | `hwnd=9310484 title='Process Signature Batch Matcher v0.3.3'`（`gui/out/verify/；/tmp/e2e.log` 驱动输出） |
| 主界面渲染 | `size=1100x720 luma=242.8 ui_ratio=0.507`（E2E-01 PASS） |

## 结论

View 层全部检查项通过（无 ❌）。C-01 的图标装配**未**把任何逻辑放入 View：View 只改了标题字符串，窗口图标在入口层装配、`.qrc` 只提供资源。无需回退。
