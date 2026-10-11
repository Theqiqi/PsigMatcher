# 24 ViewModel 接口（v0.2.1，增量）

> 前置：`22_View层接口.md`、`23_Model层接口.md`、`25_静态库API调用映射.md`
> 说明：VM 持 `View*`，集中 connect（含**对话框按钮**、语言菜单、右键菜单 QAction、表格 doubleClicked/表头排序）；经 View 公开 getter 读控件、公开方法更新 UI；不用 findChild。

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
    void exportResultsToPath(const QString& path);
    void setTemplateText(const QString& text);
    void setExportColumns(const QStringList& keys);

    // 新增（供测试/入口）
    void applyLanguage(const QString& code);      // "zh_CN" / "en_US" / ""（跟随系统）

private slots:
    // 既有：onAddSignatureRow/onDeleteSignatureRows/onManageColumns/onImportSignatures/
    //       onExportSignatures/onRefreshProcesses/onProcessesEnumerated/onStartScan/
    //       onCancelScan/onScanFinished/onExportResultsCsv/Db/Txt/onClearResults/
    //       onClearLog/onProcessFilterChanged/onConfirmProcessSelection/onReselectProcess/
    //       onDeleteResultRows/onFillCustomColumn/onTemplateTextChanged/onSelectExportColumns/onAbout
    // 新增：
    void onAddSignatureRows();                    // 新增多行（弹出数量输入）
    void onLanguageSelected(QAction* act);        // 语言菜单（或三个独立槽）
    void onProcessIconReady(int row, ...);        // 图标异步回填（或经 lambda）

private:
    void bindSignaturePage();
    void bindProcessPage();
    void bindResultPage();
    void bindGlobalActions();
    void bindContextMenus();
    void bindDialogs();                           // 新增：对话框按钮接线集中处
    void applyProcessFilter();                    // 新增：按当前筛选框文本逐行 setRowHidden（刷新后重应用）
    void startIconLoad();                         // 新增：为进程行异步取图标

    // 既有辅助 …
    psig::Table tableFromModel() const;
    void applyTableToModel(const psig::Table& table);
    psig::ResultExportOptions exportOptions() const;

    // 既有成员 …
    QString m_templateText;
    QStringList m_exportColumns;
    uint32_t m_targetPid = 0;
    QString m_targetName;
    QTranslator* m_translator = nullptr;          // 新增
    QString m_language;                           // 新增：当前语言码（""=跟随系统）
};
```

## 关键行为

- `install`：装载语言（默认按 `QLocale::system()`）→ `setProcessSelectionLocked(false)` + `showProcessPage()` → **自动触发一次 `onRefreshProcesses()`**（C-09）。
- `onAddSignatureRow`：`addRow(m_targetName)`（C-08 自动填进程列）。
- `onAddSignatureRows`：`QInputDialog::getInt` 取行数 n（取消→中止）→ `addRows(n, m_targetName)`（C-06）。
- `onProcessesEnumerated`：`setProcesses(rows)` → **`applyProcessFilter()`**（C-10 刷新后重应用筛选）→ `startIconLoad()`（C-12）。
- `bindDialogs`：对列管理/选择导出列/关于，`connect(dlg.buttonBox(), accepted/rejected, dlg, accept/reject)`；列管理内部按钮 connect 到 VM 槽（C-01/C-04/C-05）。
- `applyLanguage(code)`：卸载旧 `QTranslator` → 装载 `:/i18n/psig_gui_<code>.qm`（或 ""=跟随系统）→ `m_view->retranslateUi()` → `setLanguageChecked`（C-02）。

## main.cpp（增量）

```cpp
QApplication app(...);
MainWindow view; MainViewModel vm; vm.install(&view);
// install 内部完成默认语言装载与 retranslate；随后
view.show(); return app.exec();
```

## 约束满足说明

- VM 经 View 公开 getter 读控件、公开方法更新 UI，不用 findChild；每业务方法对应真实 psig API；后台异常经 watcher finished 回主线程（不跨线程抛异常）。
- 图标转换（BGRA→QImage→QIcon）在 VM 完成，Model 仅承载 `QIcon`。
