// view/columnmanagerdialog.cpp - 列管理对话框实现（仅 UI）
#include "view/columnmanagerdialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

ColumnManagerDialog::ColumnManagerDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("列管理"));

    m_table = new QTableWidget(0, 3, this);
    m_table->setHorizontalHeaderLabels({tr("标题"), tr("键"), tr("类型")});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);

    m_addBtn = new QPushButton(tr("添加列"), this);
    m_removeBtn = new QPushButton(tr("删除列"), this);
    m_upBtn = new QPushButton(tr("上移"), this);
    m_downBtn = new QPushButton(tr("下移"), this);

    auto* side = new QVBoxLayout();
    side->addWidget(m_addBtn);
    side->addWidget(m_removeBtn);
    side->addStretch();
    side->addWidget(m_upBtn);
    side->addWidget(m_downBtn);

    auto* body = new QHBoxLayout();
    body->addWidget(m_table, 1);
    body->addLayout(side);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(body);
    layout->addWidget(m_buttonBox);
}

void ColumnManagerDialog::setColumns(const std::vector<ColumnDef>& cols) {
    m_table->setRowCount(0);
    for (const auto& c : cols) {
        int r = m_table->rowCount();
        m_table->insertRow(r);
        m_table->setItem(r, 0, new QTableWidgetItem(c.title));
        m_table->setItem(r, 1, new QTableWidgetItem(c.key));
        m_table->setItem(r, 2, new QTableWidgetItem(c.type));
    }
}

std::vector<ColumnDef> ColumnManagerDialog::columns() const {
    std::vector<ColumnDef> out;
    for (int r = 0; r < m_table->rowCount(); ++r) {
        ColumnDef c;
        auto* t = m_table->item(r, 0);
        auto* k = m_table->item(r, 1);
        auto* ty = m_table->item(r, 2);
        c.title = t ? t->text() : QString();
        c.key = k ? k->text() : QString();
        c.type = ty ? ty->text() : QStringLiteral("text");
        c.builtin = false;
        out.push_back(c);
    }
    return out;
}
