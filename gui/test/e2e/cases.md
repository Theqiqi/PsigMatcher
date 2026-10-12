# PsigMatcher E2E 测试用例清单

- 版本：v0.3.1
- 前置：`design/v0.3.1/gui/11_GUI执行路径.md`、`12_用户流程.md`、`06_交互模式设计.md`、`05_GUI功能列表与窗口定义.md`、`08_窗口层次结构与导航图.md`
- 产物对应：`drive.ps1`（驱动）、`analyze.py`（断言）、`verify_all.sh`（编排）
- 说明：每个用例的操作步骤均对应真实可执行入口（菜单/工具栏/快捷键）；驱动脚本按步骤注入，断言脚本按预期断言，脚本与用例一一对应。

## 入口与注入方式

- 程序：`gui/out/bin/Release/PsigMatcher.exe`（静态 Qt，自包含）。
- 注入：`keybd_event`（Win32 原始输入）注入快捷键；`PostMessage(WM_CLOSE)` 关闭窗口。
- 截图：客户区（排除标题栏），保存至 `gui/out/verify/shot_xx.png`。
- 快捷键入口：Ctrl+1/2/3 切页（视图菜单）、Ctrl+N 新增行、Ctrl+R 刷新进程、F5 扫描、Ctrl+M 列管理、Ctrl+O/Ctrl+S 导入/导出、F1 关于。

## v0.3.1 变更点（相对 v0.3.0）

- C-01：悬停高亮不再越界覆盖相邻列——委托（`HoverRowDelegate`）改为在 `initStyleOption` 内只对自身单元格设底色（不再扩展绘制矩形到整行宽）。
- C-02：左键单击后，前导列（PID/Name 等）随整行同步变色，无需移动鼠标——由 `HoverRowTableView` 跟踪悬停行并在变化时重绘受影响的行。
- C-03：悬停不再遮盖前导列字体——同一越界根因，随 C-01 一并修复。
- C-04：进程表左上角空白（角落按钮）点击不再全选所有行——`setCornerButtonEnabled(false)`。
- C-05：特征码 `.db` 记录目标进程名；导入时若与当前目标进程不一致则提示，用户可选「继续导入」或「取消」。
- 窗口标题更新为「进程特征码批量匹配器 v0.3.1」（默认跟随系统语言 → 英文显示 `Process Signature Batch Matcher v0.3.1`）。
- 未新增 E2E 用例；C-01/C-02/C-03（整行悬停高亮、不越界、不遮盖前导列）由 GUI 单元测试 `View.HoverRowHighlightsWholeRowWithinCells`（真实 QTableView + 委托 + 模型，像素采样断言整行高亮且限定在该行）覆盖；C-04 由 `View.ProcessPageCornerButtonDisabled` 覆盖；C-05 由 `ViewModel.ImportTargetProcessMismatchPrompts` + psig `test_signatures_exchange` 覆盖。

## v0.3.0 变更点（相对 v0.2.4）

- C-01：扫描进度条显示**真实进度**（不再恒 100%）；由状态栏进度条随扫描推进平滑增长至 100%。
- C-02：扫描改为**区域级并行**（内部实现），同一任务更快；界面无可见变化。
- C-03：进度刷新改为**定时器采样**（约 150ms），界面刷新与扫描循环解耦。
- C-04：工作线程进度回调只写原子变量；`setTransientMessage` 仅在错误态变化时重设样式表。
- C-05：**不新增**按可读/可写/可执行过滤区域的选项。
- 窗口标题更新为「进程特征码批量匹配器 v0.3.0」（默认跟随系统语言 → 英文显示 `Process Signature Batch Matcher v0.3.0`）。
- 未新增 E2E 用例；进度/并行改动的非 E2E 部分由 psig 集成测试覆盖（`ScanProgressReportsRealWork`、`ScanParallelFindsMarkerBothRuns`），详见 `design/v0.3.0/gui/29~33` 审查矩阵与 `design/v0.3.0/gproto/77_静态库完成契约.md`。

## v0.2.4 变更点（相对 v0.2.3）

- C-01：进程列表名称改为真实进程名（Toolhelp 快照名优先），**不再出现** `Unknown Process` / `未知进程`（在 `shot_01_start.png` / `shot_02_process_refreshed.png` 可见）。
- C-02：三张表（进程/特征码/结果）鼠标悬停**整行**淡色高亮（选中色同色系更淡），单击选中仍为深色；不改变选中语义。
- C-03：结果导出（CSV/TXT）表头与 TXT 摘要随界面语言（英文 UI → `Note`/`Results (N)`）。
- C-04：特征码导出/导入**不再携带**结果页模板文本；导入不覆盖结果页模板。
- C-05：特征码导入/导出仅 `.db`；结果导出仅 `.csv`/`.txt`（移除 `.db` 入口）。
- 窗口标题更新为「进程特征码批量匹配器 v0.2.4」（默认跟随系统语言 → 英文显示 `Process Signature Batch Matcher v0.2.4`）。
- 未新增 E2E 用例；上述改动点的非 E2E 部分由 GUI 单元测试 + psig 集成测试覆盖（详见 `design/v0.2.4/gui/29~33` 审查矩阵与 `design/v0.2.4/gproto/77_静态库完成契约.md`）。

## v0.2.3 变更点（相对 v0.2.2）

- ViewModel 单类多文件拆分（`mainviewmodel.{h,cpp}` + `bind/process/signature/scan/result/language.cpp`，去类名前缀）；psig api 层按职责拆分。均为内部结构，无界面行为变化。
- 窗口标题更新为「进程特征码批量匹配器 v0.2.3」。

## v0.2.2 变更点（相对 v0.2.1）

