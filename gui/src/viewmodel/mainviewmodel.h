// viewmodel/mainviewmodel.h - ViewModel：持 View* 指针，链接事件，调静态库，操控 View 刷新
#pragma once

#include <QFutureWatcher>
#include <QObject>

#include <atomic>
#include <string>
#include <vector>

#include "model/resulttablemodel.h"
#include "model/processtablemodel.h"
#include "model/signaturetablemodel.h"
#include "psig.h"

class MainWindow;
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

private:
    void bindSignaturePage();
    void bindProcessPage();
    void bindResultPage();
    void bindGlobalActions();
    void applyInitialColumns();
    void refreshPersistentStatus();
    std::vector<psig::Pattern> collectPatterns(std::string& err) const;
    std::vector<uint32_t> selectedPids() const;
    psig::Table tableFromModel() const;
    void applyTableToModel(const psig::Table& table);

    MainWindow* m_view = nullptr;              // 绑定引用，不拥有
    SignatureTableModel* m_sigModel = nullptr; // VM 持有并负责生命周期
    ProcessTableModel* m_procModel = nullptr;
    ResultTableModel* m_resultModel = nullptr;

    QFutureWatcher<std::vector<psig::ProcessInfo>>* m_procWatcher = nullptr;
    QFutureWatcher<std::vector<psig::MatchResult>>* m_scanWatcher = nullptr;
    std::atomic_bool m_cancel{false};          // 扫描取消标志（资源句柄）
    std::string m_lastError;                   // 后台任务错误（回主线程读取）
};
