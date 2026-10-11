// view/mainwindow.cpp - 主窗口骨架实现（仅 UI）
#include "view/mainwindow.h"

#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QKeySequence>
#include <QLabel>
#include <QMenuBar>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QToolBar>

#include "view/processselectionpage.h"
#include "view/resultpage.h"
#include "view/signatureeditorpage.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(tr("进程特征码批量匹配器 v0.2.0"));
    resize(1100, 720);

    buildMenuAndToolbar();
    buildCentral();
    buildStatusAndLog();
}

void MainWindow::buildMenuAndToolbar() {
    m_importAction = new QAction(tr("导入特征码"), this);
    m_exportAction = new QAction(tr("导出特征码"), this);
    m_refreshAction = new QAction(tr("刷新进程"), this);
    m_startScanAction = new QAction(tr("开始扫描"), this);
    m_cancelScanAction = new QAction(tr("取消扫描"), this);
    m_manageColumnsAction = new QAction(tr("列管理"), this);
    m_addRowAction = new QAction(tr("新增行"), this);
    m_confirmProcessAction = new QAction(tr("确认选择"), this);
    m_reselectProcessAction = new QAction(tr("重新选择进程"), this);
    m_clearResultsAction = new QAction(tr("清空结果"), this);

    // 图标（用标准图标，替代纯文字；tooltip 保留文字）
    QStyle* st = style();
    m_importAction->setIcon(st->standardIcon(QStyle::SP_DialogOpenButton));
    m_exportAction->setIcon(st->standardIcon(QStyle::SP_DialogSaveButton));
    m_refreshAction->setIcon(st->standardIcon(QStyle::SP_BrowserReload));
    m_startScanAction->setIcon(st->standardIcon(QStyle::SP_MediaPlay));
    m_cancelScanAction->setIcon(st->standardIcon(QStyle::SP_MediaStop));
    m_manageColumnsAction->setIcon(st->standardIcon(QStyle::SP_FileDialogDetailedView));
    m_addRowAction->setIcon(st->standardIcon(QStyle::SP_FileDialogNewFolder));
    m_confirmProcessAction->setIcon(st->standardIcon(QStyle::SP_DialogApplyButton));
    m_reselectProcessAction->setIcon(st->standardIcon(QStyle::SP_ArrowBack));
    m_clearResultsAction->setIcon(st->standardIcon(QStyle::SP_TrashIcon));

    // 键盘快捷键：作为可确定性注入的操作入口（E2E 与快捷键用户）
    m_importAction->setShortcut(QKeySequence::Open);                            // Ctrl+O
    m_exportAction->setShortcut(QKeySequence::Save);                            // Ctrl+S
    m_refreshAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+R")));
    m_startScanAction->setShortcut(QKeySequence(Qt::Key_F5));
    m_cancelScanAction->setShortcut(QKeySequence(QStringLiteral("Shift+Esc")));
    m_manageColumnsAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+M")));
    m_addRowAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+N")));
    m_confirmProcessAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+Return")));

    QMenu* fileMenu = menuBar()->addMenu(tr("文件(&F)"));
    fileMenu->addAction(m_importAction);
    fileMenu->addAction(m_exportAction);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("退出"), this, &QWidget::close);

    QMenu* editMenu = menuBar()->addMenu(tr("编辑(&E)"));
    editMenu->addAction(m_addRowAction);
    editMenu->addAction(m_manageColumnsAction);

    QMenu* scanMenu = menuBar()->addMenu(tr("扫描(&S)"));
    scanMenu->addAction(m_refreshAction);
    scanMenu->addAction(m_confirmProcessAction);
    scanMenu->addSeparator();
    scanMenu->addAction(m_startScanAction);
    scanMenu->addAction(m_cancelScanAction);
    scanMenu->addAction(m_reselectProcessAction);

    QMenu* helpMenu = menuBar()->addMenu(tr("帮助(&H)"));
    m_aboutAction = new QAction(tr("关于"), this);
    m_aboutAction->setShortcut(QKeySequence(Qt::Key_F1));
    helpMenu->addAction(m_aboutAction);

    // 主工具栏：图标化；按页显隐
    m_toolbar = addToolBar(tr("主工具栏"));
    m_toolbar->setMovable(false);
    m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolbar->addAction(m_refreshAction);
    m_toolbar->addAction(m_confirmProcessAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_importAction);
    m_toolbar->addAction(m_exportAction);
    m_toolbar->addAction(m_startScanAction);
    m_toolbar->addAction(m_cancelScanAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_clearResultsAction);
    m_toolbar->addAction(m_reselectProcessAction);
}

