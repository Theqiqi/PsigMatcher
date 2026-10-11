// model/processtablemodel.cpp - 进程列表模型实现（只读，含图标与排序）
#include "model/processtablemodel.h"

#include <algorithm>

ProcessTableModel::ProcessTableModel(QObject* parent) : QAbstractTableModel(parent) {}

int ProcessTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_rows.size());
}

int ProcessTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return 4; // PID / 名称 / 路径 / 可访问
}

QVariant ProcessTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_rows.size())) return {};
    const ProcessRow& r = m_rows[static_cast<size_t>(index.row())];
    if (role == Qt::DecorationRole && index.column() == 1) {
        return r.icon; // 名称列显示进程图标（无图标时为空 → 视图不显示）
    }
    if (role != Qt::DisplayRole) return {};
    switch (index.column()) {
    case 0: return r.pid;
    case 1: return r.name;
    case 2: return r.path;
    case 3: return r.accessible ? tr("是") : tr("否");
    default: return {};
    }
}

QVariant ProcessTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) return {};
    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return QStringLiteral("PID");
        case 1: return tr("名称");
        case 2: return tr("路径");
        case 3: return tr("可访问");
        default: return {};
        }
    }
    return section + 1;
}

Qt::ItemFlags ProcessTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void ProcessTableModel::sort(int column, Qt::SortOrder order) {
    if (m_rows.empty()) return;
    const bool asc = (order == Qt::AscendingOrder);
    auto less = [column, asc](const ProcessRow& a, const ProcessRow& b) {
        bool lt = false;
        switch (column) {
        case 0: lt = a.pid < b.pid; break;
        case 1: lt = a.name.compare(b.name, Qt::CaseInsensitive) < 0; break;
        case 2: lt = a.path.compare(b.path, Qt::CaseInsensitive) < 0; break;
        case 3: lt = a.accessible < b.accessible; break;
        default: lt = a.pid < b.pid; break;
        }
        return asc ? lt : !lt;
    };
    beginResetModel();
    std::stable_sort(m_rows.begin(), m_rows.end(), less);
    endResetModel();
}

void ProcessTableModel::setProcesses(const std::vector<ProcessRow>& list) {
    beginResetModel();
    m_rows = list;
    endResetModel();
}

void ProcessTableModel::setIcon(int row, const QIcon& icon) {
    if (row < 0 || row >= static_cast<int>(m_rows.size())) return;
    m_rows[static_cast<size_t>(row)].icon = icon;
    const QModelIndex idx = index(row, 1);
    emit dataChanged(idx, idx, {Qt::DecorationRole});
}

std::vector<ProcessRow> ProcessTableModel::processes() const { return m_rows; }

ProcessRow ProcessTableModel::processAt(int row) const {
    if (row < 0 || row >= static_cast<int>(m_rows.size())) return {};
    return m_rows[static_cast<size_t>(row)];
}

void ProcessTableModel::retranslate() {
    emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
}
