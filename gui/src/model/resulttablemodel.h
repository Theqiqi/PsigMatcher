// model/resulttablemodel.h - 匹配结果模型（末列可编辑，支持删行）
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
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void setResults(const std::vector<ResultRow>& list);
    std::vector<ResultRow> results() const;
    void appendResult(const ResultRow& r);
    void clear();

    void removeRows(const std::vector<int>& rows);                       // 单行/多选批量删除
    void setCustom(const std::vector<int>& rows, const QString& text);   // 批量填充自定义列
    void retranslate();                                                  // 语言切换后刷新表头

    static int customColumn(); // 自定义列索引（末列）

private:
    std::vector<ResultRow> m_rows;
};
