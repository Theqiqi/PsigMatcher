# 10 GUI 功能流程图

- 版本：v0.1.0
- 前置：`05_GUI功能列表与窗口定义.md`

> 说明：本程序采用"控件 getter + ViewModel connect"风格，ViewModel 在 `install(MainWindow*)` 中集中 connect 控件信号。以下触发路径以控件信号为起点。

## 1. 导入特征码（F09）

```
[工具栏 导入特征码 clicked]
  → VM::onImportSignatures()
  → QFileDialog 选择文件
  → 判断扩展名:
       .csv → psig::ImportSignaturesCsv()
       .db  → psig::ImportSignaturesDb()
       else → 错误路径
  → 正常: 成功 → Model.setRows(...) → View 刷新 → 状态栏反馈 + 日志
  → 错误: 解析失败 → 状态栏红色瞬时反馈 / 决策型错误弹窗
  → 取消: QFileDialog 返回空 → 直接 return，无副作用
```

## 2. 导出特征码（F10）

```
[工具栏 导出特征码 clicked] → VM::onExportSignatures()
  → 选择目标文件 (.csv/.db)
  → psig::ExportSignaturesCsv()/ExportSignaturesDb()
  → 成功: 状态栏"已导出" + 日志
  → 失败: 瞬时反馈错误
  → 取消: 直接 return
```

## 3. 新增/删除特征码行（F01/F03/F04）

```
[新增行 clicked] → VM::onAddSignatureRow()
  → Model.addRow() → View 刷新 → 持久状态"特征码 M"更新

[删除行 clicked] 或 [表格 contextMenu 删除] → VM::onDeleteSelectedRows()
  → 取 selectionModel().selectedRows()
  → 空选择 → 瞬时反馈"未选择行"（错误路径）
  → 有选择 → Model.removeRows() → View 刷新 → 日志
```

## 4. 列管理（F05–F08）

```
[列管理 clicked] → VM::onManageColumns()
  → 打开 ColumnManagerDialog（用当前列定义初始化）
  → 对话框 exec()
  → Accepted → 取回列定义 → Model.setColumns() → View 刷新 → 日志
  → Rejected/取消 → 不改动
```

## 5. 刷新进程（F11）

```
[刷新进程 clicked] → VM::onRefreshProcesses()
  → QtConcurrent::run { psig::EnumerateProcesses() }
  → QFutureWatcher finished → 主线程:
       成功 → Model.setProcesses(...) → 持久状态"进程 N"
       失败 → 瞬时反馈/错误弹窗
  （后台异常经信号/QFuture::onFailed 回主线程，不跨线程抛）
```

## 6. 开始扫描（F13）/ 取消（F14）

```
[开始扫描 clicked] → VM::onStartScan()
  → 校验: 特征码表非空？目标进程非空？模式串合法？
       不合法 → 瞬时反馈错误（错误路径），return
  → 持久状态"扫描中"；禁用开始、启用取消
  → QtConcurrent::run { psig::ScanProcesses(targets, patterns, progressCb) }
  → 进度信号 → 状态栏进度条更新（QueuedConnection 回主线程）
  → finished → Model.setResults(...) → 自动切 ResultPage → 持久状态"就绪"
  → 命中 H → 状态栏 + 日志

[取消扫描 clicked] → VM::onCancelScan()
  → psig 取消标志置位 → 后台尽快返回 → finished(部分结果) → 状态"已取消"
```

## 7. 导出结果（F17–F19）

```
[导出CSV/DB/TXT clicked] → VM::onExportResult(fmt)
  → 结果集为空 → 瞬时反馈"无结果"（错误路径）
  → QFileDialog → psig::ExportResultsCsv/Db/Txt()
  → 成功/失败/取消 同导出特征码
```

## 8. 清空结果（F20）/ 日志（F22）

```
[清空 clicked] → VM::onClearResults() → 决策弹窗确认 → Model.clear() → 反馈
[清空日志 clicked] → VM::onClearLog() → View 日志区清空
```

## 9. 事件驱动路径分类汇总

| 功能 | 正常路径 | 错误路径 | 取消路径 |
| --- | --- | --- | --- |
| 导入 | 解析成功入库 | 格式/解析失败 | 文件对话框取消 |
| 导出 | 写文件成功 | 写失败/无权限 | 文件对话框取消 |
| 扫描 | 完成出结果 | 校验失败/读取失败 | 用户取消扫描 |
| 刷新进程 | 枚举成功 | 枚举失败 | 不适用 |
| 列管理 | 确认应用 | — | 对话框取消 |
| 清空 | 确认清空 | — | 弹窗取消 |

## 10. 约束满足说明

- 含 Qt 信号/槽触发路径；执行路径覆盖正常/错误/取消三类（事件驱动独立路径）。
