// view/processselectionpage.cpp - 进程选择页实现（仅 UI）
#include "view/processselectionpage.h"

#include <QAbstractItemModel>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

ProcessSelectionPage::ProcessSelectionPage(QWidget* parent) : QWidget(parent) {
    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText(tr("模糊过滤进程名 / 路径…"));
    m_refreshBtn = new QPushButton(tr("刷新进程"), this);
    m_selectAllBtn = new QPushButton(tr("全选"), this);

    auto* toolbar = new QHBoxLayout();
    toolbar->addWidget(m_filterEdit, 1);
    toolbar->addWidget(m_refreshBtn);
    toolbar->addWidget(m_selectAllBtn);

    m_table = new QTableView(this);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setAlternatingRowColors(true);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(toolbar);
    layout->addWidget(m_table);
}

void ProcessSelectionPage::setModel(QAbstractItemModel* model) { m_table->setModel(model); }
