// model/signaturetablemodel.cpp - 特征码表模型实现（纯数据）
#include "model/signaturetablemodel.h"

#include <algorithm>
#include <functional>

SignatureTableModel::SignatureTableModel(QObject* parent) : QAbstractTableModel(parent) {}

int SignatureTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_rows.size());
}

int SignatureTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_columns.size());
}

QVariant SignatureTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_rows.size())) return {};
    if (index.column() >= static_cast<int>(m_columns.size())) return {};
    const QStringList& cells = m_rows[static_cast<size_t>(index.row())].cells;
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (index.column() < cells.size()) return cells[index.column()];
        return QString();
    }
    return {};
}

bool SignatureTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (role != Qt::EditRole || !index.isValid()) return false;
    if (index.row() >= static_cast<int>(m_rows.size())) return false;
    if (index.column() >= static_cast<int>(m_columns.size())) return false;
    SignatureRow& row = m_rows[static_cast<size_t>(index.row())];
    while (row.cells.size() <= index.column()) row.cells << QString();
    row.cells[index.column()] = value.toString();
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    return true;
}

QVariant SignatureTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) return {};
    if (orientation == Qt::Horizontal) {
        if (section < 0 || section >= static_cast<int>(m_columns.size())) return {};
        // 内置列标题经 tr() 随语言切换；自定义列沿用其标题
        const QString& key = m_columns[static_cast<size_t>(section)].key;
        if (key == QStringLiteral("name")) return tr("名称");
        if (key == QStringLiteral("pattern")) return tr("特征码");
        if (key == QStringLiteral("note")) return tr("备注");
        if (key == QStringLiteral("process")) return tr("进程");
        return m_columns[static_cast<size_t>(section)].title;
    }
    return section + 1;
}

Qt::ItemFlags SignatureTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void SignatureTableModel::setColumns(const std::vector<ColumnDef>& columns) {
    beginResetModel();
    m_columns = columns;
    for (auto& row : m_rows) {
        while (row.cells.size() < static_cast<int>(m_columns.size())) row.cells << QString();
        while (row.cells.size() > static_cast<int>(m_columns.size())) row.cells.removeLast();
    }
    endResetModel();
}

std::vector<ColumnDef> SignatureTableModel::columns() const { return m_columns; }

void SignatureTableModel::setRows(const std::vector<SignatureRow>& rows) {
    beginResetModel();
    m_rows = rows;
    for (auto& row : m_rows) {
        while (row.cells.size() < static_cast<int>(m_columns.size())) row.cells << QString();
    }
    endResetModel();
}

std::vector<SignatureRow> SignatureTableModel::rows() const { return m_rows; }

namespace {
// 构造一个空行；若存在键为 process 的列且 processDefault 非空，则填入该列
SignatureRow MakeEmptyRow(const std::vector<ColumnDef>& columns, const QString& processDefault) {
    SignatureRow row;
    for (size_t i = 0; i < columns.size(); ++i) row.cells << QString();
    if (!processDefault.isEmpty()) {
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].key == QStringLiteral("process")) {
                row.cells[static_cast<int>(i)] = processDefault;
                break;
            }
        }
    }
    return row;
}
} // namespace

void SignatureTableModel::addRow(const QString& processDefault) {
    const int r = static_cast<int>(m_rows.size());
    beginInsertRows(QModelIndex(), r, r);
    m_rows.push_back(MakeEmptyRow(m_columns, processDefault));
    endInsertRows();
}

void SignatureTableModel::addRows(int count, const QString& processDefault) {
    if (count <= 0) return;
    const int first = static_cast<int>(m_rows.size());
    const int last = first + count - 1;
    beginInsertRows(QModelIndex(), first, last);
    for (int i = 0; i < count; ++i) m_rows.push_back(MakeEmptyRow(m_columns, processDefault));
    endInsertRows();
}

void SignatureTableModel::removeRows(const std::vector<int>& rows) {
    if (rows.empty()) return;
    std::vector<int> sorted = rows;
    std::sort(sorted.begin(), sorted.end(), std::greater<int>());
    for (int r : sorted) {
        if (r < 0 || r >= static_cast<int>(m_rows.size())) continue;
        beginRemoveRows(QModelIndex(), r, r);
        m_rows.erase(m_rows.begin() + r);
        endRemoveRows();
    }
}

int SignatureTableModel::columnIndex(const QString& key) const {
    for (size_t i = 0; i < m_columns.size(); ++i) {
        if (m_columns[i].key == key) return static_cast<int>(i);
    }
    return -1;
}

QString SignatureTableModel::valueAt(int row, const QString& key) const {
    if (row < 0 || row >= static_cast<int>(m_rows.size())) return {};
    int col = columnIndex(key);
    if (col < 0) return {};
    const QStringList& cells = m_rows[static_cast<size_t>(row)].cells;
    return (col < cells.size()) ? cells[col] : QString();
}

void SignatureTableModel::retranslate() {
    const int last = static_cast<int>(m_columns.size()) - 1;
    if (last >= 0) emit headerDataChanged(Qt::Horizontal, 0, last);
}
