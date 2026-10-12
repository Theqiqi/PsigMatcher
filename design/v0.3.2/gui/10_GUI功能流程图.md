# 10 GUI 功能流程图（v0.3.2，增量）

- 版本：v0.3.2
- 模式：修改型。仅列本版受影响功能的 Qt 信号/槽触发路径；其余沿用 `design/v0.3.1/gui/10_GUI功能流程图.md`。

## C-01 未保存状态（真实脏标记）

```
SignatureEditorPage 单元格编辑
  QTableView(edit) → SignatureTableModel::setData
      → emit dataChanged(index,index)
          → (bind.cpp) MainViewModel::markSignatureDirty
              → m_sigDirty = true
              → refreshPersistentStatus()  /* 持久状态区出现 '*' */

增行 / 删行 / 列管理（既有槽）
  addRowAction::triggered    → onAddSignatureRow   → markSignatureDirty()
  addRowsAction::triggered   → onAddSignatureRows  → markSignatureDirty()
  deleteRowsAction::triggered→ onDeleteSignatureRows→ markSignatureDirty()
  manageColumnsAction::triggered → onManageColumns → markSignatureDirty()

导出 / 导入成功
  exportSignaturesAction::triggered → onExportSignatures → exportSignaturesToPath
      → psig::ExportSignatures* 成功 → m_sigDirty = false → refreshPersistentStatus()  /* '*' 消失 */
  importSignaturesFromPath 成功 → applyTableToModel → m_sigDirty = false → refreshPersistentStatus()
```

## C-02 导入特征码（导入与保存解耦）

```
importSignaturesAction::triggered → onImportSignatures
  ├─ if (!m_sigDirty) ─────────────────────────────► 直接 QFileDialog::getOpenFileName
  └─ if (m_sigDirty)
        QMessageBox（导入前确认）
          text: 「当前特征码表有未保存改动，导入将覆盖它。是否继续？」
          buttons: [继续导入(AcceptRole)] [取消(RejectRole)]      /* 无保存/不保存 */
          ├─ 取消        → 瞬时「已取消导入」+ 日志 [import] → return（不改表）
          └─ 继续导入    → QFileDialog::getOpenFileName
                              ├─ 空 → return
                              └─ path → importSignaturesFromPath(path)
                                          ├─ 失败 → 瞬时红 + 日志红 → return
                                          ├─ 进程名不一致 → confirmTargetProcessMismatch（沿用 v0.3.1 C-05）
                                          └─ 成功 → applyTableToModel + m_sigDirty=false + 瞬时/日志
```

## C-03 关闭程序 / 退出（新增未保存提示）

```
标题栏关闭按钮（QCloseEvent）       文件菜单「退出」m_quitAction::triggered
        │                                   │ connect → MainWindow::close()
        └──────────────┬────────────────────┘
                       ▼
        MainWindow 收到 QEvent::Close
                       │
        (install) m_view->installEventFilter(this)
                       ▼
        MainViewModel::eventFilter(m_view, Close)
          ├─ m_sigDirty == false → 交基类 → 正常关闭（进程退出码 0）
          └─ m_sigDirty == true
                askSaveSignatureChanges(saveCancelAborts=true)
                  QMessageBox（保存特征码）buttons: [保存(AcceptRole)] [不保存(DestructiveRole)] [取消(RejectRole)]
                    ├─ 取消        → 返回 false → event->ignore() → 阻止关闭，窗口保持打开
                    ├─ 不保存      → 返回 true  → 关闭（改动丢弃）
                    └─ 保存        → 另存为（CSV/DB）
                                        ├─ 空 → 返回 false → 阻止关闭
                                        └─ path → exportSignaturesToPath(path) → 返回 true → 关闭
```

> 快捷键（作用于模态框）：保存 `Ctrl+S`、不保存 `Ctrl+D`、取消 `Esc`（确定性键盘入口，供可访问性与 E2E 注入）。

## C-04 程序改名

```
构建：qt_add_executable(PsigMatcher …)  → out/bin/<配置>/PsigMatcher.exe
入口：QApplication::setApplicationName("PsigMatcher")
i18n：res/i18n/PsigMatcher_{zh_CN,en_US}.ts → :/i18n/PsigMatcher_<lang>.qm
界面：setWindowTitle(tr("进程特征码批量匹配器 v0.3.2"))
```

## 约束满足说明

- 执行路径（正常/错误/取消）见 `11_GUI执行路径.md`；本图给出各改动点的信号/槽触发路径。
- 修改模式：新增/修改功能（C-01/C-02/C-03/C-04）的触发路径已补全。
