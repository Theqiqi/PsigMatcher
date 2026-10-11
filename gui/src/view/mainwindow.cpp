// view/mainwindow.cpp - 主窗口骨架实现（仅 UI）
#include "view/mainwindow.h"

#include <QAction>
#include <QApplication>
#include <QDockWidget>
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
    setWindowTitle(tr("进程特征码批量匹配器 v0.1.0"));
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

    QMenu* fileMenu = menuBar()->addMenu(tr("文件(&F)"));
    fileMenu->addAction(m_importAction);
    fileMenu->addAction(m_exportAction);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("退出"), this, &QWidget::close);

    QMenu* editMenu = menuBar()->addMenu(tr("编辑(&E)"));
    editMenu->addAction(m_manageColumnsAction);

    QMenu* scanMenu = menuBar()->addMenu(tr("扫描(&S)"));
    scanMenu->addAction(m_refreshAction);
    scanMenu->addSeparator();
    scanMenu->addAction(m_startScanAction);
    scanMenu->addAction(m_cancelScanAction);

    QMenu* helpMenu = menuBar()->addMenu(tr("帮助(&H)"));
    m_aboutAction = new QAction(tr("关于"), this);
    helpMenu->addAction(m_aboutAction);

    QToolBar* tb = addToolBar(tr("主工具栏"));
    tb->setMovable(false);
    tb->addAction(m_importAction);
    tb->addAction(m_exportAction);
    tb->addSeparator();
    tb->addAction(m_refreshAction);
    tb->addAction(m_startScanAction);
    tb->addAction(m_cancelScanAction);
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
    QMenu* viewMenu = menuBar()->addMenu(tr("视图(&V)"));
    viewMenu->addAction(m_showSignatureAction);
    viewMenu->addAction(m_showProcessAction);
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
    m_persistentLabel = new QLabel(tr("就绪 | 进程 - | 特征码 -"), this);
    m_transientLabel = new QLabel(QString(), this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setFixedWidth(160);
    m_progressBar->setVisible(false);

    statusBar()->addWidget(m_persistentLabel);
    statusBar()->addWidget(m_progressBar);
    statusBar()->addPermanentWidget(m_transientLabel);
}

void MainWindow::showSignaturePage() { m_stack->setCurrentWidget(m_signaturePage); }
void MainWindow::showProcessPage() { m_stack->setCurrentWidget(m_processPage); }
void MainWindow::showResultPage() { m_stack->setCurrentWidget(m_resultPage); }

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
