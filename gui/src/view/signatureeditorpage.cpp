// view/signatureeditorpage.cpp - 特征码编辑页实现（仅 UI）
#include "view/signatureeditorpage.h"

#include <QAbstractItemModel>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

SignatureEditorPage::SignatureEditorPage(QWidget* parent) : QWidget(parent) {
    m_addRowBtn = new QPushButton(tr("新增行"), this);
    m_deleteRowBtn = new QPushButton(tr("删除行"), this);
    m_manageColumnsBtn = new QPushButton(tr("列管理"), this);

    auto* toolbar = new QHBoxLayout();
    toolbar->addWidget(m_addRowBtn);
    toolbar->addWidget(m_deleteRowBtn);
    toolbar->addWidget(m_manageColumnsBtn);
    toolbar->addStretch();

    m_table = new QTableView(this);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setAlternatingRowColors(true);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(toolbar);
    layout->addWidget(m_table);
}

void SignatureEditorPage::setModel(QAbstractItemModel* model) { m_table->setModel(model); }

void SignatureEditorPage::selectRow(int row) {
    if (!m_table->model() || row < 0 || row >= m_table->model()->rowCount()) return;
    m_table->selectRow(row);
    m_table->scrollTo(m_table->model()->index(row, 0));
}
