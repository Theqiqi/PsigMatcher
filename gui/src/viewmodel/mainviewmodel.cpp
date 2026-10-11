// viewmodel/mainviewmodel.cpp - ViewModel 实现
#include "viewmodel/mainviewmodel.h"

#include <QAction>
#include <QDateTime>
#include <QFileDialog>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableView>
#include <QtConcurrent>

#include "view/columnmanagerdialog.h"
#include "view/mainwindow.h"
#include "view/processselectionpage.h"
#include "view/resultpage.h"
#include "view/signatureeditorpage.h"

namespace {

ColumnDef toModelColumn(const psig::ColumnDef& c) {
    ColumnDef m;
    m.key = QString::fromStdString(c.key);
    m.title = QString::fromStdString(c.title);
    m.type = QString::fromStdString(c.type);
    m.builtin = c.builtin;
    return m;
}

psig::ColumnDef toPsigColumn(const ColumnDef& c) {
    psig::ColumnDef p;
    p.key = c.key.toStdString();
    p.title = c.title.toStdString();
    p.type = c.type.toStdString();
    p.builtin = c.builtin;
    return p;
}

ResultRow toResultRow(const psig::MatchResult& m) {
    ResultRow r;
    r.patternName = QString::fromStdString(m.patternName);
    r.pid = m.pid;
    r.processName = QString::fromStdWString(m.processName);
    r.address = m.address;
    r.moduleName = QString::fromStdWString(m.moduleName);
    r.moduleBase = m.moduleBase;
    r.matchedBytes = QString::fromStdString(m.matchedBytes);
    r.hitTime = QString::fromStdString(m.hitTime);
    return r;
}

psig::MatchResult toPsigResult(const ResultRow& r) {
    psig::MatchResult m;
    m.patternName = r.patternName.toStdString();
    m.pid = r.pid;
    m.processName = r.processName.toStdWString();
    m.address = r.address;
    m.moduleName = r.moduleName.toStdWString();
    m.moduleBase = r.moduleBase;
    m.matchedBytes = r.matchedBytes.toStdString();
    m.hitTime = r.hitTime.toStdString();
    return m;
}

} // namespace

MainViewModel::MainViewModel(QObject* parent) : QObject(parent) {}

MainViewModel::~MainViewModel() {
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

    m_procWatcher = new QFutureWatcher<std::vector<psig::ProcessInfo>>(this);
    m_scanWatcher = new QFutureWatcher<std::vector<psig::MatchResult>>(this);
    connect(m_procWatcher, &QFutureWatcherBase::finished, this, &MainViewModel::onProcessesEnumerated);
    connect(m_scanWatcher, &QFutureWatcherBase::finished, this, &MainViewModel::onScanFinished);

    bindSignaturePage();
    bindProcessPage();
    bindResultPage();
    bindGlobalActions();

    applyInitialColumns();
    m_view->showSignaturePage();
    refreshPersistentStatus();
    m_view->setTransientMessage(tr("就绪"));
    m_view->appendLog(tr("[启动] 进程特征码批量匹配器已就绪"));
}

void MainViewModel::bindSignaturePage() {
    auto* page = m_view->signaturePage();
    connect(page->addRowBtn(), &QPushButton::clicked, this, &MainViewModel::onAddSignatureRow);
    connect(page->deleteRowBtn(), &QPushButton::clicked, this, &MainViewModel::onDeleteSignatureRows);
    connect(page->manageColumnsBtn(), &QPushButton::clicked, this, &MainViewModel::onManageColumns);
}

void MainViewModel::bindProcessPage() {
    auto* page = m_view->processPage();
    connect(page->refreshBtn(), &QPushButton::clicked, this, &MainViewModel::onRefreshProcesses);
    connect(page->selectAllBtn(), &QPushButton::clicked, this, [this]() {
        if (m_view->processPage()->table()->model())
            m_view->processPage()->table()->selectAll();
    });
    connect(page->filterEdit(), &QLineEdit::textChanged, this, &MainViewModel::onProcessFilterChanged);
}

