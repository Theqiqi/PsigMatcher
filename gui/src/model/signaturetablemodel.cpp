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
        if (section < static_cast<int>(m_columns.size())) return m_columns[static_cast<size_t>(section)].title;
        return {};
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

void SignatureTableModel::addRow() {
    const int r = static_cast<int>(m_rows.size());
    beginInsertRows(QModelIndex(), r, r);
    SignatureRow row;
    for (size_t i = 0; i < m_columns.size(); ++i) row.cells << QString();
    m_rows.push_back(row);
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
