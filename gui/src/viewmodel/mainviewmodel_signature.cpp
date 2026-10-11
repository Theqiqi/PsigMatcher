// viewmodel/mainviewmodel_signature.cpp - ViewModel：特征码表增删/列管理/导入导出
#include "viewmodel/mainviewmodel.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableView>

#include "view/columnmanagerdialog.h"
#include "view/mainwindow.h"
#include "view/resultpage.h"
#include "view/signatureeditorpage.h"
#include "viewmodel/viewmodel_helpers.h"

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
    vmhelp::wireDialog(&dlg, dlg.buttonBox());
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

psig::Table MainViewModel::tableFromModel() const {
    psig::Table t;
    // C-06：内置列标题随语言（经 tr()）；自定义列沿用其标题（不随语言）
    for (const auto& c : m_sigModel->columns()) {
        psig::ColumnDef p;
        p.key = c.key.toStdString();
        p.type = c.type.toStdString();
        p.builtin = c.builtin;
        const QString h = c.builtin ? vmhelp::signatureHeaderForKey(c.key) : QString();
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
    for (const auto& c : table.columns) cols.push_back(vmhelp::toModelColumn(c));
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