void MainViewModel::bindResultPage() {
    auto* page = m_view->resultPage();
    connect(page->exportCsvBtn(), &QPushButton::clicked, this, &MainViewModel::onExportResultsCsv);
    connect(page->exportDbBtn(), &QPushButton::clicked, this, &MainViewModel::onExportResultsDb);
    connect(page->exportTxtBtn(), &QPushButton::clicked, this, &MainViewModel::onExportResultsTxt);
    connect(page->clearBtn(), &QPushButton::clicked, this, &MainViewModel::onClearResults);
}

void MainViewModel::bindGlobalActions() {
    connect(m_view->importSignaturesAction(), &QAction::triggered, this, &MainViewModel::onImportSignatures);
    connect(m_view->exportSignaturesAction(), &QAction::triggered, this, &MainViewModel::onExportSignatures);
    connect(m_view->refreshProcessAction(), &QAction::triggered, this, &MainViewModel::onRefreshProcesses);
    connect(m_view->startScanAction(), &QAction::triggered, this, &MainViewModel::onStartScan);
    connect(m_view->cancelScanAction(), &QAction::triggered, this, &MainViewModel::onCancelScan);
    connect(m_view->manageColumnsAction(), &QAction::triggered, this, &MainViewModel::onManageColumns);
    connect(m_view->addSignatureRowAction(), &QAction::triggered, this, &MainViewModel::onAddSignatureRow);
    connect(m_view->showSignaturePageAction(), &QAction::triggered, m_view, &MainWindow::showSignaturePage);
    connect(m_view->showProcessPageAction(), &QAction::triggered, m_view, &MainWindow::showProcessPage);
    connect(m_view->showResultPageAction(), &QAction::triggered, m_view, &MainWindow::showResultPage);
    connect(m_view->aboutAction(), &QAction::triggered, this, [this]() {
        m_view->setTransientMessage(tr("进程特征码批量匹配器 · 基于 psig 静态库"));
    });
    connect(m_view->clearLogAction(), &QAction::triggered, this, &MainViewModel::onClearLog);
}

void MainViewModel::applyInitialColumns() {
    std::vector<ColumnDef> cols;
    for (const auto& c : psig::BuiltinSignatureColumns()) cols.push_back(toModelColumn(c));
    m_sigModel->setColumns(cols);
}

void MainViewModel::refreshPersistentStatus() {
    const int patterns = m_sigModel ? m_sigModel->rowCount() : 0;
    const int procs = m_procModel ? m_procModel->rowCount() : 0;
    m_view->setPersistentStatus(tr("就绪 | 进程 %1 | 特征码 %2").arg(procs).arg(patterns));
}

// ---------------------------------------------------------------------------
// 特征码表
// ---------------------------------------------------------------------------

void MainViewModel::onAddSignatureRow() {
    m_sigModel->addRow();
    m_view->signaturePage()->selectRow(m_sigModel->rowCount() - 1);
    refreshPersistentStatus();
    m_view->setTransientMessage(tr("已新增特征码行"));
}

void MainViewModel::onDeleteSignatureRows() {
    auto* sel = m_view->signaturePage()->table()->selectionModel();
    if (!sel) return;
    std::vector<int> rows;
    for (const auto& idx : sel->selectedRows()) rows.push_back(idx.row());
    if (rows.empty()) {
        m_view->setTransientMessage(tr("请先选择要删除的行"), true);
        return;
    }
    m_sigModel->removeRows(rows);
    refreshPersistentStatus();
    m_view->setTransientMessage(tr("已删除 %1 行").arg(rows.size()));
    m_view->appendLog(tr("[编辑] 删除特征码 %1 行").arg(rows.size()));
}

void MainViewModel::onManageColumns() {
    ColumnManagerDialog dlg(m_view);
    dlg.setColumns(m_sigModel->columns());
    if (dlg.exec() != QDialog::Accepted) return;
    m_sigModel->setColumns(dlg.columns());
    m_view->setTransientMessage(tr("列已更新"));
    m_view->appendLog(tr("[编辑] 更新列定义"));
}

