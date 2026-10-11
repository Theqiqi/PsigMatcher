# 24 ViewModel 接口文档

- 版本：v0.1.0
- 前置：`04_GUI需求分析.md`

```cpp
class MainViewModel : public QObject {
    Q_OBJECT
public:
    explicit MainViewModel(QObject* parent = nullptr);
    ~MainViewModel() override;

    // 绑定：保存 view 指针 → 取页面/控件指针 → 集中 connect → 初始化
    void install(MainWindow* view);

private slots:
    void onAddSignatureRow();
    void onDeleteSignatureRows();
    void onManageColumns();
    void onImportSignatures();
    void onExportSignatures();
    void onRefreshProcesses();
    void onStartScan();
    void onCancelScan();
    void onExportResultsCsv();
    void onExportResultsDb();
    void onExportResultsTxt();
    void onClearResults();
    void onClearLog();
    void onProcessesEnumerated();   // QFutureWatcher finished
    void onScanFinished();          // QFutureWatcher finished

private:
    void bindSignaturePage();
    void bindProcessPage();
    void bindResultPage();
    void bindGlobalActions();
    void refreshPersistentStatus();
    std::vector<psig::Pattern> collectPatterns(std::string& err) const;
    std::vector<uint32_t> selectedPids() const;

    MainWindow* m_view = nullptr;              // 绑定引用，不拥有
    SignatureTableModel* m_sigModel = nullptr; // VM 持有并负责生命周期
    ProcessTableModel* m_procModel = nullptr;
    ResultTableModel* m_resultModel = nullptr;

    QFutureWatcher<std::vector<psig::ProcessInfo>>* m_procWatcher = nullptr;
    QFutureWatcher<std::vector<psig::MatchResult>>* m_scanWatcher = nullptr;
    std::atomic_bool m_cancel{false};          // 扫描取消标志（资源句柄）
};
```

## install() 绑定内容

1. `m_view = view`（nullptr 检查）。
2. 创建 Model：`m_sigModel/m_procModel/m_resultModel`。
3. 经 View getter 注入模型：`view->signaturePage()->setModel(m_sigModel)` 等。
4. connect 全部控件信号：
   - `signaturePage()->addRowBtn() -> clicked -> onAddSignatureRow`
   - `signaturePage()->deleteRowBtn() -> clicked -> onDeleteSignatureRows`
   - `signaturePage()->manageColumnsBtn() -> clicked -> onManageColumns`
   - `processPage()->refreshBtn() -> clicked -> onRefreshProcesses`
   - `processPage()->selectAllBtn() -> clicked -> 全选`
   - `processPage()->filterEdit() -> textChanged -> 过滤（Model 代理/隐藏行）`
   - `resultPage()->exportCsvBtn()/exportDbBtn()/exportTxtBtn()/clearBtn() -> ...`
   - `view->importSignaturesAction()/exportSignaturesAction() -> ...`
   - `view->refreshProcessAction()/startScanAction()/cancelScanAction() -> ...`
   - `view->manageColumnsAction() -> onManageColumns`
   - `view->logEdit()` 右键清空 → onClearLog（或按钮）
5. 初始化：设置初始列（BuiltinSignatureColumns 转 Model ColumnDef）、`refreshPersistentStatus()`。

## main.cpp 固定流程

```cpp
QApplication app(argc, argv);
MainWindow view;
MainViewModel vm;
vm.install(&view);
view.show();
return app.exec();
```

## 约束满足说明

- ViewModel 继承 QObject、含 Q_OBJECT；持 View*（不拥有）；经 install(MainWindow*) 集中 connect；持 Model 指针与取消标志（资源句柄），唯一负责其生命周期；经 View 公开 getter/方法读写，不 findChild。
