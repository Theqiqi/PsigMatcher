# 24 ViewModel 接口（v0.2.0，增量）

> 前置：`22_View层接口.md`、`23_Model层接口.md`、`25_静态库API调用映射.md`
> 说明：VM 持 `View*`，集中 connect（含右键菜单 QAction 与表格 doubleClicked）；经 View 公开 getter 读控件、公开方法更新 UI；不用 findChild。

## 接口（增量）

```cpp
class MainViewModel : public QObject {
    Q_OBJECT
public:
    explicit MainViewModel(QObject* parent = nullptr);
    void install(MainWindow* view);

    // 可测试路径方法（既有 + 新增）
    void importSignaturesFromPath(const QString& path);
    void exportSignaturesToPath(const QString& path);
    void exportResultsToPath(const QString& path);                 // 使用当前模板/选中列/自定义列

    // 供测试注入（新增）
    void setTemplateText(const QString& text);
    void setExportColumns(const QStringList& keys);                // 空 = 全部

private slots:
    // 既有：onAddSignatureRow/onDeleteSignatureRows/onManageColumns/onImportSignatures/
    //       onExportSignatures/onRefreshProcesses/onProcessesEnumerated/onStartScan/
    //       onCancelScan/onScanFinished/onExportResultsCsv/Db/Txt/onClearResults/
    //       onClearLog/onProcessFilterChanged
    // 新增：
    void onConfirmProcessSelection();   // 双击/工具栏/右键 确认选择
    void onReselectProcess();           // 重新选择进程
    void onDeleteResultRows();          // 结果删行（单行/多选）
    void onFillCustomColumn();          // 批量填充自定义列（多选/全选）
    void onTemplateTextChanged();       // 模板文本变更
    void onSelectExportColumns();       // 选择导出列
    void onAbout();                     // 关于弹窗
    void onContextMenuRequested(const QPoint&); // 三页表格右键（或分页各自槽）

private:
    void bindSignaturePage();
    void bindProcessPage();
    void bindResultPage();
    void bindGlobalActions();
    void bindContextMenus();
    // 既有辅助 …
    psig::Table tableFromModel() const;                 // 写入 m_templateText
    void applyTableToModel(const psig::Table& table);   // 回填模板区
    std::vector<psig::MatchResult> resultRowsToPsig() const; // 含 custom
    psig::ResultExportOptions exportOptions() const;    // 模板 + 选中列

    // 既有成员 …
    QString m_templateText;                             // 表级模板文本（文档级）
    QStringList m_exportColumns;                        // 选中导出列键（空=全部）
    uint32_t m_targetPid = 0;                           // 目标进程
    QString m_targetName;
};
```

## 关键行为

- `install`：`m_view->setProcessSelectionLocked(false)` + `showProcessPage()`（启动即进程页）。
- `onConfirmProcessSelection`：校验选中 → 记录目标 → `setProcessSelectionLocked(true)` + `showSignaturePage()`。
- `onReselectProcess`：`setProcessSelectionLocked(false)` + `showProcessPage()`。
- `applyTableToModel`：`m_templateText = table.templateText` 并 `resultPage()->setTemplateText(...)`。
- `tableFromModel`：`t.templateText = m_templateText`。
- `exportOptions`：`{ m_templateText, m_exportColumns.toStdString… }`。
- `onImportSignatures`：表非空 → `QMessageBox::question`（保存/不保存/取消）→ 保存走导出特征码；取消中止。

## main.cpp（不变）

`创建 View → 创建 ViewModel → viewModel->install(view) → show() → exec()`

## 约束满足说明

- VM 经 View 公开 getter 读控件、公开方法更新 UI，不用 findChild；每业务方法对应真实 psig API；后台异常经 watcher finished 回主线程（不跨线程抛异常）。