void MainViewModel::onImportSignatures() {
    const QString path = QFileDialog::getOpenFileName(
        m_view, tr("导入特征码"), QString(), tr("特征码表 (*.csv *.db);;CSV (*.csv);;SQLite (*.db)"));
    if (path.isEmpty()) return; // 取消路径
    importSignaturesFromPath(path);
}

void MainViewModel::importSignaturesFromPath(const QString& path) {
    std::string err;
    psig::Table table;
    if (path.endsWith(".db", Qt::CaseInsensitive)) {
        table = psig::ImportSignaturesDb(path.toStdString(), err);
    } else {
        table = psig::ImportSignaturesCsv(path.toStdString(), err);
    }
    if (!err.empty()) {
        m_view->setTransientMessage(tr("导入失败: %1").arg(QString::fromStdString(err)), true);
        m_view->appendLog(tr("[导入] 失败: %1").arg(QString::fromStdString(err)));
        return;
    }
    applyTableToModel(table);
    refreshPersistentStatus();
    m_view->setTransientMessage(tr("已导入 %1 条").arg(table.rows.size()));
    m_view->appendLog(tr("[导入] %1 条").arg(table.rows.size()));
}

void MainViewModel::onExportSignatures() {
    const QString path = QFileDialog::getSaveFileName(
        m_view, tr("导出特征码"), QString(), tr("CSV (*.csv);;SQLite (*.db)"));
    if (path.isEmpty()) return;
    exportSignaturesToPath(path);
}

void MainViewModel::exportSignaturesToPath(const QString& path) {
    std::string err;
    psig::Table table = tableFromModel();
    bool ok = path.endsWith(".db", Qt::CaseInsensitive)
                  ? psig::ExportSignaturesDb(path.toStdString(), table, err)
                  : psig::ExportSignaturesCsv(path.toStdString(), table, err);
    if (!ok) {
        m_view->setTransientMessage(tr("导出失败: %1").arg(QString::fromStdString(err)), true);
        return;
    }
    m_view->setTransientMessage(tr("已导出: %1").arg(path));
    m_view->appendLog(tr("[导出] 特征码 → %1").arg(path));
}

// ---------------------------------------------------------------------------
// 进程
// ---------------------------------------------------------------------------

void MainViewModel::onRefreshProcesses() {
    m_view->setTransientMessage(tr("正在枚举进程…"));
    m_procWatcher->setFuture(QtConcurrent::run([]() { return psig::EnumerateProcesses(); }));
}

void MainViewModel::onProcessesEnumerated() {
    std::vector<ProcessRow> rows;
    for (const auto& p : m_procWatcher->result()) {
        ProcessRow r;
        r.pid = p.pid;
        r.name = QString::fromStdWString(p.name);
        r.path = QString::fromStdWString(p.path);
        r.accessible = p.accessible;
        rows.push_back(r);
    }
    m_procModel->setProcesses(rows);
    refreshPersistentStatus();
    m_view->setTransientMessage(tr("已枚举 %1 个进程").arg(rows.size()));
    m_view->appendLog(tr("[进程] 枚举 %1 个").arg(rows.size()));
}

void MainViewModel::onProcessFilterChanged(const QString& text) {
    auto* table = m_view->processPage()->table();
    const QString needle = text.trimmed();
    for (int r = 0; r < m_procModel->rowCount(); ++r) {
        const ProcessRow row = m_procModel->processAt(r);
        bool show = needle.isEmpty() || row.name.contains(needle, Qt::CaseInsensitive) ||
                    row.path.contains(needle, Qt::CaseInsensitive);
        table->setRowHidden(r, !show);
    }
}

