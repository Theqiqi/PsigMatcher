// model/processtablemodel.h - 进程列表模型（只读）
#pragma once

#include <QAbstractTableModel>

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

    void setProcesses(const std::vector<ProcessRow>& list);
    std::vector<ProcessRow> processes() const;
    ProcessRow processAt(int row) const;

private:
    std::vector<ProcessRow> m_rows;
};
