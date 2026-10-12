// view/hoverrowdelegate.cpp - 整行悬停高亮实现
#include "view/hoverrowdelegate.h"

#include <QAbstractItemView>
#include <QColor>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QTableView>

namespace {
// 悬停色：选中色 #3a6ea5 的同色系、更淡（整行提示）
const QColor kHoverColor(0xE8, 0xF0, 0xF8);
} // namespace

HoverRowDelegate::HoverRowDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void HoverRowDelegate::attach(QTableView* view) {
    if (!view) return;
    view->setMouseTracking(true);
    view->setItemDelegate(new HoverRowDelegate(view));
}

void HoverRowDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    if (opt.state & QStyle::State_MouseOver) {
        const auto* view = qobject_cast<const QAbstractItemView*>(opt.widget);
        if (view) {
            // 将单元格矩形横向扩展为整行（覆盖同行的其他列）
            QRect rowRect = opt.rect;
            rowRect.setLeft(0);
            rowRect.setRight(view->viewport()->width() - 1);
            painter->save();
            painter->fillRect(rowRect, kHoverColor);
            painter->restore();
        }
    }
    QStyledItemDelegate::paint(painter, opt, index);
}
