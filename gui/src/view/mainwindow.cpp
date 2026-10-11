// view/mainwindow.cpp - 主窗口骨架实现（仅 UI）
#include "view/mainwindow.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QMenu>
#include <QDockWidget>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
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
    resize(1100, 720);

    buildMenuAndToolbar();
    buildCentral();
    buildStatusAndLog();
    retranslateUi();
}

void MainWindow::buildMenuAndToolbar() {
    createActions();
    applyActionIconsAndShortcuts();
    buildMenus();
    buildToolbar();
}

void MainWindow::createActions() {
    m_importAction = new QAction(this);
    m_exportAction = new QAction(this);
    m_refreshAction = new QAction(this);
    m_startScanAction = new QAction(this);
    m_cancelScanAction = new QAction(this);
    m_manageColumnsAction = new QAction(this);
    m_addRowAction = new QAction(this);
    m_addRowsAction = new QAction(this);
    m_confirmProcessAction = new QAction(this);
    m_reselectProcessAction = new QAction(this);
    m_clearResultsAction = new QAction(this);
    m_quitAction = new QAction(this);

    // 视图导航动作（仅暴露给 ViewModel，由 VM connect）
    m_showSignatureAction = new QAction(this);
    m_showProcessAction = new QAction(this);
    m_showResultAction = new QAction(this);
    m_toggleLogAction = new QAction(this);   // C-08：日志开关
    m_toggleLogAction->setCheckable(true);
    m_clearLogAction = new QAction(this);    // C-08：仅在日志右键菜单使用
    m_aboutAction = new QAction(this);

    // 语言动作 + 互斥组
    m_langGroup = new QActionGroup(this);
    m_langZhAction = new QAction(this);
    m_langEnAction = new QAction(this);
    m_langSystemAction = new QAction(this);
    for (QAction* a : {m_langZhAction, m_langEnAction, m_langSystemAction}) {
        a->setCheckable(true);
        m_langGroup->addAction(a);
    }
}

void MainWindow::applyActionIconsAndShortcuts() {
    // 图标（用标准图标，替代纯文字；tooltip 保留文字）
    QStyle* st = style();
    m_importAction->setIcon(st->standardIcon(QStyle::SP_DialogOpenButton));
    m_exportAction->setIcon(st->standardIcon(QStyle::SP_DialogSaveButton));
    m_refreshAction->setIcon(st->standardIcon(QStyle::SP_BrowserReload));
    m_startScanAction->setIcon(st->standardIcon(QStyle::SP_MediaPlay));
    m_cancelScanAction->setIcon(st->standardIcon(QStyle::SP_MediaStop));
    m_manageColumnsAction->setIcon(st->standardIcon(QStyle::SP_FileDialogDetailedView));
    m_addRowAction->setIcon(st->standardIcon(QStyle::SP_FileDialogNewFolder));
    m_addRowsAction->setIcon(st->standardIcon(QStyle::SP_FileDialogNewFolder));
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
    m_showSignatureAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+1")));
    m_showProcessAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+2")));
    m_showResultAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+3")));
    m_clearLogAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+L")));
    m_aboutAction->setShortcut(QKeySequence(Qt::Key_F1));
}

void MainWindow::buildMenus() {
    // 菜单按显示顺序创建：文件 / 编辑 / 扫描 / 视图 / 语言 / 帮助
    m_fileMenu = menuBar()->addMenu(QString());
    m_fileMenu->addAction(m_importAction);
    m_fileMenu->addAction(m_exportAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_quitAction);

    m_editMenu = menuBar()->addMenu(QString());
    m_editMenu->addAction(m_addRowAction);
    m_editMenu->addAction(m_addRowsAction);
    m_editMenu->addAction(m_manageColumnsAction);

    m_scanMenu = menuBar()->addMenu(QString());
    m_scanMenu->addAction(m_refreshAction);
    m_scanMenu->addAction(m_confirmProcessAction);
    m_scanMenu->addSeparator();
    m_scanMenu->addAction(m_startScanAction);
    m_scanMenu->addAction(m_cancelScanAction);
    m_scanMenu->addAction(m_reselectProcessAction);

    m_viewMenu = menuBar()->addMenu(QString());
    m_viewMenu->addAction(m_showProcessAction);
    m_viewMenu->addAction(m_showSignatureAction);
    m_viewMenu->addAction(m_showResultAction);
    m_viewMenu->addSeparator();
    // C-08：日志开关移入视图菜单；「清空日志」从视图菜单移除，改由日志窗口右键菜单触发
    m_viewMenu->addAction(m_toggleLogAction);

    // 「语言」菜单：位于「视图」与「帮助」之间，三项互斥
    m_langMenu = menuBar()->addMenu(QString());
    m_langMenu->addAction(m_langZhAction);
    m_langMenu->addAction(m_langEnAction);
    m_langMenu->addSeparator();
    m_langMenu->addAction(m_langSystemAction);

    m_helpMenu = menuBar()->addMenu(QString());
    m_helpMenu->addAction(m_aboutAction);
}

