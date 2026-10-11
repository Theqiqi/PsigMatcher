// viewmodel/result.cpp - ViewModel：结果导出/列选择/填充/清空/关于
#include "viewmodel/mainviewmodel.h"

#include <QDialog>
#include <QFileDialog>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QTableView>

#include "view/aboutdialog.h"
#include "view/exportcolumnsdialog.h"
#include "view/mainwindow.h"
#include "view/resultpage.h"
#include "viewmodel/viewmodel_helpers.h"

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
    for (const auto& r : m_resultModel->results()) results.push_back(vmhelp::toPsigResult(r));
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
    dlg.setColumns(vmhelp::resultColumnDefs(), m_exportColumns);
    vmhelp::wireDialog(&dlg, dlg.buttonBox());
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
    vmhelp::wireDialog(&dlg, dlg.buttonBox());
    dlg.setInfo(tr("进程特征码批量匹配器 v0.2.3\n"
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
    for (const auto& k : keys) o.headers.push_back(vmhelp::resultHeaderForKey(k).toStdString());
    return o;
}
