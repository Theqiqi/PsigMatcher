// view/columnmanagerdialog.h - 列管理对话框（只画 UI）
#pragma once

#include <QDialog>

#include <vector>

#include "model/types.h"

class QTableWidget;
class QPushButton;
class QDialogButtonBox;

class ColumnManagerDialog : public QDialog {
    Q_OBJECT
public:
    explicit ColumnManagerDialog(QWidget* parent = nullptr);

    QPushButton* addBtn() const { return m_addBtn; }
    QPushButton* removeBtn() const { return m_removeBtn; }
    QPushButton* upBtn() const { return m_upBtn; }
    QPushButton* downBtn() const { return m_downBtn; }
    QTableWidget* table() const { return m_table; }
    QDialogButtonBox* buttonBox() const { return m_buttonBox; }

    void setColumns(const std::vector<ColumnDef>& cols);
    std::vector<ColumnDef> columns() const;

private:
    QTableWidget* m_table = nullptr;
    QPushButton* m_addBtn = nullptr;
    QPushButton* m_removeBtn = nullptr;
    QPushButton* m_upBtn = nullptr;
    QPushButton* m_downBtn = nullptr;
    QDialogButtonBox* m_buttonBox = nullptr;
};
