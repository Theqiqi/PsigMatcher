# 24 ViewModel 接口（v0.2.2，增量）

> 前置：`22_View层接口.md`、`23_Model层接口.md`、`25_静态库API调用映射.md`
> 说明：VM 持 `View*`，集中 connect（含对话框按钮、语言菜单、右键菜单 QAction、表格排序、日志开关/右键、重新选择进程）；经 View 公开 getter 读控件、公开方法更新 UI；不用 findChild。

## 接口（增量）

```cpp
class MainViewModel : public QObject {
    Q_OBJECT
public:
    explicit MainViewModel(QObject* parent = nullptr);
    void install(MainWindow* view);

    // 可测试路径方法（沿用 + 本次相关）
    void importSignaturesFromPath(const QString& path);
    void exportSignaturesToPath(const QString& path);
    void exportResultsToPath(const QString& path);
    void setTemplateText(const QString& text);
    void setExportColumns(const QStringList& keys);
    void applyLanguage(const QString& code, bool feedback = true); // "zh_CN"/"en_US"/""

private slots:
    // 沿用：onAddSignatureRow/onAddSignatureRows/onDeleteSignatureRows/onImportSignatures/
    //       onExportSignatures/onRefreshProcesses/onProcessesEnumerated/onStartScan/
    //       onCancelScan/onScanFinished/onExportResultsCsv/Db/Txt/onClearResults/
    //       onClearLog/onProcessFilterChanged/onConfirmProcessSelection/onDeleteResultRows/
    //       onFillCustomColumn/onTemplateTextChanged/onSelectExportColumns/onAbout
    // 本次相关：
    void onReselectProcess();                  // 变更：切换前弹保存提示（C-04）
    void onManageColumns();                    // 变更：下拉源取键/类型（C-02）
    void onToggleLog(bool visible);            // 新增：日志 dock 显隐（C-08）
    void onClearLog();                         // 变更：改为日志右键触发（C-08）

private:
    void bindSignaturePage();
    void bindProcessPage();
    void bindResultPage();
    void bindGlobalActions();
    void bindContextMenus();                   // 含日志右键（C-08）
    void bindLogActions();                     // 新增：toggleLog + clearLog 连接（C-08）
    void applyProcessFilter();
    void startIconLoad();                      // 变更：仅 accessible 行取图标（C-09）
    void applyProcessIcon(uint32_t pid, const QIcon& icon); // 仅 accessible 行注入（C-09）
    void logTagged(const QString& tag, const QString& msg);

    psig::Table tableFromModel() const;        // 变更：内置列导出标题按键翻译（C-05/C-06）
    void applyTableToModel(const psig::Table& table);
    psig::ResultExportOptions exportOptions() const; // 变更：构造随语言 headers（C-01/C-05）
    QStringList resultColumnDefs() const;      // 变更：增加 address_dec、custom→备注（C-01）
    void applyInitialColumns();                // 变更：列管理下拉初始化（C-02）
    void askSaveBeforeReselect();              // 新增：C-04 保存提示

    QString m_templateText;
    QStringList m_exportColumns;
    uint32_t m_targetPid = 0;
    QString m_targetName;
    QTranslator* m_translator = nullptr;
    QString m_language;
};
```

## 关键行为

- `resultColumnDefs()`（C-01）：在 v0.2.1 的内置列基础上增加 `address_dec`（展示「地址(十进制)」），并将自定义列标题由「自定义」改为「备注」，与静态库 `ResultColumnDefs` 对齐。
- `exportOptions()`（C-05/C-06）：构造 `psig::ResultExportOptions`，按当前 `m_language` 生成 `headers` 向量——内置列用对应语言标题（如英文 Name/Signature/Remarks、中文 名称/特征码/备注），自定义「备注」列固定不随语言；`columns` 与 `headers` 长度一致时静态库优先使用 `headers`。
- `onReselectProcess()`（C-04）：若特征码表存在未保存改动，先 `askSaveBeforeReselect()` 弹 `QMessageBox::question`（保存/不保存/取消）；保存则 `exportSignaturesToPath(m_lastPath)`（路径缺失则走另存为）；取消则中止；否则继续切换回进程选择页并刷新。
- `startIconLoad()` / `applyProcessIcon()`（C-09）：枚举完成后仅对 `accessible==true` 的进程调用 `psig::ProcessIcon` 取图标并 `setIcon`；无权限进程不注入图标（仅文本显示），不阻止选中。
- `onManageColumns()`（C-02）：列管理对话框键/类型改为下拉；`keyCombo` 选项来自 `psig::BuiltinSignatureColumns()` 的键（去重），`typeCombo` 为固定枚举；新增列仅改表元数据，不影响扫描匹配。
- `bindLogActions()`（C-08）：`toggleLogAction` → `onToggleLog`（显隐日志 dock 并同步 `setChecked`）；`clearLogAction`（日志右键）→ `onClearLog`；视图菜单不再含清空日志。
- `tableFromModel()` / `toPsigColumn`（C-05/C-06）：导出特征码表时内置列标题按 `m_language` 经 `BuiltinKeyFromTitle` 反向映射到键；自定义列直接取 `ColumnDef.title`（中文「备注」不随语言）。

## 约束满足说明

- VM 经 View 公开 getter 读控件、公开方法更新 UI，不用 findChild；每业务方法对应真实 psig API；后台异常经 watcher finished 回主线程（不跨线程抛异常）。
- 图标转换（BGRA→QImage→QIcon）在 VM 完成，且仅对 `accessible` 行注入；权限语义（是否显示图标）在 VM 与 Model 一致判定。
