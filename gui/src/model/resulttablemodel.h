// model/resulttablemodel.h - 匹配结果模型（只读）
#pragma once

#include <QAbstractTableModel>

#include <vector>

#include "model/types.h"

class ResultTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit ResultTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void setResults(const std::vector<ResultRow>& list);
    std::vector<ResultRow> results() const;
    void appendResult(const ResultRow& r);
    void clear();

private:
    std::vector<ResultRow> m_rows;
};
