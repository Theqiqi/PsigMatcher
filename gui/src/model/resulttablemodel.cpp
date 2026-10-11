// model/resulttablemodel.cpp - 匹配结果模型实现（末列可编辑，含显示格式化）
#include "model/resulttablemodel.h"

#include <QString>

#include <algorithm>
#include <functional>

namespace {
QString hexAddr(quint64 v) { return QStringLiteral("0x") + QString::number(v, 16).toUpper(); }
} // namespace

ResultTableModel::ResultTableModel(QObject* parent) : QAbstractTableModel(parent) {}

int ResultTableModel::customColumn() { return 9; }

int ResultTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_rows.size());
}

int ResultTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return 10; // 特征码名/进程/PID/地址(十进制)/地址/模块/模块基址/匹配字节/命中时间/自定义
}

QVariant ResultTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_rows.size())) return {};
    if (role != Qt::DisplayRole && role != Qt::EditRole) return {};
    const ResultRow& r = m_rows[static_cast<size_t>(index.row())];
    switch (index.column()) {
    case 0: return r.patternName;
    case 1: return r.processName;
    case 2: return r.pid;
    case 3: return QString::number(r.address); // 十进制地址
    case 4: return hexAddr(r.address);
    case 5: return r.moduleName;
    case 6: return r.moduleBase ? hexAddr(r.moduleBase) : QString();
    case 7: return r.matchedBytes;
    case 8: return r.hitTime;
    case 9: return r.custom;
    default: return {};
    }
}

bool ResultTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (role != Qt::EditRole || !index.isValid()) return false;
    if (index.row() >= static_cast<int>(m_rows.size())) return false;
    if (index.column() != customColumn()) return false; // 仅自定义列可编辑
    m_rows[static_cast<size_t>(index.row())].custom = value.toString();
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    return true;
}

QVariant ResultTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) return {};
    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return tr("特征码名");
        case 1: return tr("进程");
        case 2: return QStringLiteral("PID");
        case 3: return tr("地址(十进制)");
        case 4: return tr("地址");
        case 5: return tr("模块");
        case 6: return tr("模块基址");
        case 7: return tr("匹配字节");
        case 8: return tr("命中时间");
        case 9: return tr("自定义");
        default: return {};
        }
    }
    return section + 1;
}

Qt::ItemFlags ResultTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.column() == customColumn()) f |= Qt::ItemIsEditable;
    return f;
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

void ResultTableModel::removeRows(const std::vector<int>& rows) {
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

void ResultTableModel::setCustom(const std::vector<int>& rows, const QString& text) {
    if (rows.empty()) return;
    const int col = customColumn();
    for (int r : rows) {
        if (r < 0 || r >= static_cast<int>(m_rows.size())) continue;
        m_rows[static_cast<size_t>(r)].custom = text;
        const QModelIndex idx = index(r, col);
        emit dataChanged(idx, idx, {Qt::DisplayRole, Qt::EditRole});
    }
}

void ResultTableModel::retranslate() {
    emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
}
