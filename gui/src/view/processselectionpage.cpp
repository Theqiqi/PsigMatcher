// view/processselectionpage.cpp - 进程选择页实现（仅 UI）
#include "view/processselectionpage.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QTableView>
#include <QVBoxLayout>

ProcessSelectionPage::ProcessSelectionPage(QWidget* parent) : QWidget(parent) {
    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText(tr("模糊过滤进程名 / 路径…"));

    m_useAsTargetAction = new QAction(tr("以此进程为目标"), this);
    m_refreshAction = new QAction(tr("刷新进程"), this);
    m_selectAllAction = new QAction(tr("全选"), this);
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

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_filterEdit);
    layout->addWidget(m_table);
}

void ProcessSelectionPage::setModel(QAbstractItemModel* model) { m_table->setModel(model); }
