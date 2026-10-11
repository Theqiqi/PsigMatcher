// view/columnmanagerdialog.cpp - 列管理对话框实现（仅 UI）
#include "view/columnmanagerdialog.h"

#include <QDialogButtonBox>
#include <QComboBox>
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
    // 统一按钮文案（避免随 Qt 内建翻译出现中英混排）
    m_buttonBox->button(QDialogButtonBox::Ok)->setText(tr("确定"));
    m_buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("取消"));

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(body);
    layout->addWidget(m_buttonBox);
}

// 构建一行：第 0 列=标题（可编辑文本），第 1 列=键（下拉），第 2 列=类型（下拉）
void ColumnManagerDialog::buildRow(int r, const QString& title, const QString& key, const QString& type) {
    m_table->setItem(r, 0, new QTableWidgetItem(title));

    // 键下拉：内置键选项；若当前键不在选项中（自定义列），追加之以免丢失
    auto* keyCombo = new QComboBox(this);
    for (const QString& k : m_keyOptions) keyCombo->addItem(k);
    if (keyCombo->findText(key) < 0) keyCombo->addItem(key);
    keyCombo->setCurrentText(key);
    m_table->setCellWidget(r, 1, keyCombo);

    // 类型下拉：文本 / 数字
    auto* typeCombo = new QComboBox(this);
    typeCombo->addItem(tr("文本"), QStringLiteral("text"));
    typeCombo->addItem(tr("数字"), QStringLiteral("number"));
    int ti = typeCombo->findData(type);
    if (ti < 0) ti = typeCombo->findData(QStringLiteral("text"));
    typeCombo->setCurrentIndex(ti < 0 ? 0 : ti);
    m_table->setCellWidget(r, 2, typeCombo);
}

void ColumnManagerDialog::setColumns(const std::vector<ColumnDef>& cols) {
    m_table->setRowCount(0);
    for (const auto& c : cols) {
        const int r = m_table->rowCount();
        m_table->insertRow(r);
        buildRow(r, c.title, c.key, c.type);
    }
}

std::vector<ColumnDef> ColumnManagerDialog::columns() const {
    std::vector<ColumnDef> out;
    for (int r = 0; r < m_table->rowCount(); ++r) {
        ColumnDef c;
        auto* t = m_table->item(r, 0);
        auto* kc = qobject_cast<QComboBox*>(m_table->cellWidget(r, 1));
        auto* tc = qobject_cast<QComboBox*>(m_table->cellWidget(r, 2));
        c.title = t ? t->text() : QString();
        c.key = kc ? kc->currentText() : QString();
        c.type = tc ? tc->currentData().toString() : QStringLiteral("text");
        c.builtin = false;
        out.push_back(c);
    }
    return out;
}

void ColumnManagerDialog::addColumnRow() {
    const int r = m_table->rowCount();
    m_table->insertRow(r);
    // 新增列：键取首个内置键（避免自由输入），类型默认 text
    const QString key = m_keyOptions.isEmpty() ? QStringLiteral("col%1").arg(r + 1) : m_keyOptions.first();
    buildRow(r, tr("新列"), key, QStringLiteral("text"));
    m_table->selectRow(r);
}

void ColumnManagerDialog::removeSelectedColumnRow() {
    const int r = m_table->currentRow();
    if (r >= 0) m_table->removeRow(r);
}

void ColumnManagerDialog::moveSelectedUp() {
    const int r = m_table->currentRow();
    if (r <= 0) return;
    SwapRows(r, r - 1);
    m_table->selectRow(r - 1);
}

void ColumnManagerDialog::moveSelectedDown() {
    const int r = m_table->currentRow();
    if (r < 0 || r >= m_table->rowCount() - 1) return;
    SwapRows(r, r + 1);
    m_table->selectRow(r + 1);
}

void ColumnManagerDialog::SwapRows(int a, int b) {
    // 标题（文本项）：直接换项
    QTableWidgetItem* ia = m_table->takeItem(a, 0);
    QTableWidgetItem* ib = m_table->takeItem(b, 0);
    m_table->setItem(a, 0, ib);
    m_table->setItem(b, 0, ia);
    // 键 / 类型（下拉）：换选中值而非换控件（QTableWidget 替换 cellWidget 会删除旧控件）
    for (int col = 1; col < m_table->columnCount(); ++col) {
        auto* ca = qobject_cast<QComboBox*>(m_table->cellWidget(a, col));
        auto* cb = qobject_cast<QComboBox*>(m_table->cellWidget(b, col));
        if (!ca || !cb) continue;
        const QString ta = ca->currentText();
        const QString tb = cb->currentText();
        const QVariant da = ca->currentData();
        const QVariant db = cb->currentData();
        if (ca->findText(tb) < 0) ca->addItem(tb, db); // 自定义键可能只在一侧
        if (cb->findText(ta) < 0) cb->addItem(ta, da);
        ca->setCurrentText(tb);
        cb->setCurrentText(ta);
    }
}
