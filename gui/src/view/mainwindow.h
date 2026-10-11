// view/mainwindow.h - 主窗口骨架（只画 UI）
#pragma once

#include <QMainWindow>

class QAction;
class QActionGroup;
class QDockWidget;
class QLabel;
class QMenu;
class QProgressBar;
class QPlainTextEdit;
class QStackedWidget;
class QToolBar;
class SignatureEditorPage;
class ProcessSelectionPage;
class ResultPage;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

    // 页面 getter
    SignatureEditorPage* signaturePage() const { return m_signaturePage; }
    ProcessSelectionPage* processPage() const { return m_processPage; }
    ResultPage* resultPage() const { return m_resultPage; }

    // 全局动作 getter
    QAction* importSignaturesAction() const { return m_importAction; }
    QAction* exportSignaturesAction() const { return m_exportAction; }
    QAction* refreshProcessAction() const { return m_refreshAction; }
    QAction* startScanAction() const { return m_startScanAction; }
    QAction* cancelScanAction() const { return m_cancelScanAction; }
    QAction* manageColumnsAction() const { return m_manageColumnsAction; }
    QAction* addSignatureRowAction() const { return m_addRowAction; }
    QAction* addSignatureRowsAction() const { return m_addRowsAction; }
    QAction* confirmProcessAction() const { return m_confirmProcessAction; }
    QAction* reselectProcessAction() const { return m_reselectProcessAction; }
    QAction* clearResultsAction() const { return m_clearResultsAction; }

    // 视图导航动作 getter（供 ViewModel connect）
    QAction* showSignaturePageAction() const { return m_showSignatureAction; }
    QAction* showProcessPageAction() const { return m_showProcessAction; }
    QAction* showResultPageAction() const { return m_showResultAction; }
    QAction* clearLogAction() const { return m_clearLogAction; }
    QAction* aboutAction() const { return m_aboutAction; }

    // 语言动作 getter（供 ViewModel connect）
    QAction* languageZhAction() const { return m_langZhAction; }
    QAction* languageEnAction() const { return m_langEnAction; }
    QAction* languageSystemAction() const { return m_langSystemAction; }
    QActionGroup* languageGroup() const { return m_langGroup; }

    // 状态/日志控件 getter
    QLabel* persistentLabel() const { return m_persistentLabel; }
    QLabel* transientLabel() const { return m_transientLabel; }
    QProgressBar* progressBar() const { return m_progressBar; }
    QPlainTextEdit* logEdit() const { return m_logEdit; }

    // 公开 UI 方法
    void showSignaturePage();
    void showProcessPage();
    void showResultPage();
    void setPersistentStatus(const QString& text);
    void setTransientMessage(const QString& text, bool error = false);
    void setProgress(int value); // 0..100；负值隐藏
    void appendLog(const QString& text);   // 内部按级别着色
    void clearLog();
    // 进程选定后锁定：隐藏进程页入口/进程动作，显示「重新选择进程」
    void setProcessSelectionLocked(bool locked);
    // 语言切换后刷新全部界面文案
    void retranslateUi();
    // 勾选当前语言（code: "zh_CN" / "en_US" / ""（跟随系统））
    void setLanguageChecked(const QString& code);

private:
    void buildMenuAndToolbar();
    void buildCentral();
    void buildStatusAndLog();
    void applyToolbarForPage(int index); // 0=特征码 1=进程 2=结果

    QStackedWidget* m_stack = nullptr;
    SignatureEditorPage* m_signaturePage = nullptr;
    ProcessSelectionPage* m_processPage = nullptr;
    ResultPage* m_resultPage = nullptr;
    QToolBar* m_toolbar = nullptr;
    QDockWidget* m_logDock = nullptr;

    // 菜单（供 retranslateUi 重设标题）
    QMenu* m_fileMenu = nullptr;
    QMenu* m_editMenu = nullptr;
    QMenu* m_scanMenu = nullptr;
    QMenu* m_viewMenu = nullptr;
    QMenu* m_langMenu = nullptr;
    QMenu* m_helpMenu = nullptr;
    QAction* m_quitAction = nullptr;

    QAction* m_importAction = nullptr;
    QAction* m_exportAction = nullptr;
    QAction* m_refreshAction = nullptr;
    QAction* m_startScanAction = nullptr;
    QAction* m_cancelScanAction = nullptr;
    QAction* m_manageColumnsAction = nullptr;
    QAction* m_addRowAction = nullptr;
    QAction* m_addRowsAction = nullptr;
    QAction* m_confirmProcessAction = nullptr;
    QAction* m_reselectProcessAction = nullptr;
    QAction* m_clearResultsAction = nullptr;
    QAction* m_showSignatureAction = nullptr;
    QAction* m_showProcessAction = nullptr;
    QAction* m_showResultAction = nullptr;
    QAction* m_clearLogAction = nullptr;
    QAction* m_aboutAction = nullptr;

    QActionGroup* m_langGroup = nullptr;
    QAction* m_langZhAction = nullptr;
    QAction* m_langEnAction = nullptr;
    QAction* m_langSystemAction = nullptr;

    QLabel* m_persistentLabel = nullptr;
    QLabel* m_transientLabel = nullptr;
    QProgressBar* m_progressBar = nullptr;
    QPlainTextEdit* m_logEdit = nullptr;

    bool m_processSelectionLocked = false;
};
