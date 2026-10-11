// viewmodel/mainviewmodel.cpp - ViewModel 实现
#include "viewmodel/mainviewmodel.h"

#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QImage>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QLocale>
#include <QMenu>
#include <QMessageBox>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableView>
#include <QTranslator>
#include <QtConcurrent>

#include "view/aboutdialog.h"
#include "view/columnmanagerdialog.h"
#include "view/exportcolumnsdialog.h"
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
    r.custom = QString::fromStdString(m.custom);
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
    m.custom = r.custom.toStdString();
    return m;
}

// psig 的 BGRA 位图 → 独立缓冲的 QImage（ARGB32）
QImage toQImage(const psig::IconBitmap& bmp) {
    if (!bmp.valid()) return {};
    QImage img(reinterpret_cast<const uchar*>(bmp.pixels.data()), bmp.width, bmp.height,
               bmp.width * 4, QImage::Format_ARGB32);
    return img.copy(); // 脱离临时缓冲，持有一份独立数据
}

// 结果导出列定义（键, 标题），顺序与 psig 结果列一致
std::vector<std::pair<QString, QString>> resultColumnDefs() {
    return {{QStringLiteral("pattern_name"), QObject::tr("特征码名")},
            {QStringLiteral("process_name"), QObject::tr("进程")},
            {QStringLiteral("pid"), QObject::tr("PID")},
            {QStringLiteral("address"), QObject::tr("地址")},
            {QStringLiteral("address_dec"), QObject::tr("地址(十进制)")}, // C-01：新增十进制地址列
            {QStringLiteral("module_name"), QObject::tr("模块")},
            {QStringLiteral("module_base"), QObject::tr("模块基址")},
            {QStringLiteral("matched_bytes"), QObject::tr("匹配字节")},
            {QStringLiteral("hit_time"), QObject::tr("命中时间")},
            {QStringLiteral("custom"), QObject::tr("备注")}};             // C-01：自定义 → 备注
}

// 结果导出表头：按当前语言（经 tr()）输出，与界面结果表一致；自定义列固定「备注」不随语言（C-05/C-06）
QString resultHeaderForKey(const std::string& key) {
    if (key == "custom") return QStringLiteral("备注"); // 自定义列不随语言
    for (const auto& p : resultColumnDefs()) {
        if (p.first.toStdString() == key) return p.second;
    }
    return QString();
}

// 特征码表内置列标题：按当前语言（经 tr()）；自定义列沿用其标题（不随语言，C-06）
QString signatureHeaderForKey(const QString& key) {
    if (key == QStringLiteral("name")) return QObject::tr("名称");
    if (key == QStringLiteral("pattern")) return QObject::tr("特征码");
    if (key == QStringLiteral("note")) return QObject::tr("备注");
    if (key == QStringLiteral("process")) return QObject::tr("进程");
    return QString();
}

// 对话框按钮接线（View 不 connect；统一在 VM 完成）
void wireDialog(QDialog* dlg, QDialogButtonBox* box) {
    if (!dlg || !box) return;
    QObject::connect(box, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
}

} // namespace

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

    m_procWatcher = new QFutureWatcher<std::vector<psig::ProcessInfo>>(this);
    m_scanWatcher = new QFutureWatcher<std::vector<psig::MatchResult>>(this);
    connect(m_procWatcher, &QFutureWatcherBase::finished, this, &MainViewModel::onProcessesEnumerated);
    connect(m_scanWatcher, &QFutureWatcherBase::finished, this, &MainViewModel::onScanFinished);

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
    logTagged(QStringLiteral("startup"), tr("进程特征码批量匹配器 v0.2.2 已就绪"));

    onRefreshProcesses(); // C-09：启动后自动枚举一次进程
}

