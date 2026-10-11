// view/processselectionpage.cpp - 进程选择页实现（仅 UI）
#include "view/processselectionpage.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QSize>
#include <QTableView>
#include <QVBoxLayout>

ProcessSelectionPage::ProcessSelectionPage(QWidget* parent) : QWidget(parent) {
    m_filterEdit = new QLineEdit(this);

    m_useAsTargetAction = new QAction(this);
    m_refreshAction = new QAction(this);
    m_selectAllAction = new QAction(this);
    m_contextMenu = new QMenu(this);
    m_contextMenu->addAction(m_useAsTargetAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_refreshAction);
    m_contextMenu->addAction(m_selectAllAction);

    m_table = new QTableView(this);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setAlternatingRowColors(true);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    m_table->setSortingEnabled(true); // 点击列头排序
    m_table->horizontalHeader()->setSectionsClickable(true);
    m_table->setIconSize(QSize(16, 16));

    // 筛选框置于列表底部
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_table, 1);
    layout->addWidget(m_filterEdit);

    retranslateUi();
}

void ProcessSelectionPage::setModel(QAbstractItemModel* model) { m_table->setModel(model); }

void ProcessSelectionPage::retranslateUi() {
    m_filterEdit->setPlaceholderText(tr("模糊过滤进程名 / 路径…"));
    m_useAsTargetAction->setText(tr("以此进程为目标"));
    m_refreshAction->setText(tr("刷新进程"));
    m_selectAllAction->setText(tr("全选"));
}
