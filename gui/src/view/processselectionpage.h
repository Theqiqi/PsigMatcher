// view/processselectionpage.h - 进程选择页（只画 UI，右键菜单）
#pragma once

#include <QWidget>

class QTableView;
class QMenu;
class QAction;
class QLineEdit;
class QAbstractItemModel;

class ProcessSelectionPage : public QWidget {
    Q_OBJECT
public:
    explicit ProcessSelectionPage(QWidget* parent = nullptr);

    QTableView* table() const { return m_table; }
    QLineEdit* filterEdit() const { return m_filterEdit; }
    QMenu* contextMenu() const { return m_contextMenu; }
    QAction* useAsTargetAction() const { return m_useAsTargetAction; }
    QAction* refreshAction() const { return m_refreshAction; }
    QAction* selectAllAction() const { return m_selectAllAction; }

    void setModel(QAbstractItemModel* model);
    void retranslateUi();   // 语言切换后刷新本页文案（含筛选框占位符）

private:
    QTableView* m_table = nullptr;
    QLineEdit* m_filterEdit = nullptr;
    QMenu* m_contextMenu = nullptr;
    QAction* m_useAsTargetAction = nullptr;
    QAction* m_refreshAction = nullptr;
    QAction* m_selectAllAction = nullptr;
};
