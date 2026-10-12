// view/hoverrowdelegate.cpp - 整行悬停高亮实现
#include "view/hoverrowdelegate.h"

#include <QColor>
#include <QModelIndex>
#include <QStyle>
#include <QStyleOptionViewItem>

#include "view/hoverrowtableview.h"

namespace {
// 悬停色：选中色 #3a6ea5 的同色系、更淡（整行提示）
const QColor kHoverColor(0xE8, 0xF0, 0xF8);
} // namespace

HoverRowDelegate::HoverRowDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void HoverRowDelegate::initStyleOption(QStyleOptionViewItem* option,
                                       const QModelIndex& index) const {
    QStyledItemDelegate::initStyleOption(option, index);
    // 清除原生「单格」悬停高亮，避免悬停列单独变深（C-01）
    option->state &= ~QStyle::State_MouseOver;
    // 悬停行的非选中单元格：设整行淡色底色（各单元格只画自身矩形，不越界）（C-01/C-03）
    const auto* view = qobject_cast<const HoverRowTableView*>(option->widget);
    if (view && index.row() == view->hoveredRow() &&
        !(option->state & QStyle::State_Selected)) {
        option->backgroundBrush = kHoverColor;
    }
}
