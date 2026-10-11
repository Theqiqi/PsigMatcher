// model/signaturetablemodel.h - 特征码表模型（动态列，可编辑）
#pragma once

#include <QAbstractTableModel>

#include <vector>

#include "model/types.h"

class SignatureTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit SignatureTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void setColumns(const std::vector<ColumnDef>& columns);
    std::vector<ColumnDef> columns() const;
    void setRows(const std::vector<SignatureRow>& rows);
    std::vector<SignatureRow> rows() const;
    void addRow();
    void removeRows(const std::vector<int>& rows);
    QString valueAt(int row, const QString& key) const;

private:
    int columnIndex(const QString& key) const;

    std::vector<ColumnDef> m_columns;
    std::vector<SignatureRow> m_rows;
};
