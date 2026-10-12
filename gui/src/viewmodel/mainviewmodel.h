// viewmodel/mainviewmodel.h - ViewModel：持 View* 指针，链接事件，调静态库，操控 View 刷新
#pragma once

#include <QFutureWatcher>
#include <QIcon>
#include <QObject>
#include <QString>
#include <QStringList>

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "model/resulttablemodel.h"
#include "model/processtablemodel.h"
#include "model/signaturetablemodel.h"
#include "psig.h"

class MainWindow;
class QAction;
class QTranslator;
class QPoint;
class QModelIndex;
class QTimer;

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

    // 语言切换（"zh_CN" / "en_US" / ""（跟随系统））；供菜单与测试调用
    // feedback=false 时不产生瞬时/日志反馈（启动装载默认语言时使用）
    void applyLanguage(const QString& code, bool feedback = true);

private slots:
    void onAddSignatureRow();
    void onAddSignatureRows();
    void onDeleteSignatureRows();
    void onManageColumns();
    void onImportSignatures();
    void onExportSignatures();
    void onRefreshProcesses();
    void onProcessesEnumerated();
    void onStartScan();
    void onCancelScan();
    void onScanFinished();
    void onProgressTick();                         // C-03：定时采样扫描进度刷新界面
    void onExportResultsCsv();
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
    void onToggleLog();                           // C-08：日志 dock 显隐
    void onSignatureContextMenu(const QPoint& pos);
    void onProcessContextMenu(const QPoint& pos);
    void onResultContextMenu(const QPoint& pos);
    void onLogContextMenu(const QPoint& pos);    // C-08：日志窗口右键菜单

private:
    void bindSignaturePage();
    void bindProcessPage();
    void bindResultPage();
    void bindGlobalActions();
    void bindContextMenus();
    void bindLogActions();                       // C-08：日志开关 + 日志右键清空
    void applyInitialColumns();
    void applyProcessFilter();                    // 按当前筛选框文本逐行 setRowHidden
    void startIconLoad();                         // 为进程行异步取图标（仅 accessible）
    void applyProcessIcon(uint32_t pid, const QIcon& icon);
    void refreshPersistentStatus();
    void logTagged(const QString& tag, const QString& message); // 追加带级别标签的日志
    void markSignatureDirty();                    // C-04：特征码表有未保存改动
    bool askSaveBeforeReselect();                 // C-04：重新选择进程前保存提示（false=取消切换）
    bool confirmTargetProcessMismatch(const QString& libProc); // C-05：导入进程不一致提示（true=继续）
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
    std::shared_ptr<std::atomic_bool> m_alive; // 图标后台任务存活标志

    // C-03/C-04：扫描进度定时采样——工作线程只写原子变量，主线程定时读取刷新
    QTimer* m_progressTimer = nullptr;
    std::atomic<size_t> m_progDone{0};         // 已完成区域数
    std::atomic<size_t> m_progTotal{0};        // 总区域数
    std::atomic<size_t> m_progHits{0};         // 命中累计
    std::atomic<uint32_t> m_progPid{0};        // 当前 pid

    QString m_templateText;                    // 表级模板文本（文档级）
    QStringList m_exportColumns;               // 选中导出列键（空 = 全部）
    uint32_t m_targetPid = 0;                  // 目标进程
    QString m_targetName;

    QTranslator* m_translator = nullptr;       // 应用翻译器（VM 持有）
    QString m_language;                        // 当前语言码（""=跟随系统）
    bool m_sigDirty = false;                   // C-04：特征码表是否有未保存改动
};
