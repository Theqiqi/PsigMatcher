# 09 Qt Model/View 数据绑定（v0.3.0，增量）

> 模式：修改型。分层绑定沿用：View 持控件、Model 纯数据、ViewModel 经 `View*` 操控并集中 `connect`。仅列出受影响绑定。

## 扫描进度绑定（C-01/C-03/C-04）

- **工作线程侧**：`psig::ScanProcesses` 的进度回调**只更新原子变量**（如 `std::atomic<uint64_t> done/total`、`std::atomic<size_t> hits`、`std::atomic<uint32_t> currentPid`），**不**调用 `QMetaObject::invokeMethod`、不触碰任何 Qt 对象。
- **主线程侧**：`MainViewModel` 持一个 `QTimer`（约 100~200ms，`Qt::CoarseTimer`）：
  - 扫描开始：`timer->start()`；
  - 每次 `timeout`：读取原子变量 → `pct = total ? done*100/total : 0` → `m_view->setProgress(pct)` + `m_view->setTransientMessage(扫描 pid=… 命中 N)`；
  - 扫描结束（`onScanFinished`）：`timer->stop()` → `setProgress(-1)`。
- **绑定方向不变**：View → ViewModel → Model；进度经 View 公开方法（`setProgress` / `setTransientMessage`）刷新，ViewModel 不直接操作控件内部。

## 进度条控件（`MainWindow`）

- 控件（不变）：状态栏 `QProgressBar`（中，`range 0..100`，`setProgress(-1)` 隐藏）+ `m_transientLabel`（右）。
- **样式表优化（C-04）**：`setTransientMessage` 仅在**错误态发生变化**时 `setStyleSheet`；连续同态刷新不重设样式表。

## 其余表绑定（不变，引用 v0.2.4）

- 进程表、特征码表、结果表的列定义、导出/导入过滤器、悬停 delegate：沿用 `design/v0.2.4/gui/09_Qt_ModelView绑定.md`。

## 静态库调用映射

- 扫描：`psig::ScanProcesses(req, progress, cancel)` —— `progress` 回调改为**写原子变量**的轻量函数（C-04）；进度语义改为真实工作量（C-01）；内部按区域并行（C-02）。
- 取消：`psig::RequestCancel(true)`（沿用）。
