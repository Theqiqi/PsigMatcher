// model/processtablemodel.h - 进程列表模型（只读，含图标与排序）
#pragma once

#include <QAbstractTableModel>
#include <QIcon>

#include <vector>

#include "model/types.h"

class ProcessTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit ProcessTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void setProcesses(const std::vector<ProcessRow>& list);
    void setIcon(int row, const QIcon& icon);   // 异步图标回填
    std::vector<ProcessRow> processes() const;
    ProcessRow processAt(int row) const;
    void retranslate();   // 语言切换后刷新表头

private:
    std::vector<ProcessRow> m_rows;
};
