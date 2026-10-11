// model/resulttablemodel.cpp - 匹配结果模型实现（只读，含显示格式化）
#include "model/resulttablemodel.h"

#include <QString>

namespace {
QString hexAddr(quint64 v) { return QStringLiteral("0x") + QString::number(v, 16).toUpper(); }
} // namespace

ResultTableModel::ResultTableModel(QObject* parent) : QAbstractTableModel(parent) {}

int ResultTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_rows.size());
}

int ResultTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return 8; // 特征码名/进程/PID/地址/模块/模块基址/匹配字节/命中时间
}

QVariant ResultTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_rows.size())) return {};
    if (role != Qt::DisplayRole) return {};
    const ResultRow& r = m_rows[static_cast<size_t>(index.row())];
    switch (index.column()) {
    case 0: return r.patternName;
    case 1: return r.processName;
    case 2: return r.pid;
    case 3: return hexAddr(r.address);
    case 4: return r.moduleName;
    case 5: return r.moduleBase ? hexAddr(r.moduleBase) : QString();
    case 6: return r.matchedBytes;
    case 7: return r.hitTime;
    default: return {};
    }
}

QVariant ResultTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) return {};
    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return QStringLiteral("特征码名");
        case 1: return QStringLiteral("进程");
        case 2: return QStringLiteral("PID");
        case 3: return QStringLiteral("地址");
        case 4: return QStringLiteral("模块");
        case 5: return QStringLiteral("模块基址");
        case 6: return QStringLiteral("匹配字节");
        case 7: return QStringLiteral("命中时间");
        default: return {};
        }
    }
    return section + 1;
}

Qt::ItemFlags ResultTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void ResultTableModel::setResults(const std::vector<ResultRow>& list) {
    beginResetModel();
    m_rows = list;
    endResetModel();
}

std::vector<ResultRow> ResultTableModel::results() const { return m_rows; }

void ResultTableModel::appendResult(const ResultRow& r) {
    const int row = static_cast<int>(m_rows.size());
    beginInsertRows(QModelIndex(), row, row);
    m_rows.push_back(r);
    endInsertRows();
}

void ResultTableModel::clear() {
    beginResetModel();
    m_rows.clear();
    endResetModel();
}
