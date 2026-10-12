// viewmodel/scan.cpp - ViewModel：扫描启动/取消/完成与模式收集
#include "viewmodel/mainviewmodel.h"

#include <QtConcurrent>

#include "view/mainwindow.h"
#include "viewmodel/viewmodel_helpers.h"

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

    // C-03/C-04：重置进度原子变量并启动定时采样（界面刷新与扫描循环解耦）
    m_progDone.store(0);
    m_progTotal.store(0);
    m_progHits.store(0);
    m_progPid.store(0);
    if (m_progressTimer) m_progressTimer->start();

    // 工作线程回调只更新原子变量，不触碰任何界面对象（C-04）
    auto cb = [this](const psig::ScanProgress& sp) {
        m_progPid.store(sp.currentPid);
        m_progHits.store(sp.hits);
        m_progTotal.store(sp.total);
        m_progDone.store(sp.done);
    };
    std::atomic_bool* cancelPtr = &m_cancel;
    m_scanWatcher->setFuture(QtConcurrent::run([req, cb, cancelPtr]() {
        return psig::ScanProcesses(req, cb, cancelPtr);
    }));
}

// C-03：定时采样进度（主线程）——读取工作线程写入的原子变量并刷新状态栏
void MainViewModel::onProgressTick() {
    const size_t total = m_progTotal.load();
    const size_t done = m_progDone.load();
    const int pct = total ? static_cast<int>(done * 100 / total) : 0;
    const uint32_t pid = m_progPid.load();
    const size_t hits = m_progHits.load();
    m_view->setProgress(pct);
    m_view->setTransientMessage(tr("扫描 pid=%1 … 命中 %2").arg(pid).arg(hits));
}

void MainViewModel::onCancelScan() {
    m_cancel.store(true);
    psig::RequestCancel(true);
    m_view->setTransientMessage(tr("正在取消扫描…"));
}

void MainViewModel::onScanFinished() {
    if (m_progressTimer) m_progressTimer->stop();
    std::vector<ResultRow> rows;
    for (const auto& m : m_scanWatcher->result()) rows.push_back(vmhelp::toResultRow(m));
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
