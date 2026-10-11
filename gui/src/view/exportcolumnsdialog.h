// view/exportcolumnsdialog.h - 选择导出列对话框（只画 UI）
#pragma once

#include <QDialog>
#include <QStringList>

#include <utility>
#include <vector>

class QListWidget;
class QDialogButtonBox;

class ExportColumnsDialog : public QDialog {
    Q_OBJECT
public:
    explicit ExportColumnsDialog(QWidget* parent = nullptr);

    // cols: (key, title) 有序；selectedKeys 为预勾选键（空 = 全选）
    void setColumns(const std::vector<std::pair<QString, QString>>& cols,
                    const QStringList& selectedKeys);
    QStringList selectedKeys() const;

    QListWidget* list() const { return m_list; }
    QDialogButtonBox* buttonBox() const { return m_buttons; }

private:
    QListWidget* m_list = nullptr;
    QDialogButtonBox* m_buttons = nullptr;
};
