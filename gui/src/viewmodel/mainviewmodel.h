// viewmodel/mainviewmodel.h - ViewModel：持 View* 指针，链接事件，调静态库，操控 View 刷新
#pragma once

#include <QFutureWatcher>
#include <QObject>
#include <QStringList>

#include <atomic>
#include <string>
#include <vector>

#include "model/resulttablemodel.h"
#include "model/processtablemodel.h"
#include "model/signaturetablemodel.h"
#include "psig.h"

class MainWindow;
class QPoint;
class QModelIndex;

class MainViewModel : public QObject {
    Q_OBJECT
public:
    explicit MainViewModel(QObject* parent = nullptr);
    ~MainViewModel() override;

    // 绑定：保存 view 指针 → 取页面/控件指针 → 集中 connect → 初始化
    void install(MainWindow* view);

    // 可测试的业务操作（按路径，供单元/集成/用户流程测试直接驱动）
    void importSignaturesFromPath(const QString& path);
    void exportSignaturesToPath(const QString& path);
    void exportResultsToPath(const QString& path);

    // 供测试注入
    void setTemplateText(const QString& text);
    void setExportColumns(const QStringList& keys);
    QStringList exportColumns() const { return m_exportColumns; }

private slots:
    void onAddSignatureRow();
    void onDeleteSignatureRows();
    void onManageColumns();
    void onImportSignatures();
    void onExportSignatures();
    void onRefreshProcesses();
    void onProcessesEnumerated();
    void onStartScan();
    void onCancelScan();
    void onScanFinished();
    void onExportResultsCsv();
    void onExportResultsDb();
    void onExportResultsTxt();
    void onClearResults();
    void onClearLog();
    void onProcessFilterChanged(const QString& text);
    // v0.2.0 新增
    void onConfirmProcessSelection();
    void onReselectProcess();
    void onDeleteResultRows();
    void onFillCustomColumn();
    void onTemplateTextChanged();
    void onSelectExportColumns();
    void onAbout();
    void onSignatureContextMenu(const QPoint& pos);
    void onProcessContextMenu(const QPoint& pos);
    void onResultContextMenu(const QPoint& pos);

private:
    void bindSignaturePage();
    void bindProcessPage();
    void bindResultPage();
    void bindGlobalActions();
    void bindContextMenus();
    void applyInitialColumns();
    void refreshPersistentStatus();
    std::vector<psig::Pattern> collectPatterns(std::string& err) const;
    std::vector<uint32_t> selectedPids() const;
    psig::Table tableFromModel() const;
    void applyTableToModel(const psig::Table& table);
    psig::ResultExportOptions exportOptions() const;

    MainWindow* m_view = nullptr;              // 绑定引用，不拥有
    SignatureTableModel* m_sigModel = nullptr; // VM 持有并负责生命周期
    ProcessTableModel* m_procModel = nullptr;
    ResultTableModel* m_resultModel = nullptr;

    QFutureWatcher<std::vector<psig::ProcessInfo>>* m_procWatcher = nullptr;
    QFutureWatcher<std::vector<psig::MatchResult>>* m_scanWatcher = nullptr;
    std::atomic_bool m_cancel{false};          // 扫描取消标志（资源句柄）
    std::string m_lastError;                   // 后台任务错误（回主线程读取）

    QString m_templateText;                    // 表级模板文本（文档级）
    QStringList m_exportColumns;               // 选中导出列键（空 = 全部）
    uint32_t m_targetPid = 0;                  // 目标进程
    QString m_targetName;
};
