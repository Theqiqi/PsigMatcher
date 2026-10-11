# 10 GUI 功能流程图（v0.2.1，增量）

> 前置：`05_GUI功能列表与窗口定义.md`、`design/v0.2.0/gui/10_GUI功能流程图.md`（基线）
> 说明：仅给出**新增/修改功能**的 Qt 信号/槽触发路径；未列出的沿用 v0.2.0。

## 1. 通用约定

- View 只暴露控件 getter（含对话框 `QDialogButtonBox* buttonBox()`）与公开 UI 方法，**不写 connect**；全部 `connect` 在 `ViewModel::install` 集中完成。
- 对话框按钮：View 暴露 `buttonBox()`；VM 在 `exec()` 前连接 `accepted→dlg.accept`、`rejected→dlg.reject`（修复 C-01/C-04/C-05）。

## 2. 功能流程图（信号/槽路径）

### F-03 对话框按钮生效（C-01 / C-04 / C-05）
```
[VM 创建对话框] dlg(view)
    → connect(dlg.buttonBox(), &QDialogButtonBox::accepted, &dlg, &QDialog::accept)
    → connect(dlg.buttonBox(), &QDialogButtonBox::rejected, &dlg, &QDialog::reject)
    → dlg.exec()
        ├─ 接受 → QDialog::Accepted → VM 读取结果并应用
        └─ 取消 → QDialog::Rejected → VM 放弃

列管理内部按钮（ColumnManagerDialog）：
    [添加列]   QPushButton::clicked → VM 槽：table 追加一行空列
    [删除列]   QPushButton::clicked → VM 槽：删除选中行
    [上移]     QPushButton::clicked → VM 槽：选中行上移
    [下移]     QPushButton::clicked → VM 槽：选中行下移
```
> 说明：列管理内部按钮的 `clicked` 由 VM 连接（View 零 connect），直接操作对话框内的 `QTableWidget`。

### F-01 / F-02 界面语言切换（C-02）
```
[菜单 语言→中文/English/跟随系统] QAction::triggered(group) → VM::onLanguageSelected(code)
    → 选择语言码：zh_CN / en_US / ""（跟随系统，用 QLocale::system()）
    → 卸载旧 QTranslator → 装载新 QTranslator（:/i18n/psig_gui_<code>.qm）
    → view->retranslateUi()（刷新菜单/工具栏/各页/状态栏文案）
    → 状态/日志反馈；菜单勾选态更新
```

### F-04 / F-05 / F-06 新增一行 / 新增多行 / 进程列自动填充（C-06 / C-08）
```
[右键/编辑菜单 新增一行] QAction::triggered → VM::onAddSignatureRow
    → SignatureTableModel::addRow(目标进程名)   // 进程列默认值
    → 选中新行 → 状态/日志
[右键/编辑菜单 新增多行…] QAction::triggered → VM::onAddSignatureRows
    → QInputDialog::getInt(行数, 1..N)   ─ 取消 → 中止（取消路径）
    → SignatureTableModel::addRows(n, 目标进程名)
    → 状态/日志「已新增 N 行」
```

### F-07 / F-10 进程自动枚举 + 筛选保持（C-09 / C-10）
```
[启动 install()] → VM::onRefreshProcesses（自动触发一次）
    → QtConcurrent::run(psig::EnumerateProcesses) → QFutureWatcher::finished → VM::onProcessesEnumerated
        → ProcessTableModel::setProcesses(rows)
        → 重新应用当前筛选：VM::applyProcessFilter()（保留筛选框文本，仅显示匹配项）   // 修复 C-10
[筛选框 编辑] QLineEdit::textChanged → VM::onProcessFilterChanged → 逐行 setRowHidden
```

### F-08 / F-09 列排序 + 图标显示（C-11 / C-12）
```
[进程表头 点击] QHeaderView::sectionClicked → QTableView::sortByColumn → ProcessTableModel::sort(col, order)
    → 就地排序 + layoutChanged → 表头显示排序指示符

[进程枚举完成] VM::onProcessesEnumerated
    → 对每个可访问进程：QtConcurrent::run(psig::ProcessIcon(pid)) → 回主线程
        → 字节 → QImage(ARGB32) → QIcon → ProcessTableModel::setIcon(row) → DecorationRole 刷新
    → 无图标/默认图标 → 使用系统默认进程图标（psig 返回的默认图标或 View 内置标准图标）
```

### F-11 导出 CSV 编码（C-03）
```
[右键 导出▸CSV] QAction::triggered → VM::onExportResultsCsv → 选路径 → VM::exportResultsToPath
    → psig::ExportResultsCsv(path, results, err, opt)   // 静态库写 UTF-8 BOM
    → 状态/日志
```

### F-12 配色统一 + 日志着色（C-13 / C-14）
```
[应用启动] main.cpp: app.setStyleSheet(":/style.qss")（白底基调）
[追加日志] MainWindow::appendLog(text)
    → 依据前缀/关键字选择颜色（信息/成功/错误）→ 以 HTML 富文本或格式追加
```

## 3. 约束满足说明

- 已用 Qt 信号/槽触发路径表达各功能流程；connect 归属 ViewModel（View 零 connect，对话框按钮亦由 VM 连接）。
- 各功能的正常/错误/取消三类路径见 `11_GUI执行路径.md`。
