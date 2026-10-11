// view/resultpage.cpp - 匹配结果页实现（仅 UI）
#include "view/resultpage.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QHeaderView>
#include <QMenu>
#include <QPlainTextEdit>
#include <QTableView>
#include <QVBoxLayout>

ResultPage::ResultPage(QWidget* parent) : QWidget(parent) {
    m_templateEdit = new QPlainTextEdit(this);
    m_templateEdit->setPlaceholderText(tr("模板文本（随导入/导出携带）"));
    m_templateEdit->setMaximumHeight(90);

    // 导出收敛到右键菜单的「导出」二级子菜单
    m_exportCsvAction = new QAction(tr("导出 CSV"), this);
    m_exportDbAction = new QAction(tr("导出 DB"), this);
    m_exportTxtAction = new QAction(tr("导出 TXT"), this);
    m_selectColumnsAction = new QAction(tr("选择导出列…"), this);
    m_deleteRowsAction = new QAction(tr("删除选中行"), this);
    m_fillCustomAction = new QAction(tr("批量填充自定义列…"), this);
    m_clearAction = new QAction(tr("清空结果"), this);

    m_contextMenu = new QMenu(this);
    QMenu* exportMenu = m_contextMenu->addMenu(tr("导出"));
    exportMenu->addAction(m_exportCsvAction);
    exportMenu->addAction(m_exportDbAction);
    exportMenu->addAction(m_exportTxtAction);
    m_contextMenu->addAction(m_selectColumnsAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_deleteRowsAction);
    m_contextMenu->addAction(m_fillCustomAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_clearAction);

    m_table = new QTableView(this);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setAlternatingRowColors(true);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    // 末列（自定义）可编辑；其余列由模型 flags 置为只读

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_templateEdit);
    layout->addWidget(m_table);
}

void ResultPage::setModel(QAbstractItemModel* model) { m_table->setModel(model); }

void ResultPage::setTemplateText(const QString& text) { m_templateEdit->setPlainText(text); }

QString ResultPage::templateText() const { return m_templateEdit->toPlainText(); }
