# 10 GUI 功能流程图（v0.3.0，增量）

> 模式：修改型。仅列出本轮新增/变更的 Qt 信号→槽 触发路径；未列出者沿用 v0.2.4 `10_GUI功能流程图.md`。

## 扫描进度（C-01/C-03/C-04）

- 开始扫描：`m_startScanAction::triggered` → `MainViewModel::onStartScan`：
  1. 校验/收集 `patterns`、`selectedPids()`；
  2. 重置进度原子变量（`done=0/total=0/hits=0/currentPid=0`）；
  3. 启动进度 `QTimer`（约 100~200ms）；
  4. `m_scanWatcher->setFuture(QtConcurrent::run([...]{ return psig::ScanProcesses(req, cb, cancel); }))`。
- 进度回调 `cb`（**后台线程**）：只写原子变量（`done/total/hits/currentPid`），**不投递界面事件**。
- 进度定时器：`QTimer::timeout` → 主线程读原子变量 → `pct = total ? done*100/total : 0` → `m_view->setProgress(pct)` + `m_view->setTransientMessage("扫描 pid=… 命中 N")`。
- 扫描完成：`m_scanWatcher::finished` → `onScanFinished` → `timer->stop()` → 结果填入 `ResultTableModel` → `setProgress(-1)` → 切结果页 → 日志/瞬时反馈。

## 扫描执行（C-02）

- `psig::ScanProcesses(req, cb, cancel)` 内部（**无 Qt 信号**）：
  - 预统计总工作量（所有目标进程可读区域数之和）；
  - 按区域并行扫描（工作线程池/`std::thread`，并行度按 CPU）；
  - 每完成一个区域：原子累加 `done`、汇总命中、按粗粒度触发 `cb`；
  - 全部完成后合并各线程局部结果 → 返回。

## 取消（沿用语义）

- `m_cancelScanAction::triggered` → `onCancelScan` → `m_cancel.store(true)` + `psig::RequestCancel(true)`；所有工作线程轮询取消标志后尽快退出。

## 不受影响

- 结果导出、特征码导入/导出、进程枚举、表格悬停等路径沿用 v0.2.4 `10_GUI功能流程图.md`。
