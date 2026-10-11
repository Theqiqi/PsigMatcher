// view/resultpage.h - 匹配结果页（只画 UI）
#pragma once

#include <QWidget>

class QTableView;
class QPushButton;
class QAbstractItemModel;

class ResultPage : public QWidget {
    Q_OBJECT
public:
    explicit ResultPage(QWidget* parent = nullptr);

    QTableView* table() const { return m_table; }
    QPushButton* exportCsvBtn() const { return m_exportCsvBtn; }
    QPushButton* exportDbBtn() const { return m_exportDbBtn; }
    QPushButton* exportTxtBtn() const { return m_exportTxtBtn; }
    QPushButton* clearBtn() const { return m_clearBtn; }

    void setModel(QAbstractItemModel* model);

private:
    QTableView* m_table = nullptr;
    QPushButton* m_exportCsvBtn = nullptr;
    QPushButton* m_exportDbBtn = nullptr;
    QPushButton* m_exportTxtBtn = nullptr;
    QPushButton* m_clearBtn = nullptr;
};
