// viewmodel/mainviewmodel.cpp - ViewModel 核心：构造/装配/状态与日志
// 说明：MainViewModel 按功能域拆分为多个 partial 实现文件（同一声明 mainviewmodel.h）：
//   bind.cpp      —— bind* 装配与初始列
//   process.cpp   —— 进程枚举/筛选/选中/重选/图标
//   signature.cpp —— 特征码增删/列管理/导入导出
//   scan.cpp      —— 扫描启动/取消/完成
//   result.cpp    —— 结果导出/列选择/填充/清空/关于
//   language.cpp  —— 语言切换
#include "viewmodel/mainviewmodel.h"

#include <QEvent>
#include <QMenu>
#include <QPlainTextEdit>
#include <QTimer>

#include "view/mainwindow.h"
#include "view/processselectionpage.h"
#include "view/resultpage.h"
#include "view/signatureeditorpage.h"

MainViewModel::MainViewModel(QObject* parent) : QObject(parent),
    m_alive(std::make_shared<std::atomic_bool>(true)) {}

MainViewModel::~MainViewModel() {
    m_alive->store(false); // 通知图标后台任务停止回填
    if (m_scanWatcher) m_scanWatcher->waitForFinished();
    if (m_procWatcher) m_procWatcher->waitForFinished();
}

void MainViewModel::install(MainWindow* view) {
    if (!view) return;
    m_view = view;

    m_sigModel = new SignatureTableModel(this);
    m_procModel = new ProcessTableModel(this);
    m_resultModel = new ResultTableModel(this);

    m_view->signaturePage()->setModel(m_sigModel);
    m_view->processPage()->setModel(m_procModel);
    m_view->resultPage()->setModel(m_resultModel);
    m_view->processPage()->setDefaultColumnWidths(); // C-07：进程列宽

    // C-03：关闭裁决归属 ViewModel（View 不含事件处理逻辑）
    m_view->installEventFilter(this);

    m_procWatcher = new QFutureWatcher<std::vector<psig::ProcessInfo>>(this);
    m_scanWatcher = new QFutureWatcher<std::vector<psig::MatchResult>>(this);
    connect(m_procWatcher, &QFutureWatcherBase::finished, this, &MainViewModel::onProcessesEnumerated);
    connect(m_scanWatcher, &QFutureWatcherBase::finished, this, &MainViewModel::onScanFinished);

    // C-03：扫描进度定时采样（工作线程写原子变量，主线程定时读取刷新界面）
    m_progressTimer = new QTimer(this);
    m_progressTimer->setInterval(150);
    m_progressTimer->setTimerType(Qt::CoarseTimer);
    connect(m_progressTimer, &QTimer::timeout, this, &MainViewModel::onProgressTick);

    bindSignaturePage();
    bindProcessPage();
    bindResultPage();
    bindGlobalActions();
    bindContextMenus();
    bindLogActions();                         // C-08：日志开关 + 日志右键清空

    applyInitialColumns();

    // 默认语言：跟随系统（QLocale() 初值取自系统语言，测试可经 setDefault 覆盖）
    applyLanguage(QString(), /*feedback=*/false);

    // 启动即为进程页（未选定进程前锁定特征码/结果页）
    m_view->setProcessSelectionLocked(false);
    m_view->showProcessPage();
    refreshPersistentStatus();
    m_view->setTransientMessage(tr("就绪，请选择目标进程"));
    logTagged(QStringLiteral("startup"), tr("进程特征码批量匹配器 v0.3.3 已就绪"));

    onRefreshProcesses(); // C-09：启动后自动枚举一次进程
}

void MainViewModel::logTagged(const QString& tag, const QString& message) {
    if (!m_view) return;
    m_view->appendLog(QStringLiteral("[") + tag + QStringLiteral("] ") + message);
}

void MainViewModel::refreshPersistentStatus() {
    const int patterns = m_sigModel ? m_sigModel->rowCount() : 0;
    const int procs = m_procModel ? m_procModel->rowCount() : 0;
    const QString target = m_targetPid ? QStringLiteral("%1(%2)").arg(m_targetName).arg(m_targetPid)
                                       : QStringLiteral("-");
    // C-01：未保存改动时在持久状态末尾追加 '*'（真实脏标记的唯一可见位置）
    const QString dirtyMark = m_sigDirty ? QStringLiteral(" *") : QString();
    m_view->setPersistentStatus(tr("就绪 | 目标 %1 | 进程 %2 | 特征码 %3%4")
                                   .arg(target)
                                   .arg(procs)
                                   .arg(patterns)
                                   .arg(dirtyMark));
}

void MainViewModel::markSignatureDirty() {
    m_sigDirty = true;
    refreshPersistentStatus(); // C-01：立即反映未保存标记
}

// C-03：关闭请求拦截——有未保存改动则提示保存/不保存/取消；取消则阻止关闭
bool MainViewModel::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_view && event && event->type() == QEvent::Close) {
        if (m_sigDirty && !askSaveSignatureChanges(/*saveCancelAborts=*/true)) {
            event->ignore();
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}

// C-08：日志窗口右键菜单 / 清空 / 视图菜单开关
void MainViewModel::onLogContextMenu(const QPoint& pos) {
    m_view->logContextMenu()->exec(m_view->logEdit()->viewport()->mapToGlobal(pos));
}

void MainViewModel::onToggleLog() {
    const bool now = !m_view->logDockVisible();
    m_view->setLogDockVisible(now);
    m_view->setToggleLogChecked(now);
}

void MainViewModel::onClearLog() {
    m_view->clearLog();
}
