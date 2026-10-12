# 10 GUI 功能流程图（v0.2.4，增量）

> 模式：修改型。仅列出本轮新增/变更的 Qt 信号→槽 触发路径；未列出者沿用 v0.2.2 `10_GUI功能流程图.md`。

## 进程列表名称（C-01）
- 进入进程页 / 刷新 → `onRefreshProcesses` → `psig::EnumerateProcesses()`（Toolhelp `szExeFile` 优先，路径兜底）→ `sigProcessesEnumerated` → `ProcessTableModel::setRows` → 名称列显示真实名（无「未知进程」）。
- 无新增信号/槽；仅 psig 取值来源变更。

## 表格整行悬停（C-02）
- 纯绘制路径，无信号：`QTableView` 收到鼠标进入/离开 → 触发 `repaint` → 自定义 `QStyledItemDelegate::paint` 检测 `State_MouseOver` → 对整行绘制淡色背景。
- 不发射信号、不改 Model、不写日志。

## 结果导出随语言（C-03）
- 结果页「导出结果」→ `onExportResultsCsv/Txt` → `exportResultsToPath` → 构造 `psig::ResultExportOptions{headers=翻译后表头, summary=翻译后摘要}` → `psig::ExportResultsCsv/Txt` → 写文件 → 日志/瞬时反馈（随语言）。

## 特征码导出/导入解耦（C-04）
- 导出：特征码页「导出」→ `onExportSignatures` → `tableFromModel()`（**不取**结果页 `m_templateText`）→ `psig::ExportSignaturesDb` → 文件首行为列头。
- 导入：特征码页「导入」→ `onImportSignatures` → `psig::ImportSignaturesDb` → `applyTableToModel()`（**不写**结果页模板文本）→ 特征码表刷新。

## 导出格式入口收敛（C-05）
- 特征码导入/导出：文件对话框过滤器 `*.db`（移除 `*.csv`）。
- 结果导出：文件对话框过滤器 `*.csv` / `*.txt`（移除 `*.db`）。
- psig 侧 `ImportSignaturesCsv`/`ExportSignaturesCsv`/`ExportResultsDb` 保留，仅无界面入口。