void MainViewModel::bindSignaturePage() {
    // 行操作改由表格右键菜单触发
    auto* page = m_view->signaturePage();
    connect(page->addRowAction(), &QAction::triggered, this, &MainViewModel::onAddSignatureRow);
    connect(page->addRowsAction(), &QAction::triggered, this, &MainViewModel::onAddSignatureRows);
    connect(page->deleteRowsAction(), &QAction::triggered, this, &MainViewModel::onDeleteSignatureRows);
    connect(page->manageColumnsAction(), &QAction::triggered, this, &MainViewModel::onManageColumns);
}

void MainViewModel::bindProcessPage() {
    auto* page = m_view->processPage();
    connect(page->refreshAction(), &QAction::triggered, this, &MainViewModel::onRefreshProcesses);
    connect(page->useAsTargetAction(), &QAction::triggered, this, &MainViewModel::onConfirmProcessSelection);
    connect(page->selectAllAction(), &QAction::triggered, this, [this]() {
        if (m_view->processPage()->table()->model())
            m_view->processPage()->table()->selectAll();
    });
    connect(page->filterEdit(), &QLineEdit::textChanged, this, &MainViewModel::onProcessFilterChanged);
    // 双击进程行 = 确认选择
    connect(page->table(), &QTableView::doubleClicked, this, [this](const QModelIndex&) {
        onConfirmProcessSelection();
    });
    // 按列排序（点击表头）后重应用筛选（C-10/C-11）
    connect(page->table()->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this,
            [this](int, Qt::SortOrder) { applyProcessFilter(); });
}

void MainViewModel::bindResultPage() {
    auto* page = m_view->resultPage();
    connect(page->exportCsvAction(), &QAction::triggered, this, &MainViewModel::onExportResultsCsv);
    connect(page->exportDbAction(), &QAction::triggered, this, &MainViewModel::onExportResultsDb);
    connect(page->exportTxtAction(), &QAction::triggered, this, &MainViewModel::onExportResultsTxt);
    connect(page->selectColumnsAction(), &QAction::triggered, this, &MainViewModel::onSelectExportColumns);
    connect(page->deleteRowsAction(), &QAction::triggered, this, &MainViewModel::onDeleteResultRows);
    connect(page->fillCustomAction(), &QAction::triggered, this, &MainViewModel::onFillCustomColumn);
    connect(page->clearAction(), &QAction::triggered, this, &MainViewModel::onClearResults);
    connect(page->templateEdit(), &QPlainTextEdit::textChanged, this, &MainViewModel::onTemplateTextChanged);
}

void MainViewModel::bindGlobalActions() {
    connect(m_view->importSignaturesAction(), &QAction::triggered, this, &MainViewModel::onImportSignatures);
    connect(m_view->exportSignaturesAction(), &QAction::triggered, this, &MainViewModel::onExportSignatures);
    connect(m_view->refreshProcessAction(), &QAction::triggered, this, &MainViewModel::onRefreshProcesses);
    connect(m_view->startScanAction(), &QAction::triggered, this, &MainViewModel::onStartScan);
    connect(m_view->cancelScanAction(), &QAction::triggered, this, &MainViewModel::onCancelScan);
    connect(m_view->manageColumnsAction(), &QAction::triggered, this, &MainViewModel::onManageColumns);
    connect(m_view->addSignatureRowAction(), &QAction::triggered, this, &MainViewModel::onAddSignatureRow);
    connect(m_view->addSignatureRowsAction(), &QAction::triggered, this, &MainViewModel::onAddSignatureRows);
    connect(m_view->confirmProcessAction(), &QAction::triggered, this, &MainViewModel::onConfirmProcessSelection);
    connect(m_view->reselectProcessAction(), &QAction::triggered, this, &MainViewModel::onReselectProcess);
    connect(m_view->clearResultsAction(), &QAction::triggered, this, &MainViewModel::onClearResults);
    connect(m_view->showSignaturePageAction(), &QAction::triggered, m_view, &MainWindow::showSignaturePage);
    connect(m_view->showProcessPageAction(), &QAction::triggered, m_view, &MainWindow::showProcessPage);
    connect(m_view->showResultPageAction(), &QAction::triggered, m_view, &MainWindow::showResultPage);
    connect(m_view->aboutAction(), &QAction::triggered, this, &MainViewModel::onAbout);
    // 语言菜单（三项互斥，触发即切换）
    connect(m_view->languageZhAction(), &QAction::triggered, this,
            [this]() { applyLanguage(QStringLiteral("zh_CN")); });
    connect(m_view->languageEnAction(), &QAction::triggered, this,
            [this]() { applyLanguage(QStringLiteral("en_US")); });
    connect(m_view->languageSystemAction(), &QAction::triggered, this,
            [this]() { applyLanguage(QString()); });
}

