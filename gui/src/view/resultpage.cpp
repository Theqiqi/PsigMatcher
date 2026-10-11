// view/resultpage.cpp - 匹配结果页实现（仅 UI）
#include "view/resultpage.h"

#include <QAbstractItemModel>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

ResultPage::ResultPage(QWidget* parent) : QWidget(parent) {
    m_exportCsvBtn = new QPushButton(tr("导出CSV"), this);
    m_exportDbBtn = new QPushButton(tr("导出DB"), this);
    m_exportTxtBtn = new QPushButton(tr("导出TXT"), this);
    m_clearBtn = new QPushButton(tr("清空"), this);

    auto* toolbar = new QHBoxLayout();
    toolbar->addWidget(m_exportCsvBtn);
    toolbar->addWidget(m_exportDbBtn);
    toolbar->addWidget(m_exportTxtBtn);
    toolbar->addStretch();
    toolbar->addWidget(m_clearBtn);

    m_table = new QTableView(this);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers); // 只读集合
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setAlternatingRowColors(true);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(toolbar);
    layout->addWidget(m_table);
}

void ResultPage::setModel(QAbstractItemModel* model) { m_table->setModel(model); }
