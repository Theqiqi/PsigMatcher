// view/resultpage.h - 匹配结果页（只画 UI，模板文本 + 右键菜单）
#pragma once

#include <QWidget>

class QTableView;
class QPlainTextEdit;
class QMenu;
class QAction;
class QAbstractItemModel;

class ResultPage : public QWidget {
    Q_OBJECT
public:
    explicit ResultPage(QWidget* parent = nullptr);

    QTableView* table() const { return m_table; }
    QPlainTextEdit* templateEdit() const { return m_templateEdit; }
    QMenu* contextMenu() const { return m_contextMenu; }
    QAction* exportCsvAction() const { return m_exportCsvAction; }
    QAction* exportDbAction() const { return m_exportDbAction; }
    QAction* exportTxtAction() const { return m_exportTxtAction; }
    QAction* selectColumnsAction() const { return m_selectColumnsAction; }
    QAction* deleteRowsAction() const { return m_deleteRowsAction; }
    QAction* fillCustomAction() const { return m_fillCustomAction; }
    QAction* clearAction() const { return m_clearAction; }

    void setModel(QAbstractItemModel* model);
    void setTemplateText(const QString& text);
    QString templateText() const;
    void retranslateUi();   // 语言切换后刷新本页文案（含导出子菜单标题）

private:
    QTableView* m_table = nullptr;
    QPlainTextEdit* m_templateEdit = nullptr;
    QMenu* m_contextMenu = nullptr;
    QMenu* m_exportMenu = nullptr;
    QAction* m_exportCsvAction = nullptr;
    QAction* m_exportDbAction = nullptr;
    QAction* m_exportTxtAction = nullptr;
    QAction* m_selectColumnsAction = nullptr;
    QAction* m_deleteRowsAction = nullptr;
    QAction* m_fillCustomAction = nullptr;
    QAction* m_clearAction = nullptr;
};