void MainViewModel::bindContextMenus() {
    connect(m_view->signaturePage()->table(), &QWidget::customContextMenuRequested,
            this, &MainViewModel::onSignatureContextMenu);
    connect(m_view->processPage()->table(), &QWidget::customContextMenuRequested,
            this, &MainViewModel::onProcessContextMenu);
    connect(m_view->resultPage()->table(), &QWidget::customContextMenuRequested,
            this, &MainViewModel::onResultContextMenu);
}

void MainViewModel::onSignatureContextMenu(const QPoint& pos) {
    auto* t = m_view->signaturePage()->table();
    m_view->signaturePage()->contextMenu()->exec(t->viewport()->mapToGlobal(pos));
}

void MainViewModel::onProcessContextMenu(const QPoint& pos) {
    auto* t = m_view->processPage()->table();
    m_view->processPage()->contextMenu()->exec(t->viewport()->mapToGlobal(pos));
}

void MainViewModel::onResultContextMenu(const QPoint& pos) {
    auto* page = m_view->resultPage();
    page->exportCsvAction()->setEnabled(m_resultModel->rowCount() > 0);
    page->exportDbAction()->setEnabled(m_resultModel->rowCount() > 0);
    page->exportTxtAction()->setEnabled(m_resultModel->rowCount() > 0);
    auto* t = page->table();
    page->contextMenu()->exec(t->viewport()->mapToGlobal(pos));
}

// C-08：日志开关（视图菜单）与日志窗口右键「清空日志」
void MainViewModel::bindLogActions() {
    // 视图菜单的日志开关
    connect(m_view->toggleLogAction(), &QAction::triggered, this, &MainViewModel::onToggleLog);
    m_view->setToggleLogChecked(true); // 日志 dock 默认随主窗口显示
    // 日志窗口右键菜单的「清空日志」
    connect(m_view->logEdit(), &QPlainTextEdit::customContextMenuRequested, this,
            &MainViewModel::onLogContextMenu);
    connect(m_view->clearLogAction(), &QAction::triggered, this, &MainViewModel::onClearLog);
}

void MainViewModel::onLogContextMenu(const QPoint& pos) {
    m_view->logContextMenu()->exec(m_view->logEdit()->viewport()->mapToGlobal(pos));
}

void MainViewModel::onToggleLog() {
    const bool now = !m_view->logDockVisible();
    m_view->setLogDockVisible(now);
    m_view->setToggleLogChecked(now);
}

void MainViewModel::applyInitialColumns() {
    std::vector<ColumnDef> cols;
    for (const auto& c : psig::BuiltinSignatureColumns()) cols.push_back(toModelColumn(c));
    // C-08：「进程」列（GUI 侧内置，键 process）——新增行时自动填当前目标进程名
    ColumnDef process;
    process.key = QStringLiteral("process");
    process.title = QStringLiteral("进程");
    process.type = QStringLiteral("text");
    process.builtin = true;
    cols.push_back(process);
    m_sigModel->setColumns(cols);
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
    m_view->setPersistentStatus(
        tr("就绪 | 目标 %1 | 进程 %2 | 特征码 %3").arg(target).arg(procs).arg(patterns));
}

void MainViewModel::markSignatureDirty() { m_sigDirty = true; }

// ---------------------------------------------------------------------------
// 语言
// ---------------------------------------------------------------------------