std::vector<uint32_t> MainViewModel::selectedPids() const {
    std::vector<uint32_t> pids;
    auto* sel = m_view->processPage()->table()->selectionModel();
    if (!sel) return pids;
    for (const auto& idx : sel->selectedRows()) {
        ProcessRow r = m_procModel->processAt(idx.row());
        if (r.pid != 0) pids.push_back(r.pid);
    }
    return pids;
}

// ---------------------------------------------------------------------------
// 扫描
// ---------------------------------------------------------------------------

void MainViewModel::onStartScan() {
    std::string err;
    std::vector<psig::Pattern> patterns = collectPatterns(err);
    if (!err.empty()) {
        m_view->setTransientMessage(tr("特征码错误: %1").arg(QString::fromStdString(err)), true);
        return;
    }
    if (patterns.empty()) {
        m_view->setTransientMessage(tr("特征码表为空，无法扫描"), true);
        return;
    }
    std::vector<uint32_t> pids = selectedPids();
    if (pids.empty()) {
        m_view->setTransientMessage(tr("未选择目标进程（请在进程选择页多选）"), true);
        m_view->showProcessPage();
        return;
    }

    psig::ScanRequest req;
    req.pids = pids;
    req.patterns = patterns;
    m_cancel.store(false);
    psig::RequestCancel(false);

    m_view->setTransientMessage(tr("正在扫描 %1 个进程 × %2 个特征码…").arg(pids.size()).arg(patterns.size()));
    m_view->setProgress(0);
    m_view->appendLog(tr("[扫描] 开始: %1 进程 × %2 特征码").arg(pids.size()).arg(patterns.size()));

    MainWindow* view = m_view;
    std::atomic_bool* cancelPtr = &m_cancel;
    auto cb = [view](const psig::ScanProgress& sp) {
        const int pct = sp.total ? static_cast<int>(sp.done * 100 / sp.total) : 0;
        const uint32_t pid = sp.currentPid;
        const size_t hits = sp.hits;
        QMetaObject::invokeMethod(view, [view, pct, pid, hits]() {
            view->setProgress(pct);
            view->setTransientMessage(QStringLiteral("扫描 pid=%1 … 命中 %2").arg(pid).arg(hits));
        }, Qt::QueuedConnection);
    };
    m_scanWatcher->setFuture(QtConcurrent::run([req, cb, cancelPtr]() {
        return psig::ScanProcesses(req, cb, cancelPtr);
    }));
}

void MainViewModel::onCancelScan() {
    m_cancel.store(true);
    psig::RequestCancel(true);
    m_view->setTransientMessage(tr("正在取消扫描…"));
}

void MainViewModel::onScanFinished() {
    std::vector<ResultRow> rows;
    for (const auto& m : m_scanWatcher->result()) rows.push_back(toResultRow(m));
    m_resultModel->setResults(rows);
    m_view->setProgress(-1);
    m_view->showResultPage();
    const bool cancelled = m_cancel.load();
    m_view->setTransientMessage(cancelled ? tr("已取消，命中 %1 条").arg(rows.size())
                                          : tr("扫描完成，命中 %1 条").arg(rows.size()));
    m_view->appendLog(tr("[扫描] %1，命中 %2 条")
                          .arg(cancelled ? tr("已取消") : tr("完成"))
                          .arg(rows.size()));
}

// ---------------------------------------------------------------------------
// 结果导出 / 清空 / 日志
// ---------------------------------------------------------------------------

void MainViewModel::onExportResultsCsv() {
    if (m_resultModel->rowCount() == 0) {
        m_view->setTransientMessage(tr("无结果可导出"), true);
        return;
    }
    const QString path = QFileDialog::getSaveFileName(m_view, tr("导出结果 CSV"), QString(),
                                                      tr("CSV (*.csv)"));
    if (path.isEmpty()) return;
    exportResultsToPath(path);
}

void MainViewModel::onExportResultsDb() {
    if (m_resultModel->rowCount() == 0) {
        m_view->setTransientMessage(tr("无结果可导出"), true);
        return;
    }
    const QString path = QFileDialog::getSaveFileName(m_view, tr("导出结果 DB"), QString(),
                                                      tr("SQLite (*.db)"));
    if (path.isEmpty()) return;
    exportResultsToPath(path);
}

