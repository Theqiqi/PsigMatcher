// viewmodel/mainviewmodel_bind.cpp - ViewModel：控件/动作装配（bind*）与初始列
#include "viewmodel/mainviewmodel.h"

#include <QAction>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QPlainTextEdit>
#include <QTableView>

#include "view/mainwindow.h"
#include "view/processselectionpage.h"
#include "view/resultpage.h"
#include "view/signatureeditorpage.h"
#include "viewmodel/viewmodel_helpers.h"

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

void MainViewModel::bindLogActions() {
    // 视图菜单的日志开关
    connect(m_view->toggleLogAction(), &QAction::triggered, this, &MainViewModel::onToggleLog);
    m_view->setToggleLogChecked(true); // 日志 dock 默认随主窗口显示
    // 日志窗口右键菜单的「清空日志」
    connect(m_view->logEdit(), &QPlainTextEdit::customContextMenuRequested, this,
            &MainViewModel::onLogContextMenu);
    connect(m_view->clearLogAction(), &QAction::triggered, this, &MainViewModel::onClearLog);
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

void MainViewModel::applyInitialColumns() {
    std::vector<ColumnDef> cols;
    for (const auto& c : psig::BuiltinSignatureColumns()) cols.push_back(vmhelp::toModelColumn(c));
    // C-08：「进程」列（GUI 侧内置，键 process）——新增行时自动填当前目标进程名
    ColumnDef process;
    process.key = QStringLiteral("process");
    process.title = QStringLiteral("进程");
    process.type = QStringLiteral("text");
    process.builtin = true;
    cols.push_back(process);
    m_sigModel->setColumns(cols);
}
