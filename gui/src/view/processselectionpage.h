// view/processselectionpage.h - 进程选择页（只画 UI）
#pragma once

#include <QWidget>

class QTableView;
class QPushButton;
class QLineEdit;
class QAbstractItemModel;

class ProcessSelectionPage : public QWidget {
    Q_OBJECT
public:
    explicit ProcessSelectionPage(QWidget* parent = nullptr);

    QTableView* table() const { return m_table; }
    QPushButton* refreshBtn() const { return m_refreshBtn; }
    QPushButton* selectAllBtn() const { return m_selectAllBtn; }
    QLineEdit* filterEdit() const { return m_filterEdit; }

    void setModel(QAbstractItemModel* model);

private:
    QTableView* m_table = nullptr;
    QPushButton* m_refreshBtn = nullptr;
    QPushButton* m_selectAllBtn = nullptr;
    QLineEdit* m_filterEdit = nullptr;
};
