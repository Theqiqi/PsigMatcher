// model/processtablemodel.cpp - 进程列表模型实现（只读）
#include "model/processtablemodel.h"

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
    if (role != Qt::DisplayRole) return {};
    const ProcessRow& r = m_rows[static_cast<size_t>(index.row())];
    switch (index.column()) {
    case 0: return r.pid;
    case 1: return r.name;
    case 2: return r.path;
    case 3: return r.accessible ? QStringLiteral("是") : QStringLiteral("否");
    default: return {};
    }
}

QVariant ProcessTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) return {};
    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return QStringLiteral("PID");
        case 1: return QStringLiteral("名称");
        case 2: return QStringLiteral("路径");
        case 3: return QStringLiteral("可访问");
        default: return {};
        }
    }
    return section + 1;
}

Qt::ItemFlags ProcessTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void ProcessTableModel::setProcesses(const std::vector<ProcessRow>& list) {
    beginResetModel();
    m_rows = list;
    endResetModel();
}

std::vector<ProcessRow> ProcessTableModel::processes() const { return m_rows; }

ProcessRow ProcessTableModel::processAt(int row) const {
    if (row < 0 || row >= static_cast<int>(m_rows.size())) return {};
    return m_rows[static_cast<size_t>(row)];
}
