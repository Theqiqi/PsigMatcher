# 20 GUI 模块分析（v0.3.0，增量）

> 前置：`04_GUI需求分析.md`、`14_API能力需求.md`、`design/v0.2.4/gui/20_GUI模块分析.md`（上一版本）
> 说明：标注本次改动涉及的层与文件；未涉及模块引用 v0.2.4 文档。

## 1. 四层结构（不变）

```
View（只画 UI，可单独编译）  →  Model（QAbstractTableModel，纯数据）  →  ViewModel（QObject，持 View*，集中 connect，调 psig）  →  静态库 psig
```

## 2. 改动涉及的层与文件

| 层 | 文件 | 变更点 | 对应 |
| --- | --- | --- | --- |
| ViewModel | `viewmodel/mainviewmodel.h` | 新增 `QTimer* m_progressTimer`、进度原子变量（`m_progDone/Total/Hits/Pid`）、槽 `onProgressTick()` | C-03、C-04 |
| ViewModel | `viewmodel/scan.cpp` | `onStartScan`：重置原子变量 + 启动定时器；回调改为**只写原子变量**；新增 `onProgressTick()` 定时采样刷新；`onScanFinished` 停止定时器 | C-01、C-03、C-04 |
| ViewModel | `viewmodel/mainviewmodel.cpp` | `install()` 创建并连接 `m_progressTimer`（150ms，`Qt::CoarseTimer`）；版本串 v0.3.0 | C-03 |
| View | `view/mainwindow.h` | 新增 `bool m_transientError`（记录瞬时标签错误态） | C-04 |
| View | `view/mainwindow.cpp` | `setTransientMessage`：仅在**错误态变化**时 `setStyleSheet`；版本串 v0.3.0 | C-04 |
| ViewModel | `viewmodel/result.cpp` | 版本串 v0.3.0（关于对话框） | — |
| 静态库 | `psig` | 已由静态库流程修正（进度真实化、区域并行、回调粒度） | C-01、C-02、C-04 |

## 3. 未涉及模块（引用 v0.2.4）

- `model/*`（进程/特征码/结果模型均无改动）。
- `view/*` 除 `mainwindow.{h,cpp}` 外（进程/特征码/结果页、各对话框、`hoverrowdelegate`）无改动。
- `viewmodel/{bind,process,signature,result,language,viewmodel_helpers}` 除上述文件外无改动。
- `src/main.cpp`、`gui/CMakeLists.txt`、`src/CMakeLists.txt`、i18n 机制（本次仅更新版本串翻译条目）。

## 约束满足说明

- GUI 分为 View/Model/ViewModel/静态库 4 层；View 只画 UI 可单独编译。
- 已标注改动涉及的层与文件；未涉及模块引用 v0.2.4 文档。
- 并行能力在 psig 内部实现，GUI 不建线程池、不调用平台 API。