void MainViewModel::applyLanguage(const QString& code, bool feedback) {
    m_language = code;
    // 解析实际生效语言：空 = 跟随系统（中文为源语言，非中文回退英文）
    QString effective = code;
    if (effective.isEmpty()) {
        effective = QLocale().name().startsWith(QStringLiteral("zh"))
                        ? QStringLiteral("zh_CN")
                        : QStringLiteral("en_US");
    }

    if (m_translator) {
        QApplication::removeTranslator(m_translator);
        delete m_translator;
        m_translator = nullptr;
    }
    // 源语言为中文：仅非中文目标需要装载翻译
    if (effective != QStringLiteral("zh_CN")) {
        auto* translator = new QTranslator(this);
        if (translator->load(QStringLiteral(":/i18n/psig_gui_") + effective + QStringLiteral(".qm"))) {
            QApplication::installTranslator(translator);
            m_translator = translator;
        } else {
            delete translator;
        }
    }

    if (m_view) {
        m_view->setLanguageChecked(code);
        m_view->retranslateUi();
    }
    if (m_sigModel) m_sigModel->retranslate();
    if (m_procModel) m_procModel->retranslate();
    if (m_resultModel) m_resultModel->retranslate();

    if (feedback && m_view) {
        const QString label = (effective == QStringLiteral("zh_CN")) ? QStringLiteral("中文")
                                                                     : QStringLiteral("English");
        m_view->setTransientMessage(tr("语言已切换为 %1").arg(label));
        logTagged(QStringLiteral("settings"), tr("语言 → %1").arg(effective));
    }
}

// ---------------------------------------------------------------------------
// 特征码表
// ---------------------------------------------------------------------------

void MainViewModel::onAddSignatureRow() {
    m_sigModel->addRow(m_targetName); // C-08：自动填进程列
    markSignatureDirty();             // C-04：标记为未保存改动
    m_view->signaturePage()->selectRow(m_sigModel->rowCount() - 1);
    refreshPersistentStatus();
    m_view->setTransientMessage(tr("已新增 1 行"));
    logTagged(QStringLiteral("edit"), tr("新增特征码 1 行"));
}

void MainViewModel::onAddSignatureRows() {
    bool ok = false;
    const int count = QInputDialog::getInt(m_view, tr("新增多行"), tr("新增行数："), 1, 1, 10000, 1, &ok);
    if (!ok) return; // 取消路径
    m_sigModel->addRows(count, m_targetName);
    markSignatureDirty();             // C-04：标记为未保存改动
    m_view->signaturePage()->selectRow(m_sigModel->rowCount() - 1);
    refreshPersistentStatus();
    m_view->setTransientMessage(tr("已新增 %1 行").arg(count));
    logTagged(QStringLiteral("edit"), tr("新增特征码 %1 行").arg(count));
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
    markSignatureDirty();             // C-04：标记为未保存改动
    refreshPersistentStatus();
    m_view->setTransientMessage(tr("已删除 %1 行").arg(rows.size()));
    logTagged(QStringLiteral("edit"), tr("删除特征码 %1 行").arg(rows.size()));
}

void MainViewModel::onManageColumns() {
    ColumnManagerDialog dlg(m_view);
    // C-02：键下拉选项来自 psig 内置列键（View 不依赖 psig，由 VM 注入）
    QStringList keyOpts;
    for (const auto& c : psig::BuiltinSignatureColumns()) keyOpts << QString::fromStdString(c.key);
    dlg.setKeyOptions(keyOpts);
    dlg.setColumns(m_sigModel->columns());
    wireDialog(&dlg, dlg.buttonBox());
    connect(dlg.addBtn(), &QPushButton::clicked, &dlg, &ColumnManagerDialog::addColumnRow);
    connect(dlg.removeBtn(), &QPushButton::clicked, &dlg, &ColumnManagerDialog::removeSelectedColumnRow);
    connect(dlg.upBtn(), &QPushButton::clicked, &dlg, &ColumnManagerDialog::moveSelectedUp);
    connect(dlg.downBtn(), &QPushButton::clicked, &dlg, &ColumnManagerDialog::moveSelectedDown);
    if (dlg.exec() != QDialog::Accepted) return;
    m_sigModel->setColumns(dlg.columns());
    markSignatureDirty();             // C-04：列定义更新属未保存改动
    m_view->setTransientMessage(tr("列已更新"));
    logTagged(QStringLiteral("edit"), tr("更新列定义"));
}