void MainViewModel::onExportResultsTxt() {
    if (m_resultModel->rowCount() == 0) {
        m_view->setTransientMessage(tr("无结果可导出"), true);
        return;
    }
    const QString path = QFileDialog::getSaveFileName(m_view, tr("导出结果 TXT"), QString(),
                                                      tr("文本 (*.txt)"));
    if (path.isEmpty()) return;
    exportResultsToPath(path);
}

void MainViewModel::exportResultsToPath(const QString& path) {
    if (m_resultModel->rowCount() == 0) {
        m_view->setTransientMessage(tr("无结果可导出"), true);
        return;
    }
    std::vector<psig::MatchResult> results;
    for (const auto& r : m_resultModel->results()) results.push_back(toPsigResult(r));
    std::string err;
    bool ok = false;
    if (path.endsWith(".db", Qt::CaseInsensitive)) {
        ok = psig::ExportResultsDb(path.toStdString(), results, err);
    } else if (path.endsWith(".txt", Qt::CaseInsensitive)) {
        ok = psig::ExportResultsTxt(path.toStdString(), results, err);
    } else {
        ok = psig::ExportResultsCsv(path.toStdString(), results, err);
    }
    if (!ok) {
        m_view->setTransientMessage(tr("导出失败: %1").arg(QString::fromStdString(err)), true);
        return;
    }
    m_view->setTransientMessage(tr("已导出: %1").arg(path));
    m_view->appendLog(tr("[导出] 结果 → %1").arg(path));
}

void MainViewModel::onClearResults() {
    if (m_resultModel->rowCount() == 0) return;
    if (QMessageBox::question(m_view, tr("确认"), tr("确定清空全部匹配结果？")) != QMessageBox::Yes) {
        return; // 取消路径
    }
    m_resultModel->clear();
    m_view->setTransientMessage(tr("结果已清空"));
    m_view->appendLog(tr("[结果] 已清空"));
}

void MainViewModel::onClearLog() {
    m_view->clearLog();
}

// ---------------------------------------------------------------------------
// 转换辅助
// ---------------------------------------------------------------------------

std::vector<psig::Pattern> MainViewModel::collectPatterns(std::string& err) const {
    err.clear();
    std::vector<psig::Pattern> out;
    const int n = m_sigModel->rowCount();
    for (int i = 0; i < n; ++i) {
        const QString raw = m_sigModel->valueAt(i, QStringLiteral("pattern")).trimmed();
        if (raw.isEmpty()) continue;
        const QString name = m_sigModel->valueAt(i, QStringLiteral("name"));
        psig::Pattern p = psig::ParsePattern(raw.toStdString(),
                                             name.isEmpty() ? raw.toStdString() : name.toStdString());
        if (!p.valid) {
            err = "第 " + std::to_string(i + 1) + " 行模式非法: " + p.error;
            return {};
        }
        out.push_back(p);
    }
    return out;
}

psig::Table MainViewModel::tableFromModel() const {
    psig::Table t;
    for (const auto& c : m_sigModel->columns()) t.columns.push_back(toPsigColumn(c));
    for (const auto& r : m_sigModel->rows()) {
        psig::TableRow tr;
        for (const auto& cell : r.cells) tr.cells.push_back(cell.toStdString());
        t.rows.push_back(tr);
    }
    return t;
}

void MainViewModel::applyTableToModel(const psig::Table& table) {
    std::vector<ColumnDef> cols;
    for (const auto& c : table.columns) cols.push_back(toModelColumn(c));
    if (!cols.empty()) m_sigModel->setColumns(cols);
    std::vector<SignatureRow> rows;
    for (const auto& r : table.rows) {
        SignatureRow sr;
        for (const auto& cell : r.cells) sr.cells << QString::fromStdString(cell);
        rows.push_back(sr);
    }
    m_sigModel->setRows(rows);
}