void MainWindow::buildToolbar() {
    // 主工具栏：图标化；按页显隐
    m_toolbar = addToolBar(QString());
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
}

void MainWindow::buildStatusAndLog() {
    // 历史日志：独立 Dock（不写入状态栏）
    m_logDock = new QDockWidget(this);
    m_logEdit = new QPlainTextEdit(m_logDock);
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumBlockCount(2000);
    // C-08：日志窗口右键菜单（含「清空日志」）；由 ViewModel 连接动作
    m_logEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    m_logContextMenu = new QMenu(m_logEdit);
    m_logContextMenu->addAction(m_clearLogAction);
    m_logDock->setWidget(m_logEdit);
    addDockWidget(Qt::BottomDockWidgetArea, m_logDock);

    // 状态栏：左=持久状态区，右=瞬时反馈区；进度条置于左区
    m_persistentLabel = new QLabel(this);
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
    m_addRowAction->setVisible(sigPage);
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

void MainWindow::appendLog(const QString& text) {
    // 按语言无关的 ASCII 标签着色（标签由 VM 追加，不随语言变化）
    QString color = QStringLiteral("#333333");
    if (text.startsWith(QStringLiteral("[error]")))
        color = QStringLiteral("#c62828");   // 错误：红
    else if (text.startsWith(QStringLiteral("[export]")) || text.startsWith(QStringLiteral("[import]")))
        color = QStringLiteral("#2e7d32");   // 成功：绿
    else if (text.startsWith(QStringLiteral("[startup]")) || text.startsWith(QStringLiteral("[settings]")))
        color = QStringLiteral("#455a64");   // 设置/启动：灰蓝
    const QString html = QStringLiteral("<span style=\"color:%1;\">%2</span>")
                             .arg(color, text.toHtmlEscaped());
    m_logEdit->appendHtml(html);
}

void MainWindow::clearLog() { m_logEdit->clear(); }

bool MainWindow::logDockVisible() const { return m_logDock && m_logDock->isVisible(); }

void MainWindow::setLogDockVisible(bool visible) {
    if (m_logDock) m_logDock->setVisible(visible);
}

void MainWindow::setToggleLogChecked(bool checked) {
    if (m_toggleLogAction) m_toggleLogAction->setChecked(checked);
}

void MainWindow::setLanguageChecked(const QString& code) {
    if (code == QStringLiteral("zh_CN")) m_langZhAction->setChecked(true);
    else if (code == QStringLiteral("en_US")) m_langEnAction->setChecked(true);
    else m_langSystemAction->setChecked(true);
}

void MainWindow::retranslateUi() {
    setWindowTitle(tr("进程特征码批量匹配器 v0.2.3"));

    m_fileMenu->setTitle(tr("文件(&F)"));
    m_editMenu->setTitle(tr("编辑(&E)"));
    m_scanMenu->setTitle(tr("扫描(&S)"));
    m_viewMenu->setTitle(tr("视图(&V)"));
    m_langMenu->setTitle(tr("语言(&L)"));
    m_helpMenu->setTitle(tr("帮助(&H)"));

    m_importAction->setText(tr("导入特征码"));
    m_exportAction->setText(tr("导出特征码"));
    m_refreshAction->setText(tr("刷新进程"));
    m_startScanAction->setText(tr("开始扫描"));
    m_cancelScanAction->setText(tr("取消扫描"));
    m_manageColumnsAction->setText(tr("列管理"));
    m_addRowAction->setText(tr("新增一行"));
    m_addRowsAction->setText(tr("新增多行…"));
    m_confirmProcessAction->setText(tr("确认选择"));
    m_reselectProcessAction->setText(tr("重新选择进程"));
    m_clearResultsAction->setText(tr("清空结果"));
    m_quitAction->setText(tr("退出"));
    m_showSignatureAction->setText(tr("特征码编辑"));
    m_showProcessAction->setText(tr("进程选择"));
    m_showResultAction->setText(tr("匹配结果"));
    m_toggleLogAction->setText(tr("日志")); // C-08：视图菜单开关
    m_clearLogAction->setText(tr("清空日志"));
    m_aboutAction->setText(tr("关于"));
    m_langZhAction->setText(tr("中文"));
    m_langEnAction->setText(tr("English"));
    m_langSystemAction->setText(tr("跟随系统"));

    m_toolbar->setWindowTitle(tr("主工具栏"));
    m_logDock->setWindowTitle(tr("历史日志"));

    m_signaturePage->retranslateUi();
    m_processPage->retranslateUi();
    m_resultPage->retranslateUi();
}
