// view/mainwindow.h - 主窗口骨架（只画 UI）
#pragma once

#include <QMainWindow>

class QAction;
class QLabel;
class QProgressBar;
class QPlainTextEdit;
class QStackedWidget;
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

    // 视图导航动作 getter（供 ViewModel connect）
    QAction* showSignaturePageAction() const { return m_showSignatureAction; }
    QAction* showProcessPageAction() const { return m_showProcessAction; }
    QAction* showResultPageAction() const { return m_showResultAction; }
    QAction* clearLogAction() const { return m_clearLogAction; }
    QAction* aboutAction() const { return m_aboutAction; }

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
    void appendLog(const QString& text);
    void clearLog();

private:
    void buildMenuAndToolbar();
    void buildCentral();
    void buildStatusAndLog();

    QStackedWidget* m_stack = nullptr;
    SignatureEditorPage* m_signaturePage = nullptr;
    ProcessSelectionPage* m_processPage = nullptr;
    ResultPage* m_resultPage = nullptr;

    QAction* m_importAction = nullptr;
    QAction* m_exportAction = nullptr;
    QAction* m_refreshAction = nullptr;
    QAction* m_startScanAction = nullptr;
    QAction* m_cancelScanAction = nullptr;
    QAction* m_manageColumnsAction = nullptr;
    QAction* m_showSignatureAction = nullptr;
    QAction* m_showProcessAction = nullptr;
    QAction* m_showResultAction = nullptr;
    QAction* m_clearLogAction = nullptr;
    QAction* m_aboutAction = nullptr;

    QLabel* m_persistentLabel = nullptr;
    QLabel* m_transientLabel = nullptr;
    QProgressBar* m_progressBar = nullptr;
    QPlainTextEdit* m_logEdit = nullptr;
};
