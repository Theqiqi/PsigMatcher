# 10 GUI 功能流程图（v0.2.2，增量）

> 仅列出本轮新增/变更的 Qt 信号→槽 触发路径。

## 列管理下拉（C-02）
列管理动作 → ColumnManagerDialog（键/类型为 QComboBox）→ accepted → onManageColumns → sigModel->setColumns(dlg.columns())。

## 重新选择进程保存提示（C-04）
reselectProcessAction::triggered → onReselectProcess → QMessageBox(保存/不保存/取消)
  - 保存 → exportSignaturesToPath() → setProcessSelectionLocked(false) + showProcessPage()
  - 不保存 → setProcessSelectionLocked(false) + showProcessPage()
  - 取消 → 返回（停留在当前页）

## 日志开关与右键（C-08）
视图菜单「历史日志」action::toggled → m_logDock->setVisible(checked)
m_logEdit::customContextMenuRequested → 弹出菜单（含 m_clearLogAction）→ onClearLog → m_view->clearLog()

## 权限图标（C-09）
onProcessesEnumerated → startIconLoad：仅 accessible 进程请求 psig::ProcessIcon/DefaultProcessIcon → applyProcessIcon(row, icon)（无权限行跳过，DecorationRole 空）

## 导出（C-01/C-05/C-06）
导出结果动作 → ExportColumnsDialog（含 address_dec）→ onExportResults → exportResultsToPath → psig::ExportResultsCsv(opt{columns, headers})
导出特征码动作 → onExportSignatures → tableFromModel(翻译 builtin 标题) → psig::ExportSignaturesCsv
