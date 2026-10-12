# 24 ViewModel 接口（v0.3.0，增量）

> 前置：`22_View层接口.md`、`23_Model层接口.md`、`25_静态库API调用映射.md`
> 说明：VM 持 `View*`，集中 connect；经 View 公开 getter 读控件、公开方法更新 UI；不用 findChild。

## 接口（增量）

```cpp
// viewmodel/mainviewmodel.h（节选，仅列本版相关）
class MainViewModel : public QObject {
    Q_OBJECT
public:
    void install(MainWindow* view);              // 变更：创建并连接进度 QTimer
    // 其余公开方法沿用

private slots:
    void onStartScan();                          // 变更：重置原子变量 + 启动定时器 + 轻量回调
    void onScanFinished();                       // 变更：停止定时器
    void onProgressTick();                       // 新增：定时采样进度刷新界面（C-03）

private:
    QTimer* m_progressTimer = nullptr;           // 新增：进度定时采样（主线程）
    std::atomic<size_t> m_progDone{0};           // 新增：已完成区域数
    std::atomic<size_t> m_progTotal{0};          // 新增：总区域数
    std::atomic<size_t> m_progHits{0};           // 新增：命中累计
    std::atomic<uint32_t> m_progPid{0};          // 新增：当前 pid
};
```

## 关键行为

- `install()`（C-03）：`m_progressTimer = new QTimer(this); setInterval(150); setTimerType(Qt::CoarseTimer); connect(timeout → onProgressTick)`。
- `onStartScan()`（C-01/C-03/C-04）：
  - 重置进度原子变量（`done/total/hits/pid = 0`）；
  - `m_progressTimer->start()`；
  - 进度回调 `cb = [this](const ScanProgress& sp){ 仅写 4 个原子变量; }`（**不触碰界面**）；
  - `QtConcurrent::run(ScanProcesses(req, cb, cancelPtr))`（单个外层任务，并行在 psig 内部）。
- `onProgressTick()`（C-03）：主线程读取原子变量 → `pct = total ? done*100/total : 0` → `m_view->setProgress(pct)` + `m_view->setTransientMessage(tr("扫描 pid=%1 … 命中 %2"))`。
- `onScanFinished()`（C-03）：`m_progressTimer->stop()` → 结果入 Model → `setProgress(-1)` → 切结果页 → 反馈（沿用）。

## 约束满足说明

- VM 经 View 公开 getter/方法读写，不用 findChild；每业务方法对应真实 psig API。
- VM **不创建线程/线程池**；并行由 psig 提供；进度回调仅写原子变量。