void MainViewModel::onImportSignatures() {
    // 前置询问：当前特征码表非空时先问是否保存（需决策 → 弹窗）
    if (m_sigModel->rowCount() > 0) {
        QMessageBox box(m_view);
        box.setWindowTitle(tr("导入前确认"));
        box.setText(tr("当前特征码表可能未保存，是否先保存？"));
        QPushButton* saveBtn = box.addButton(tr("保存"), QMessageBox::AcceptRole);
        box.addButton(tr("不保存"), QMessageBox::DestructiveRole);
        QPushButton* cancelBtn = box.addButton(tr("取消"), QMessageBox::RejectRole);
        box.exec();
        if (box.clickedButton() == cancelBtn) return; // 取消路径
        if (box.clickedButton() == saveBtn) {
            const QString savePath = QFileDialog::getSaveFileName(
                m_view, tr("保存特征码"), QString(), tr("CSV (*.csv);;SQLite (*.db)"));
            if (savePath.isEmpty()) return; // 取消保存 → 中止导入
            exportSignaturesToPath(savePath);
        }
    }
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
        logTagged(QStringLiteral("error"), tr("导入失败: %1").arg(QString::fromStdString(err)));
        return;
    }
    applyTableToModel(table);
    m_sigDirty = false;               // C-04：导入后视为已保存
    refreshPersistentStatus();
    m_view->setTransientMessage(tr("已导入 %1 条").arg(table.rows.size()));
    logTagged(QStringLiteral("import"), tr("%1 条").arg(table.rows.size()));
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
        logTagged(QStringLiteral("error"), tr("导出失败: %1").arg(QString::fromStdString(err)));
        return;
    }
    m_sigDirty = false;               // C-04：导出后视为已保存
    m_view->setTransientMessage(tr("已导出: %1").arg(path));
    logTagged(QStringLiteral("export"), tr("特征码 → %1").arg(path));
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
    applyProcessFilter();   // C-10：刷新后重应用当前筛选
    startIconLoad();        // C-12：异步回填进程图标
    refreshPersistentStatus();
    m_view->setTransientMessage(tr("已枚举 %1 个进程").arg(rows.size()));
    logTagged(QStringLiteral("process"), tr("枚举 %1 个").arg(rows.size()));
}

void MainViewModel::applyProcessFilter() {
    if (!m_view) return;
    onProcessFilterChanged(m_view->processPage()->filterEdit()->text());
}

void MainViewModel::onProcessFilterChanged(const QString& text) {
    auto* table = m_view->processPage()->table();
    const QString needle = text.trimmed();
    for (int r = 0; r < m_procModel->rowCount(); ++r) {
        // C-03：按展示名过滤（空名进程使用系统进程兜底名）
        const QString name = m_procModel->data(m_procModel->index(r, 1)).toString();
        const ProcessRow row = m_procModel->processAt(r);
        bool show = needle.isEmpty() || name.contains(needle, Qt::CaseInsensitive) ||
                    row.path.contains(needle, Qt::CaseInsensitive);
        table->setRowHidden(r, !show);
    }
}