- C-03：进程列表中空名进程（如 PID 0/4 系统进程）显示兜底名（该兜底已在 v0.2.4 由 C-01 移除）。
- C-07：进程页列宽调整 — PID = 基准、Name = PID×2、Accessible = PID、Path 占剩余（仍可手动拖动）。
- C-08：视图菜单新增「日志」开关（显隐 History Log dock）；「清空日志」改为仅在日志窗口右键菜单中。
- C-09：仅对 `accessible==true` 的进程异步取图标；无权限进程不显示图标（不阻止选中）。
- 窗口标题更新为「进程特征码批量匹配器 v0.2.2」（默认跟随系统语言 → 英文显示 `Process Signature Batch Matcher v0.2.2`）。

## v0.2.1 变更点（相对 v0.2.0）

- C-09：进程列表**启动即自动枚举**一次（无需手动刷新）；筛选框由顶部移到底部。
- C-02：新增中/英界面语言与「语言」菜单（位于「视图」与「帮助」之间）。
- C-12：进程页名称列显示进程图标（缺省用系统默认图标）。
- C-13/C-14：整体统一白底基调，日志按级别着色点缀。
- C-07：Release 版无控制台窗口（WIN32 子系统）；Debug 保留。
- 窗口标题更新为「进程特征码批量匹配器 v0.2.1」。

> 说明：语言切换（C-02）无独立快捷键，其正确性由单元测试（`ViewModel.ApplyLanguageSwitchesText`、`ViewModel.SignatureHeadersFollowLanguageExceptCustom`）与集成测试覆盖；E2E 侧重真实窗口渲染与流程联通。

---

## E2E-01 启动主界面（进程页）

- 场景：冷启动，主窗口渲染，默认落在进程选择页（启动自动枚举进行中）。
- 前置：已编译 `PsigMatcher.exe`；无残留实例。
- 操作步骤：
  1. 启动 `PsigMatcher.exe`。
  2. 等待主窗口句柄就绪并置前。
  3. 截取客户区 → `shot_01_start.png`。
- 预期结果：窗口标题为「进程特征码批量匹配器 v0.3.1」；客户区尺寸 ≥ 600×400；画面非黑屏（平均亮度 > 30）；含 UI 元素（非背景像素占比 > 2%）。

## E2E-02 进程表被填充（C-09 启动自动枚举 / Ctrl+R）

- 场景：启动后自动枚举系统进程并填充进程表（可再按 Ctrl+R 刷新）。
- 前置：E2E-01 通过。
- 操作步骤：
  1. 按 Ctrl+R → 触发异步枚举，等待 2.8s。
  2. 截图 `shot_02_process_refreshed.png`。
- 预期结果：相对启动画面存在差异（差异像素比例 > 0.5%）**或**进程页内容明显增多（非背景像素占比 > 6%），证明进程表被数据填充。

## E2E-03 页面导航（视图菜单）

- 场景：在特征码编辑页 / 匹配结果页之间切换（进程页为启动落点）。
- 前置：E2E-01 通过。
- 操作步骤：
  1. 按 Ctrl+1 → 特征码编辑页，截图 `shot_03_signature.png`。
  2. 按 Ctrl+3 → 匹配结果页，截图 `shot_05_result.png`。
- 预期结果：特征码页与启动（进程）页画面不同、结果页与特征码页画面不同（差异像素比例 > 1%），证明页面确实切换。

## E2E-04 新增特征码行（编辑菜单 / Ctrl+N）

- 场景：在特征码编辑页新增一行。
- 前置：处于特征码编辑页。
- 操作步骤：
  1. 按 Ctrl+N → 表格新增一行。
  2. 截图 `shot_04_addrow.png`。
- 预期结果：`shot_04_addrow.png` 与新增前的 `shot_03_signature.png` 存在差异（差异像素比例 > 0.1%），证明表格内容变化。

## E2E-05 列管理对话框（编辑菜单 / Ctrl+M）

- 场景：打开列管理对话框并取消。
- 前置：任意页面。
- 操作步骤（人工/可选，自动化默认不纳入必过断言）：
  1. 按 Ctrl+M → 弹出列管理对话框。
  2. 按 Esc 取消。
- 预期结果：弹出对话框后主窗口被模态遮挡；取消后回到主界面；内部按钮（添加/删除/上移/下移）与 OK/Cancel 均生效（由单元/集成测试覆盖）。

## E2E-06 关于弹窗（帮助菜单 / F1）

- 场景：按 F1 弹出关于对话框，确认其为模态弹窗而非状态栏文本。
- 前置：处于匹配结果页（`shot_05_result.png` 已截取）。
- 操作步骤：
  1. 按 F1 → 弹出 `AboutDialog`（模态）。
  2. 截图 `shot_06_about.png`。
  3. 按 Esc 关闭对话框。
- 预期结果：`shot_06_about.png` 与 `shot_05_result.png` 存在差异（差异像素比例 > 0.4%），证明弹窗确实覆盖在主窗口之上；OK 按钮可关闭（由 View/集成测试覆盖）。

## E2E-07 关闭程序

- 场景：正常退出。
- 前置：程序运行中（关于弹窗已关闭）。
- 操作步骤：
  1. 向主窗口发送 `WM_CLOSE`。
  2. 等待进程退出（≤ 8s）。
- 预期结果：进程正常退出，退出码为 `0`（写入 `out/verify/exit_code.txt`）。

---

## 权限说明

以上用例均操作自身进程/界面，不需要管理员权限。详见 `design/v0.1.0/gui/33_admin_privilege_guide.md`。

## 运行方式

```bash
bash gui/test/e2e/verify_all.sh          # 默认 Release
bash gui/test/e2e/verify_all.sh Debug    # 指定配置
```
