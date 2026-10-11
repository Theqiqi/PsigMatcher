// view/signatureeditorpage.h - 特征码编辑页（只画 UI）
#pragma once

#include <QWidget>

class QTableView;
class QPushButton;
class QAbstractItemModel;

class SignatureEditorPage : public QWidget {
    Q_OBJECT
public:
    explicit SignatureEditorPage(QWidget* parent = nullptr);

    // 控件 getter（供 ViewModel connect）
    QTableView* table() const { return m_table; }
    QPushButton* addRowBtn() const { return m_addRowBtn; }
    QPushButton* deleteRowBtn() const { return m_deleteRowBtn; }
    QPushButton* manageColumnsBtn() const { return m_manageColumnsBtn; }

    // 公开 UI 方法（供 ViewModel 更新界面）
    void setModel(QAbstractItemModel* model);
    void selectRow(int row);

private:
    QTableView* m_table = nullptr;
    QPushButton* m_addRowBtn = nullptr;
    QPushButton* m_deleteRowBtn = nullptr;
    QPushButton* m_manageColumnsBtn = nullptr;
};
