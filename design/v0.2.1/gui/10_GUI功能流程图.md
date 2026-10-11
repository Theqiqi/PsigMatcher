# 10 GUI 功能流程图（v0.2.0，增量）

> 前置：`05_GUI功能列表与窗口定义.md`、`design/v0.1.0/gui/10_GUI功能流程图.md`（基线）
> 说明：仅给出**新增/修改功能**的 Qt 信号/槽触发路径；未列出的沿用 v0.1.0。

## 1. 通用约定

- View 只暴露控件 getter 与公开 UI 方法，**不写 connect**；全部 `connect` 在 ViewModel::install 集中完成。
- 元素级操作：View 提供 `QMenu* xxxContextMenu()` getter，其 `QAction` 由 View 创建，VM 连接 `triggered`。

## 2. 功能流程图（信号/槽路径）

### F-03 确认选择进程（C-04）
```
[双击进程行] QTableView::doubleClicked  ─┐
[工具栏 确认选择] QAction::triggered     ─┼─► VM::onConfirmProcessSelection
[右键 以此进程为目标] QAction::triggered ─┘        │
                                                    ├─ 校验：是否选中进程行？
                                                    │    否 → 瞬时反馈「请先选择目标进程」(错误路径)
                                                    └─ 是 → 记录目标 pid
                                                            → view->setProcessSelectionLocked(true)（隐藏进程页入口/进程动作，显示 重新选择进程）
                                                            → view->showSignaturePage()
                                                            → 状态/日志更新
```

### F-04 重新选择进程（C-04）
```
[工具栏/视图菜单 重新选择进程] QAction::triggered → VM::onReselectProcess
    → view->setProcessSelectionLocked(false) → view->showProcessPage() → 瞬时/日志
```

### F-02 按页工具栏（C-03）
```
页面切换（showSignaturePage/showProcessPage/showResultPage）
    → MainWindow::applyToolbarForPage(page)  // View 内部：显示本页动作组，隐藏他组
    （VM 在导航时调用，View 仅执行显隐）
```

### F-05 特征码行右键菜单（C-02）
```
[表格 右键] → QMenu::exec（View 构建）
    ├─ 新增行       QAction::triggered → VM::onAddSignatureRow
    ├─ 删除选中行   QAction::triggered → VM::onDeleteSignatureRows
    └─ 列管理       QAction::triggered → VM::onManageColumns
```

### F-06/F-07 结果页右键菜单 + 删除（C-05/C-06）
```
[表格 右键] → QMenu::exec
    ├─ 导出 ▸ CSV/DB/TXT  QAction::triggered → VM::onExportResultsCsv/Db/Txt
    ├─ 选择导出列…         QAction::triggered → VM::onSelectExportColumns
    ├─ 删除选中行          QAction::triggered → VM::onDeleteResultRows
    └─ 清空结果            QAction::triggered → VM::onClearResults
```

### F-08 末列自定义编辑（C-08）
```
[双击单格编辑] QTableView::edit → ResultTableModel::setData(col=9) → dataChanged
[右键 批量填充] QAction::triggered → VM::onFillCustomColumn
    → 取选中行（或全部）→ ResultTableModel::setCustom(rows, text) → dataChanged → 瞬时反馈
```

### F-09 模板文本编辑（C-07）
```
[模板区编辑] QPlainTextEdit::textChanged → VM::onTemplateTextChanged → 暂存 m_templateText（表级元数据）
（导出时随结果携带；导入特征码表时回填）
```

### F-10 选择导出列（C-07）
```
[右键 选择导出列…] QAction::triggered → VM::onSelectExportColumns
    → 打开列勾选对话框（View 提供）→ 记录 m_exportColumns → 瞬时反馈「将导出 N 列」
```

### F-12 关于（C-10）
```
[帮助→关于] QAction::triggered → VM::onAbout → AboutDialog dlg(view); dlg.exec()
```

### F-13 导入前置询问（C-11）
```
[导入触发] QAction::triggered → VM::onImportSignatures
    ├─ 表为空 → 直接选文件
    └─ 表非空 → QMessageBox::question(保存/不保存/取消)
                 ├─ 保存   → 走导出特征码流程（用户取消保存则中止导入）
                 ├─ 不保存 → 继续选文件
                 └─ 取消   → 中止（取消路径）
    → 选文件 → VM::importSignaturesFromPath → psig::ImportSignatures* → 应用表 + 模板回填
```

## 3. 约束满足说明

- 已用 Qt 信号/槽触发路径表达各功能流程；connect 归属 ViewModel（View 零 connect）。
- 各功能的正常/错误/取消三类路径见 `11_GUI执行路径.md`。
