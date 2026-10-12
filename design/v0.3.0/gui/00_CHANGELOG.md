# v0.3.0 CHANGELOG

> **v0.3.0 = v0.2.4 + 扫描进度修复 / 区域并行扫描 / 定时进度采样 / 回调削减**
>
> 本版本为「修改/添加」型流程，输入为 `.idea/v0.3.0.md`。
> 上一版本为 `v0.2.4`（tag: `v0.2.4`，design/v0.2.4/）。已归档版本目录不再原地修改。
> 本次改动全部写入 `design/v0.3.0/`。

## 版本号

- 沿用 v0.2.x 起算约定：修改型流程按次版本 +1（`v0.2.4 → v0.3.0`）。
- 本流程版本号：**v0.3.0**。

## 改动点清单（初稿，详见 `gui/01_想法分析.md`）

| 编号 | 改动点（原始想法） | 类型 | 影响层 |
| --- | --- | --- | --- |
| C-01 | 扫描进度条恒为 100%（`sp.done = total; sp.total = total;`）→ 修复为真实已完成量/总量 | 修复 | psig + GUI |
| C-02 | 扫描为进程×区域双重串行 → 按内存区域分块，交多线程并行扫描 | 改进 | psig |
| C-03 | 每读 1MiB 即刷新界面 → 改用定时器（约 100~200ms）定期采样进度 | 改进 | GUI |
| C-04 | 每次进度回调都跨线程投递界面事件 + 每次 `setStyleSheet` → 工作线程只更新原子变量，界面定时读取 | 改进 | GUI |
| C-05 | 明确**不**新增「可读/可写/可执行区域过滤」选项（可读必需；写/执行过滤有漏报风险） | 范围界定 | — |

## 范围界定（不做的事）

- 不新增按可读/可写/可执行过滤区域的选项（C-05）。
- 不改变扫描语义（命中判定、结果字段、导出格式均不变）。
- 不新建窗口/页面，不改技术栈（Qt6::Widgets / Qt6::Concurrent / MSVC+Ninja / vcpkg）。

## 交付结果（流程完成）

- **psig 改动**：`src/api/process_scan.cpp`（`ScanProcesses` 重构为「规划 → 区域级并行 → 汇合」；进度真实化 `done`/`total`=区域工作量；回调按区域触发）；`include/psig.h`（`ScanProgress` 语义注释、`ProgressCallback` 并发契约、`ScanProcesses` 并行说明；版本 v0.3.0）；`test/integration/test_scan.cpp`（+3 用例）。
- **GUI 改动**：`viewmodel/mainviewmodel.{h,cpp}`（`QTimer` + 进度原子变量 + `onProgressTick`）；`viewmodel/scan.cpp`（回调只写原子变量、定时采样、`onScanFinished` 停止定时器）；`view/mainwindow.{h,cpp}`（`setTransientMessage` 仅在错误态变化时重设样式表）；版本串 v0.3.0；i18n `.ts` 同步。
- **构建/测试**：
  - psig Debug/Release 编译通过；`psig_unit`（42 例）+ `psig_integration`（21 例）全绿（各 2/2 目标）。
  - GUI Debug/Release 编译通过；`psig_test_{view,model,viewmodel,mvvm,userflow}` 全绿（各 5/5）。
  - E2E：`verify_all.sh Release` 7/7 通过（截图证实版本号 v0.3.0）。
- **审查**：View/Model/ViewModel/静态库调用/横切 5 张矩阵全部通过（无 ❌）；psig data/algorithm/atomic/api 4 层审查（前三层未受影响跳过，api 层通过）。
- **tag**：`v0.3.0`（annotated，指向最终提交）。
- **不做的事**：不新增窗口/页面、不改技术栈、不改变扫描语义（命中判定/结果字段/导出格式）、不新增区域权限过滤选项、不删除 psig 公共 API。
