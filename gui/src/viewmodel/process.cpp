// viewmodel/process.cpp - ViewModel：进程枚举/筛选/选中/重选/图标
#include "viewmodel/mainviewmodel.h"

#include <QFileDialog>
#include <QImage>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QTableView>
#include <QtConcurrent>

#include "view/mainwindow.h"
#include "view/processselectionpage.h"
#include "viewmodel/viewmodel_helpers.h"

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
            QImage img = vmhelp::toQImage(bmp);
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
