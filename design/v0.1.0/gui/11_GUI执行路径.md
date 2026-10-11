# 11 GUI 执行路径文档

- 版本：v0.1.0
- 前置：`10_GUI功能流程图.md`

> 每条路径给出：触发 → 信号/槽 → 处理 → 结果 → 界面变化。含正常、错误、取消三类。

## P1 导入特征码 — 正常
1. 用户点击工具栏 `导入特征码`（QAction::triggered）。
2. VM::onImportSignatures() 调用 QFileDialog::getOpenFileName（过滤器 `*.csv *.db`）。
3. 依据扩展名调用 `psig::ImportSignaturesCsv` / `psig::ImportSignaturesDb`。
4. 返回成功：VM 调 `Model.setRows(rows)`、`Model.setColumns(cols)`。
5. View：SignatureEditorPage 表格刷新；状态栏右侧显示"已导入 N 条"；日志追加。

## P2 导入特征码 — 错误
1–2 同 P1。
3. 静态库返回失败（格式错误/文件损坏）。
4. VM 调 `view->setTransientMessage("导入失败: ...", Error)`；决策型错误用 QMessageBox::warning。
5. 数据不变。

## P3 导入特征码 — 取消
1–2 同 P1。
3. QFileDialog 返回空字符串。
4. VM 直接 `return`，无副作用，无反馈（或状态栏"已取消"）。

## P4 导出特征码 — 正常
1. 点击 `导出特征码` → QFileDialog::getSaveFileName。
2. 调 `psig::ExportSignaturesCsv` / `ExportSignaturesDb`。
3. 成功 → 状态栏"已导出: path" + 日志。

## P5 导出特征码 — 错误 / P6 — 取消
- 错误：静态库返回失败 → 红色瞬时反馈。
- 取消：文件对话框返回空 → return。

## P7 新增行 — 正常
1. 点击 `新增行` → VM::onAddSignatureRow()。
2. `Model.addRow()`（默认新行）。
3. 表格新增一行并进入编辑；持久状态"特征码 M"更新。

## P8 删除行 — 正常
1. 选中一行或多行 → 点击 `删除行` 或右键"删除"。
2. VM 取 `selectionModel()->selectedRows()`。
3. `Model.removeRows(indices)` → 刷新 → 日志。

## P9 删除行 — 错误
1. 未选中任何行 → 点击删除。
2. 选择为空 → 状态栏"请先选择要删除的行"，不执行删除。

## P10 列管理 — 正常
1. 菜单 编辑→列管理 → ColumnManagerDialog 打开。
2. 用户增/删/改名/排序 → 确定。
3. VM 取回列定义 → `Model.setColumns()` → 表格列变化 → 日志。

## P11 列管理 — 取消
- 对话框 Rejected → 不改动，关闭。

## P12 刷新进程 — 正常
1. 点击 `刷新进程` → VM::onRefreshProcesses()。
2. `QtConcurrent::run(psig::EnumerateProcesses)`；QFutureWatcher。
3. finished → `Model.setProcesses(list)` → 进程表刷新 → 持久状态"进程 N" + 日志。

## P13 刷新进程 — 错误
- 枚举返回空/异常 → 状态栏错误反馈或弹窗；列表保持。

## P14 开始扫描 — 正常
1. 点击 `开始扫描`。
2. 校验（见 P15）通过。
3. 持久状态"扫描中"；进度条显示。
4. `QtConcurrent::run(psig::ScanProcesses(...))`；进度回调经信号回主线程更新进度条。
5. finished → `Model.setResults(results)` → 自动切 ResultPage → 持久状态"就绪"；"命中 H 条" + 日志。

## P15 开始扫描 — 错误
- 特征码表为空 / 未选进程 / 模式串非法 → 状态栏红色反馈（或决策弹窗）→ 不启动扫描。

## P16 开始扫描 — 取消
1. 扫描进行中点击 `取消扫描`。
2. VM 调 `psig::RequestCancel()`（或 token 置位）。
3. 后台尽快返回已收集结果。
4. finished(partial) → 展示已得结果 → 状态"已取消" + 日志。

## P17 导出结果 — 正常 / P18 — 错误 / P19 — 取消
- 正常：结果非空 → 选择文件 → `psig::ExportResults{Csv,Db,Txt}` → "已导出"。
- 错误：结果为空 → "无结果可导出"；或写失败 → 红字。
- 取消：文件对话框空 → return。

## P20 清空结果 — 正常 / P21 — 取消
- 正常：点击清空 → QMessageBox::question 确认 → `Model.clear()` → 反馈。
- 取消：弹窗选否 → 不清空。

## P22 清空日志
- 点击/右键清空 → View 日志区清空（不影响状态栏）。

## 约束满足说明

- 每条执行路径均含正常/错误/取消三类（事件驱动独立路径）。
