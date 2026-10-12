// view/hoverrowtableview.cpp - 整行悬停视图实现
#include "view/hoverrowtableview.h"

#include <QAbstractItemModel>
#include <QMouseEvent>

#include "view/hoverrowdelegate.h"

HoverRowTableView::HoverRowTableView(QWidget* parent) : QTableView(parent) {
    setMouseTracking(true);
    viewport()->setMouseTracking(true);
    setItemDelegate(new HoverRowDelegate(this));
}

void HoverRowTableView::mouseMoveEvent(QMouseEvent* event) {
    const int row = indexAt(event->pos()).row();
    if (row != m_hoverRow) {
        updateRow(m_hoverRow); // 旧行恢复
        updateRow(row);        // 新行高亮
        m_hoverRow = row;
    }
    QTableView::mouseMoveEvent(event);
}

void HoverRowTableView::leaveEvent(QEvent* event) {
    if (m_hoverRow >= 0) {
        updateRow(m_hoverRow);
        m_hoverRow = -1;
    }
    QTableView::leaveEvent(event);
}

void HoverRowTableView::updateRow(int row) {
    if (row < 0 || !model() || row >= model()->rowCount()) return;
    const QRect r = visualRect(model()->index(row, 0));
    viewport()->update(0, r.top(), viewport()->width(), r.height());
}
