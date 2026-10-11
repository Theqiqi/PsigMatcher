// view/signatureeditorpage.cpp - 特征码编辑页实现（仅 UI）
#include "view/signatureeditorpage.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QHeaderView>
#include <QMenu>
#include <QTableView>
#include <QVBoxLayout>

SignatureEditorPage::SignatureEditorPage(QWidget* parent) : QWidget(parent) {
    // 行操作统一收敛到表格右键菜单（不再使用按钮条）
    m_addRowAction = new QAction(tr("新增行"), this);
    m_deleteRowsAction = new QAction(tr("删除选中行"), this);
    m_manageColumnsAction = new QAction(tr("列管理"), this);
    m_contextMenu = new QMenu(this);
    m_contextMenu->addAction(m_addRowAction);
    m_contextMenu->addAction(m_deleteRowsAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_manageColumnsAction);

    m_table = new QTableView(this);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setAlternatingRowColors(true);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_table);
}

void SignatureEditorPage::setModel(QAbstractItemModel* model) { m_table->setModel(model); }

void SignatureEditorPage::selectRow(int row) {
    if (!m_table->model() || row < 0 || row >= m_table->model()->rowCount()) return;
    m_table->selectRow(row);
    m_table->scrollTo(m_table->model()->index(row, 0));
}