void MainViewModel::startIconLoad() {
    if (!m_procModel || m_procModel->rowCount() == 0) return;
    std::vector<uint32_t> pids;
    pids.reserve(static_cast<size_t>(m_procModel->rowCount()));
    // C-09：仅对可访问（有权限）的进程取图标；无权限进程不显示图标（不阻止选择）
    for (const auto& r : m_procModel->processes()) {
        if (r.accessible) pids.push_back(r.pid);
    }

    MainViewModel* self = this;
    std::shared_ptr<std::atomic_bool> alive = m_alive;
    // 后台顺序取图标；不保留 QFuture（任务独立运行），仅以 alive 标志保证析构安全
    QFuture<void> iconFuture = QtConcurrent::run([self, pids, alive]() {
        for (uint32_t pid : pids) {
            if (!alive->load()) return; // VM 已析构，停止回填
            psig::IconBitmap bmp = psig::ProcessIcon(pid);
            if (!bmp.valid()) bmp = psig::DefaultProcessIcon();
            QImage img = toQImage(bmp);
            if (img.isNull()) continue;
            if (!alive->load()) return;
            QMetaObject::invokeMethod(self, [self, pid, img]() {
                self->applyProcessIcon(pid, QIcon(QPixmap::fromImage(img)));
            }, Qt::QueuedConnection);
        }
    });
    Q_UNUSED(iconFuture);
}

