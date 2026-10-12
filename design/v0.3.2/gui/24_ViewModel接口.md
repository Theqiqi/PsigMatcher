# 24 ViewModel 接口（v0.3.2，增量）

> 前置：`22_View层接口.md`、`23_Model层接口.md`、静态库 API 文档（`psig.h`）
> 说明：ViewModel 持 `MainWindow*`/Model*，集中 `connect`，调静态库 API，经 View 公开方法刷新 UI。

## MainViewModel（增量）

```cpp
class MainViewModel : public QObject {
    Q_OBJECT
public:
    // 沿用（本版行为增强）
    bool isSignatureDirty() const { return m_sigDirty; }   // 新增：未保存状态查询（供测试与状态刷新）
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;   // 新增：拦截 View 的关闭请求（C-03）
private slots:
    // onImportSignatures 行为变更（C-02）
private:
    void markSignatureDirty();                              // 变更：置脏并刷新持久状态（C-01）
    // askSaveBeforeReselect → 重命名 + 参数化（C-01/C-03）
    bool askSaveSignatureChanges(bool saveCancelAborts);
};
```

## 行为变更

### install()（C-01/C-03）

- 新增 `m_view->installEventFilter(this);`（关闭裁决归属 ViewModel，View 不含事件逻辑）。
- 其余装配顺序不变：创建 Model → `setModel` → 创建 watcher/timer → `bind*` → 初始列 → 语言 → 进程页 → 刷新状态。

### markSignatureDirty()（C-01）

```cpp
void MainViewModel::markSignatureDirty() {
    m_sigDirty = true;
    refreshPersistentStatus();      // 持久状态区出现 '*'
}
```

- 触发源：`dataChanged`（单元格编辑，`bindSignaturePage` 连接）+ 既有增行/删行/列管理槽。

### refreshPersistentStatus()（C-01）

- 组装 `就绪 | 目标 %1 | 进程 %2 | 特征码 %3%4`，其中 `%4 = m_sigDirty ? " *" : ""`。

### onImportSignatures()（C-02）

1. 若 `isSignatureDirty()`：
   - 弹 `QMessageBox`：`title=导入前确认`，`text=当前特征码表有未保存改动，导入将覆盖它。是否继续？`，按钮 `[继续导入(AcceptRole)] [取消(RejectRole)]`；
   - `取消` → 瞬时「已取消导入」+ 日志 `[import] 取消导入（未保存改动）`，**return**（不弹文件对话框）；
   - `继续导入` → 继续第 2 步。
2. `QFileDialog::getOpenFileName`（`特征码表 (*.db)`）→ 空则 return。
3. `importSignaturesFromPath(path)`（沿用：C-05 目标进程不一致提示、`applyTableToModel`、`m_sigDirty=false`）。

> **删除**旧的「[保存/不保存/取消] + 保存走另存为」内嵌逻辑。

### askSaveSignatureChanges(bool saveCancelAborts)（C-03，重命名自 `askSaveBeforeReselect`）

```cpp
// 返回 false = 中止当前动作（阻止关闭 / 中止重选进程）
bool MainViewModel::askSaveSignatureChanges(bool saveCancelAborts) {
    QMessageBox box(m_view);
    box.setWindowTitle(tr("保存特征码"));
    box.setText(tr("当前特征码表尚未保存，是否保存？"));
    QPushButton* saveBtn    = box.addButton(tr("保存"),   QMessageBox::AcceptRole);
    QPushButton* discardBtn = box.addButton(tr("不保存"), QMessageBox::DestructiveRole);
    QPushButton* cancelBtn  = box.addButton(tr("取消"),   QMessageBox::RejectRole);
    saveBtn->setShortcut(QKeySequence(QStringLiteral("Ctrl+S")));     // 确定性键盘入口
    discardBtn->setShortcut(QKeySequence(QStringLiteral("Ctrl+D")));
    box.exec();
    if (box.clickedButton() == cancelBtn) return false;
    if (box.clickedButton() == saveBtn) {
        const QString path = QFileDialog::getSaveFileName(
            m_view, tr("保存特征码"), QString(), tr("SQLite (*.db);;CSV (*.csv)"));
        if (path.isEmpty()) return !saveCancelAborts;   // 关闭=中止；重选=沿用「不阻断」
        exportSignaturesToPath(path);
    }
    return true;
}
```

- 调用点：`onReselectProcess()` → `askSaveSignatureChanges(false)`；`eventFilter(Close)` → `askSaveSignatureChanges(true)`。

### eventFilter()（C-03）

```cpp
bool MainViewModel::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_view && event->type() == QEvent::Close) {
        if (m_sigDirty && !askSaveSignatureChanges(/*saveCancelAborts=*/true)) {
            event->ignore();
            return true;                       // 拦截关闭
        }
    }
    return QObject::eventFilter(watched, event);
}
```

## 绑定（install / bind*）

| 位置 | 新增 connect | 用途 |
| --- | --- | --- |
| `bindSignaturePage()` | `m_sigModel, &QAbstractItemModel::dataChanged → markSignatureDirty` | C-01 单元格编辑计入未保存 |
| `bindGlobalActions()` | `m_view->quitAction(), &QAction::triggered → m_view, &QWidget::close` | C-03 修复「退出」死项，复用关闭提示 |

## 约束满足说明

- 每个 VM 业务方法对应真实静态库 API 调用（见 `25_静态库API调用映射.md`）；本版未新增静态库调用。
- `connect` 集中在 `install()`/`bind*`；VM 经 View 公开 getter 读控件、公开方法更新 UI（未新增 `findChild`）。
- 后台线程异常经信号回主线程（本版无新增后台任务）；弹窗在主线程执行。
- 指针判空：`eventFilter` 与 `askSaveSignatureChanges` 经 `m_view` 使用前依既有 `install` 保证非空；`clickedButton()` 判空沿用。