void MainWindow::buildCentral() {
    m_stack = new QStackedWidget(this);
    m_signaturePage = new SignatureEditorPage(this);
    m_processPage = new ProcessSelectionPage(this);
    m_resultPage = new ResultPage(this);
    m_stack->addWidget(m_signaturePage); // index 0
    m_stack->addWidget(m_processPage);   // index 1
    m_stack->addWidget(m_resultPage);    // index 2
    setCentralWidget(m_stack);

    // 视图导航动作（仅暴露给 ViewModel，由 VM connect）
    m_showSignatureAction = new QAction(tr("特征码编辑"), this);
    m_showProcessAction = new QAction(tr("进程选择"), this);
    m_showResultAction = new QAction(tr("匹配结果"), this);
    m_clearLogAction = new QAction(tr("清空日志"), this);
    m_showSignatureAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+1")));
    m_showProcessAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+2")));
    m_showResultAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+3")));
    m_clearLogAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+L")));
    QMenu* viewMenu = menuBar()->addMenu(tr("视图(&V)"));
    viewMenu->addAction(m_showProcessAction);
    viewMenu->addAction(m_showSignatureAction);
    viewMenu->addAction(m_showResultAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_clearLogAction);
}

void MainWindow::buildStatusAndLog() {
    // 历史日志：独立 Dock（不写入状态栏）
    QDockWidget* dock = new QDockWidget(tr("历史日志"), this);
    m_logEdit = new QPlainTextEdit(dock);
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumBlockCount(2000);
    dock->setWidget(m_logEdit);
    addDockWidget(Qt::BottomDockWidgetArea, dock);

    // 状态栏：左=持久状态区，右=瞬时反馈区；进度条置于左区
    m_persistentLabel = new QLabel(tr("就绪 | 目标 - | 进程 - | 特征码 -"), this);
    m_transientLabel = new QLabel(QString(), this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setFixedWidth(160);
    m_progressBar->setVisible(false);

    statusBar()->addWidget(m_persistentLabel);
    statusBar()->addWidget(m_progressBar);
    statusBar()->addPermanentWidget(m_transientLabel);
}

void MainWindow::showSignaturePage() {
    m_stack->setCurrentWidget(m_signaturePage);
    applyToolbarForPage(m_stack->currentIndex());
}
void MainWindow::showProcessPage() {
    m_stack->setCurrentWidget(m_processPage);
    applyToolbarForPage(m_stack->currentIndex());
}
void MainWindow::showResultPage() {
    m_stack->setCurrentWidget(m_resultPage);
    applyToolbarForPage(m_stack->currentIndex());
}

void MainWindow::applyToolbarForPage(int index) {
    const bool processPage = (index == 1);
    const bool sigPage = (index == 0);
    const bool resPage = (index == 2);
    m_refreshAction->setVisible(processPage);
    m_confirmProcessAction->setVisible(processPage);
    m_importAction->setVisible(sigPage);
    m_exportAction->setVisible(sigPage);
    m_startScanAction->setVisible(sigPage);
    m_cancelScanAction->setVisible(sigPage);
    m_clearResultsAction->setVisible(resPage);
    m_reselectProcessAction->setVisible(m_processSelectionLocked && !processPage);
}

void MainWindow::setProcessSelectionLocked(bool locked) {
    m_processSelectionLocked = locked;
    m_showProcessAction->setVisible(!locked);   // 视图菜单：选定后隐藏「进程选择」
    applyToolbarForPage(m_stack->currentIndex());
}

void MainWindow::setPersistentStatus(const QString& text) { m_persistentLabel->setText(text); }

void MainWindow::setTransientMessage(const QString& text, bool error) {
    m_transientLabel->setStyleSheet(error ? QStringLiteral("color:#c62828;") : QString());
    m_transientLabel->setText(text);
}

void MainWindow::setProgress(int value) {
    if (value < 0) {
        m_progressBar->setVisible(false);
        return;
    }
    m_progressBar->setVisible(true);
    m_progressBar->setValue(value);
}

void MainWindow::appendLog(const QString& text) { m_logEdit->appendPlainText(text); }

void MainWindow::clearLog() { m_logEdit->clear(); }