void MainViewModel::applyProcessIcon(uint32_t pid, const QIcon& icon) {
    if (!m_procModel || icon.isNull()) return;
    const int n = m_procModel->rowCount();
    for (int r = 0; r < n; ++r) {
        // C-09：仅对可访问进程注入图标
        if (m_procModel->processAt(r).pid == pid && m_procModel->processAt(r).accessible) {
            m_procModel->setIcon(r, icon);
            break;
        }
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

void MainViewModel::onConfirmProcessSelection() {
    auto* sel = m_view->processPage()->table()->selectionModel();
    if (!sel || sel->selectedRows().isEmpty()) {
        m_view->setTransientMessage(tr("请先选择目标进程"), true); // 错误路径
        return;
    }
    const int row = sel->selectedRows().first().row();
    const ProcessRow r = m_procModel->processAt(row);
    m_targetPid = r.pid;
    // C-03：空名进程（如 PID 0/4）沿用表格展示名（系统进程兜底名），避免目标名为空
    m_targetName = m_procModel->data(m_procModel->index(row, 1)).toString();
    if (m_targetName.isEmpty()) m_targetName = r.name;
    m_view->setProcessSelectionLocked(true);
    m_view->showSignaturePage();
    refreshPersistentStatus();
    m_view->setTransientMessage(tr("已选定目标：%1 (pid %2)").arg(m_targetName).arg(m_targetPid));
    logTagged(QStringLiteral("nav"), tr("选定目标 %1 (pid %2)").arg(m_targetName).arg(m_targetPid));
}

void MainViewModel::onReselectProcess() {
    if (m_sigModel->rowCount() > 0 && m_sigDirty) {
        if (!askSaveBeforeReselect()) return; // C-04：取消 → 中止重新选择
    }
    m_view->setProcessSelectionLocked(false);
    m_view->showProcessPage();
    m_view->setTransientMessage(tr("请重新选择目标进程"));
    logTagged(QStringLiteral("nav"), tr("返回进程页"));
}

// C-04：重新选择进程前，若特征码表有未保存改动，弹窗询问是否保存；返回 false 表示取消（中止切换）
bool MainViewModel::askSaveBeforeReselect() {
    QMessageBox box(m_view);
    box.setWindowTitle(tr("保存特征码"));
    box.setText(tr("当前特征码表尚未保存，是否保存？"));
    QPushButton* saveBtn = box.addButton(tr("保存"), QMessageBox::AcceptRole);
    box.addButton(tr("不保存"), QMessageBox::DestructiveRole);
    QPushButton* cancelBtn = box.addButton(tr("取消"), QMessageBox::RejectRole);
    box.exec();
    if (box.clickedButton() == cancelBtn) return false; // 取消 → 中止重新选择
    if (box.clickedButton() == saveBtn) {
        const QString path = QFileDialog::getSaveFileName(
            m_view, tr("保存特征码"), QString(), tr("CSV (*.csv);;SQLite (*.db)"));
        if (!path.isEmpty()) exportSignaturesToPath(path);
    }
    return true;
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
    logTagged(QStringLiteral("scan"), tr("开始: %1 进程 × %2 特征码").arg(pids.size()).arg(patterns.size()));

    MainWindow* view = m_view;
    std::atomic_bool* cancelPtr = &m_cancel;
    auto cb = [view](const psig::ScanProgress& sp) {
        const int pct = sp.total ? static_cast<int>(sp.done * 100 / sp.total) : 0;
        const uint32_t pid = sp.currentPid;
        const size_t hits = sp.hits;
        QMetaObject::invokeMethod(view, [view, pct, pid, hits]() {
            view->setProgress(pct);
            view->setTransientMessage(MainViewModel::tr("扫描 pid=%1 … 命中 %2").arg(pid).arg(hits));
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
    logTagged(QStringLiteral("scan"), tr("%1，命中 %2 条")
                                          .arg(cancelled ? tr("已取消") : tr("完成"))
                                          .arg(rows.size()));
}

// ---------------------------------------------------------------------------
// 结果导出 / 删行 / 填充 / 清空 / 日志
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
    const psig::ResultExportOptions opt = exportOptions();
    std::string err;
    bool ok = false;
    if (path.endsWith(".db", Qt::CaseInsensitive)) {
        ok = psig::ExportResultsDb(path.toStdString(), results, err, opt);
    } else if (path.endsWith(".txt", Qt::CaseInsensitive)) {
        ok = psig::ExportResultsTxt(path.toStdString(), results, err, opt);
    } else {
        ok = psig::ExportResultsCsv(path.toStdString(), results, err, opt);
    }
    if (!ok) {
        m_view->setTransientMessage(tr("导出失败: %1").arg(QString::fromStdString(err)), true);
        logTagged(QStringLiteral("error"), tr("导出失败: %1").arg(QString::fromStdString(err)));
        return;
    }
    m_view->setTransientMessage(tr("已导出: %1").arg(path));
    logTagged(QStringLiteral("export"), tr("结果 → %1").arg(path));
}

void MainViewModel::onDeleteResultRows() {
    auto* sel = m_view->resultPage()->table()->selectionModel();
    if (!sel) return;
    std::vector<int> rows;
    for (const auto& idx : sel->selectedRows()) rows.push_back(idx.row());
    if (rows.empty()) {
        m_view->setTransientMessage(tr("请先选择要删除的行"), true);
        return;
    }
    m_resultModel->removeRows(rows);
    m_view->setTransientMessage(tr("已删除 %1 行").arg(rows.size()));
    logTagged(QStringLiteral("result"), tr("删除 %1 行").arg(rows.size()));
}

void MainViewModel::onFillCustomColumn() {
    auto* sel = m_view->resultPage()->table()->selectionModel();
    std::vector<int> rows;
    if (sel) for (const auto& idx : sel->selectedRows()) rows.push_back(idx.row());

    bool fillAll = false;
    if (rows.empty()) {
        if (m_resultModel->rowCount() == 0) {
            m_view->setTransientMessage(tr("无结果可填充"), true);
            return;
        }
        if (QMessageBox::question(m_view, tr("批量填充"),
                tr("未选择行，是否填充全部 %1 行？").arg(m_resultModel->rowCount())) != QMessageBox::Yes) {
            m_view->setTransientMessage(tr("请选择要填充的行"), true);
            return;
        }
        fillAll = true;
    }

    bool ok = false;
    const QString text = QInputDialog::getMultiLineText(m_view, tr("批量填充自定义列"),
                                                        tr("自定义列内容："), m_templateText, &ok);
    if (!ok) return; // 取消路径

    std::vector<int> targets = rows;
    if (fillAll) {
        targets.clear();
        for (int i = 0; i < m_resultModel->rowCount(); ++i) targets.push_back(i);
    }
    m_resultModel->setCustom(targets, text);
    m_view->setTransientMessage(tr("已填充 %1 行自定义列").arg(targets.size()));
    logTagged(QStringLiteral("result"), tr("填充自定义列 %1 行").arg(targets.size()));
}

void MainViewModel::onTemplateTextChanged() {
    m_templateText = m_view->resultPage()->templateText();
}

void MainViewModel::onSelectExportColumns() {
    ExportColumnsDialog dlg(m_view);
    dlg.setColumns(resultColumnDefs(), m_exportColumns);
    wireDialog(&dlg, dlg.buttonBox());
    if (dlg.exec() != QDialog::Accepted) return;
    const QStringList keys = dlg.selectedKeys();
    if (keys.isEmpty()) {
        m_view->setTransientMessage(tr("至少选择一列"), true);
        return;
    }
    m_exportColumns = keys;
    m_view->setTransientMessage(tr("将导出 %1 列").arg(keys.size()));
    logTagged(QStringLiteral("result"), tr("选择导出列 %1 个").arg(keys.size()));
}

void MainViewModel::onAbout() {
#ifdef QT_DEBUG
    const QString cfg = QStringLiteral("Debug");
#else
    const QString cfg = QStringLiteral("Release");
#endif
    AboutDialog dlg(m_view);
    wireDialog(&dlg, dlg.buttonBox());
    dlg.setInfo(tr("进程特征码批量匹配器 v0.2.2\n"
                   "基于 psig 静态库（进程特征码批量匹配）\n"
                   "构建配置：%1\nQt 版本：%2")
                    .arg(cfg, QString::fromLatin1(qVersion())));
    dlg.exec();
}

void MainViewModel::onClearResults() {
    if (m_resultModel->rowCount() == 0) return;
    if (QMessageBox::question(m_view, tr("确认"), tr("确定清空全部匹配结果？")) != QMessageBox::Yes) {
        return; // 取消路径
    }
    m_resultModel->clear();
    m_view->setTransientMessage(tr("结果已清空"));
    logTagged(QStringLiteral("result"), tr("已清空"));
}

void MainViewModel::onClearLog() {
    m_view->clearLog();
}

// ---------------------------------------------------------------------------
// 转换辅助 / 测试注入
// ---------------------------------------------------------------------------

void MainViewModel::setTemplateText(const QString& text) {
    m_templateText = text;
    if (m_view) m_view->resultPage()->setTemplateText(text);
}

void MainViewModel::setExportColumns(const QStringList& keys) { m_exportColumns = keys; }

psig::ResultExportOptions MainViewModel::exportOptions() const {
    psig::ResultExportOptions o;
    o.templateText = m_templateText.toStdString();
    for (const auto& k : m_exportColumns) o.columns.push_back(k.toStdString());
    // C-05/C-06：表头随界面语言；按 psig 实际选出的键顺序构造，保证与列一一对应
    const std::vector<std::string> keys = psig::SelectResultKeys(o.columns);
    o.headers.reserve(keys.size());
    for (const auto& k : keys) o.headers.push_back(resultHeaderForKey(k).toStdString());
    return o;
}

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
    // C-06：内置列标题随语言（经 tr()）；自定义列沿用其标题（不随语言）
    for (const auto& c : m_sigModel->columns()) {
        psig::ColumnDef p;
        p.key = c.key.toStdString();
        p.type = c.type.toStdString();
        p.builtin = c.builtin;
        const QString h = c.builtin ? signatureHeaderForKey(c.key) : QString();
        p.title = (c.builtin && !h.isEmpty()) ? h.toStdString() : c.title.toStdString();
        t.columns.push_back(p);
    }
    for (const auto& r : m_sigModel->rows()) {
        psig::TableRow tr;
        for (const auto& cell : r.cells) tr.cells.push_back(cell.toStdString());
        t.rows.push_back(tr);
    }
    t.templateText = m_templateText.toStdString();
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
    // 模板随表携带
    m_templateText = QString::fromStdString(table.templateText);
    m_view->resultPage()->setTemplateText(m_templateText);
}
