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

    // 键下拉选项（来自 psig 内置列键，由 ViewModel 注入；View 不依赖 psig）
    void setKeyOptions(const QStringList& keys) { m_keyOptions = keys; }

    // 由 ViewModel 连接按钮后调用（View 内部只操作自身表格）
    void addColumnRow();             // 追加一空列
    void removeSelectedColumnRow();  // 删除选中列
    void moveSelectedUp();           // 选中列上移
    void moveSelectedDown();         // 选中列下移

private:
    void SwapRows(int a, int b);     // 交换两行的单元格项
    // 构建一行：标题文本 + 键/类型下拉
    void buildRow(int r, const QString& title, const QString& key, const QString& type);

    QTableWidget* m_table = nullptr;
    QPushButton* m_addBtn = nullptr;
    QPushButton* m_removeBtn = nullptr;
    QPushButton* m_upBtn = nullptr;
    QPushButton* m_downBtn = nullptr;
    QDialogButtonBox* m_buttonBox = nullptr;
    QStringList m_keyOptions;
};
