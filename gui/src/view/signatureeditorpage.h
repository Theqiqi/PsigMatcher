// view/signatureeditorpage.h - 特征码编辑页（只画 UI，右键菜单）
#pragma once

#include <QWidget>

class QTableView;
class QMenu;
class QAction;
class QAbstractItemModel;

class SignatureEditorPage : public QWidget {
    Q_OBJECT
public:
    explicit SignatureEditorPage(QWidget* parent = nullptr);

    // 控件 getter（供 ViewModel connect）
    QTableView* table() const { return m_table; }
    QMenu* contextMenu() const { return m_contextMenu; }
    QAction* addRowAction() const { return m_addRowAction; }
    QAction* deleteRowsAction() const { return m_deleteRowsAction; }
    QAction* manageColumnsAction() const { return m_manageColumnsAction; }

    // 公开 UI 方法（供 ViewModel 更新界面）
    void setModel(QAbstractItemModel* model);
    void selectRow(int row);

private:
    QTableView* m_table = nullptr;
    QMenu* m_contextMenu = nullptr;
    QAction* m_addRowAction = nullptr;
    QAction* m_deleteRowsAction = nullptr;
    QAction* m_manageColumnsAction = nullptr;
};
