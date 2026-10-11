// view/exportcolumnsdialog.cpp - 选择导出列对话框实现（仅 UI）
#include "view/exportcolumnsdialog.h"

#include <QDialogButtonBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>

ExportColumnsDialog::ExportColumnsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("选择导出列"));
    setMinimumWidth(300);

    m_list = new QListWidget(this);
    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_list);
    layout->addWidget(m_buttons);
}

void ExportColumnsDialog::setColumns(const std::vector<std::pair<QString, QString>>& cols,
                                     const QStringList& selectedKeys) {
    m_list->clear();
    for (const auto& c : cols) {
        auto* item = new QListWidgetItem(c.second, m_list);
        item->setData(Qt::UserRole, c.first);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        const bool checked = selectedKeys.isEmpty() || selectedKeys.contains(c.first);
        item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }
}

QStringList ExportColumnsDialog::selectedKeys() const {
    QStringList keys;
    for (int i = 0; i < m_list->count(); ++i) {
        QListWidgetItem* item = m_list->item(i);
        if (item->checkState() == Qt::Checked) keys << item->data(Qt::UserRole).toString();
    }
    return keys;
}
