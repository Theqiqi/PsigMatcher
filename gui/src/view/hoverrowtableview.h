// view/hoverrowtableview.h - 支持整行悬停的表格视图（只画 UI）
#pragma once

#include <QTableView>

class QMouseEvent;
class QEvent;

// 整行悬停高亮：视图跟踪「悬停行」，悬停行变化时重绘受影响的行。
// 委托（HoverRowDelegate）对每个单元格只画自身矩形，避免越界覆盖相邻列。
// 仅负责视图行为：不发射信号、不修改 Model、不写日志。
class HoverRowTableView : public QTableView {
    Q_OBJECT
public:
    explicit HoverRowTableView(QWidget* parent = nullptr);

    int hoveredRow() const { return m_hoverRow; } // -1 表示无悬停行

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void updateRow(int row); // 重绘该行的视口矩形
    int m_hoverRow = -1;
};
